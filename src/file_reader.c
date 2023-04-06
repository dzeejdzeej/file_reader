#include <file_reader.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

/***********************************************************
 * LOCAL FUNCTION DECLARATIONS
***********************************************************/

static bool check_file_and_prepare_stats(const char* file_name, struct stat* file_stat_buffer);
static bool is_file_virtual(const char* const file_name);
static File_Reader* normal_file_reader_new(const char* file_name);
static File_Reader* virtual_file_reader_new(const char* file_name);
static size_t calculate_no_of_lines(const File_Reader* file_reader);
static void file_reader_calculate_lines_offset(File_Reader* file_reader);
static size_t calculate_line_length(const File_Reader* file_reader, size_t line);


/***********************************************************
 * FILE_READER_H API FUNCTIONS DEFINITIONS
***********************************************************/

File_Reader* file_reader_new(const char* file_name)
{
    if (file_name == NULL)
    {
        printf("Icorrect file name: \"%s\"\n", file_name);
        return NULL;
    }

    File_Reader* file_reader = NULL;

    if (is_file_virtual(file_name) == true)
    {
        file_reader = virtual_file_reader_new(file_name);
    }
    else
    {
        file_reader = normal_file_reader_new(file_name);
    }

    if (file_reader != NULL && file_reader->buffer_size > 0)
    {
        file_reader_calculate_lines_offset(file_reader);
    }

    return file_reader;    
}

void file_reader_delete(File_Reader* file_reader)
{
    if (file_reader == NULL)
    {
        printf("Can't delete file_reader pointed by NULL pointer\n");
        return;
    }

    if (file_reader->line_offset != NULL)
    {
        free(file_reader->line_offset);
    }

    free(file_reader);
}

size_t file_reader_get_file_size(const File_Reader* file_reader)
{
    if (file_reader == NULL)
    {
        printf("Can't open given file_reader\n");
        return 0;
    }

    // size of file is smaller by 1 beacuse lack of '\0'
    return file_reader->buffer_size - 1;
}

size_t file_reader_get_no_of_lines(const File_Reader* file_reader)
{
    if (file_reader == NULL)
    {
        printf("Can't open given file_reader\n");
        return 0;       
    }

    return file_reader->no_of_lines;
}

const char* file_reader_get_file_buffer(const File_Reader* file_reader)
{
    if (file_reader == NULL)
    {
        printf("Can't open given file_reader\n");
        return NULL;
    }

    return file_reader->buffer;
}

char* file_reader_get_copy_of_file_buffer(const File_Reader* file_reader)
{
    if (file_reader == NULL)
    {
        printf("Can't open given file_reader\n");
        return NULL;
    }
    
    char* copy_buffer = calloc(file_reader->buffer_size, sizeof(*copy_buffer));

    if (copy_buffer == NULL)
    {
        printf("Can't create copy_buffer\n");
        return NULL;
    }

    memcpy(copy_buffer, file_reader->buffer, file_reader->buffer_size);

    return copy_buffer;
}

void file_reader_delete_copy_of_file_buffer(char* copy_buffer)
{
    if (copy_buffer == NULL)
    {
        printf("Can't delete buffer pointed by NULL pointer\n");
        return;
    }

    free(copy_buffer);
}

static File_Reader* normal_file_reader_new(const char* file_name)
{
    struct stat file_stat_buffer = {0};
    size_t read_attempts_counter = 0;

    if (check_file_and_prepare_stats(file_name, &file_stat_buffer) == false)
    {
        return NULL;
    }

    const size_t file_size_in_bytes = (size_t)file_stat_buffer.st_size;

    if (file_size_in_bytes == 0)
    {
        File_Reader* empty_file = NULL;
        return empty_file;
    }

    // buffer is extended by 1 becasue we need to add \0 at the end
    const size_t buffer_size_in_bytes = file_size_in_bytes + 1;

    char* buffer = calloc(buffer_size_in_bytes, sizeof(*buffer));
    if (buffer == NULL)
    {
        printf("Can't create buffer for normal file: \"%s\"\n", file_name);
        return NULL;
    }

    while (true)
    {
        FILE* const normal_file = fopen(file_name, "r");
        if (normal_file == NULL)
        {
            printf("Can't open a normal file: \"%s\"\n", file_name);
            free(buffer);
            return NULL;
        }

        // read the file and copy the content of file to the buffer
        const size_t bytes_read_from_file = 
          fread(buffer, sizeof(buffer[0]), file_size_in_bytes, normal_file);

        // check if there is no error while perfoming operations on file
        const bool has_error = ferror(normal_file);

        // there will be no more operations on file so close it now
        fclose(normal_file);
    
        /* if error occurs or bytes read from file are not equal the file size then
           try again MAX_NO_OF_READ_ATTEMPTS times, if still error close function */
        if (has_error == true || bytes_read_from_file != file_size_in_bytes)
        {
            ++read_attempts_counter;
            if (read_attempts_counter >= MAX_NO_OF_READ_ATTEMPTS)
            {
                printf("Can't perform operations on file \"%s\"\n", file_name);
                free(buffer);
                return NULL;
            }
            else
            {
                // start at the beggining of the loop and try once again
                continue;
            }
        }

        // if no error just leave the loop
        break;
    }

    // Add line termination at the end of buffer, -1 beacuse array starts with index 0
    buffer[buffer_size_in_bytes - 1] = '\0';

    // create file reader instance based on buffer size and intialize
    File_Reader* normal_file_reader =
        calloc(1, sizeof(*normal_file_reader) + buffer_size_in_bytes);
    
    if (normal_file_reader == NULL)
    {
        printf("Can't create file reader instance for normal file: \"%s\"\n", file_name);
        free(buffer);
        return NULL;
    }

    normal_file_reader->name = file_name;
    normal_file_reader->buffer_size = buffer_size_in_bytes;
    memcpy(normal_file_reader->buffer, buffer, buffer_size_in_bytes);
    free(buffer);
    normal_file_reader->no_of_lines = calculate_no_of_lines(normal_file_reader);
    
    return normal_file_reader;
}

static File_Reader* virtual_file_reader_new(const char* file_name)
{
    char* buffer = calloc(VIRTUAL_FILE_BUFFER_IN_BYTES, sizeof(*buffer));
    if (buffer == NULL)
    {
        printf("Can't create buffer for virtual file: \"%s\"\n", file_name);
        return NULL;
    }

    size_t buffer_size_in_bytes = VIRTUAL_FILE_BUFFER_IN_BYTES;
    size_t bytes_read_from_file = 0;
    size_t read_attempts_counter = 0;

    while (true)
    {
        FILE* const virtual_file = fopen(file_name, "r");

        if (virtual_file == NULL)
        {
            printf("Can't open a virtual file: \"%s\"\n", file_name);
            free(buffer);
            return NULL;
        }

        // read the file, copy the content of file to buffer and return amount of read bytes
        bytes_read_from_file = 
          fread(buffer, sizeof(buffer[0]), buffer_size_in_bytes, virtual_file);

        // check if read file has EOF sign
        const bool has_eof = feof(virtual_file);

        // check if there is no error while perfoming operations on file
        const bool has_error = ferror(virtual_file);

        // there will be no more operations on file so close it now
        fclose(virtual_file);

        // if error occurs try again MAX_NO_OF_READ_ATTEMPTS times, if still error close function
        if (has_error == true)
        {
            ++read_attempts_counter;
            if (read_attempts_counter >= MAX_NO_OF_READ_ATTEMPTS)
            {
                printf("Can't perform operations on file \"%s\"\n", file_name);
                free(buffer);
                return NULL;
            }
            else
            {
                // start at the beggining of the loop and try once again
                continue;
            }
        }

        // if read bytes don't excced buffer and file has eof just leave the loop
        if (bytes_read_from_file < buffer_size_in_bytes && has_eof)
        {
            break;
        }

        /* If previous statement is not true, it means that the buffer were too small.
           So realease old buffer and create new one with doubled size.
        */
        printf("Buffer size (%ld bytes) was too small, double the size of buffer \n",
                buffer_size_in_bytes);
        free(buffer);
        buffer_size_in_bytes = buffer_size_in_bytes * 2;
        buffer = calloc(buffer_size_in_bytes, sizeof(*buffer));
        if (buffer == NULL)
        {
            printf("Can't create buffer for virtual file: \"%s\"\n", file_name);
            return NULL;
        }
    }

    // add space for '/0' sign
    const size_t file_reader_buffer_size = bytes_read_from_file + 1;
    buffer[file_reader_buffer_size - 1] = '\0';

    // create file reader instance based on buffer size and intialize
    File_Reader* virtual_file_reader =
        calloc(1, sizeof(*virtual_file_reader) + file_reader_buffer_size);
    
    if (virtual_file_reader == NULL)
    {
        printf("Can't create file reader instance for virtual file: \"%s\"\n", file_name);
        free(buffer);
        return NULL;
    }

    virtual_file_reader->name = file_name;
    virtual_file_reader->buffer_size = file_reader_buffer_size;
    memcpy(virtual_file_reader->buffer, buffer, file_reader_buffer_size);
    free(buffer);
    virtual_file_reader->no_of_lines = calculate_no_of_lines(virtual_file_reader);
    
    return virtual_file_reader;
}

/*
    Verify if given file name is not null and fetch file stats for given file
*/
static bool check_file_and_prepare_stats(const char* file_name, struct stat* file_stat_buffer)
{
    if (file_name == NULL)
    {
        printf("Icorrect file name: \"%s\"\n", file_name);
        return false;
    }

    if (stat(file_name, file_stat_buffer) == -1)
    {
        printf("Could not fetch file stats correctly for file \"%s\"\n", file_name);
        return false;
    }

    return true;
}

static bool is_file_virtual(const char* const file_name)
{
    struct stat file_stat_buffer = {0};

    if (check_file_and_prepare_stats(file_name, &file_stat_buffer) == false)
    {
        return false;
    }

    // virtual files have size which equals 0
    if (file_stat_buffer.st_size != 0)
    {
        return false;
    }

    FILE* file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Could not open given file \"%s\"\n", file_name);
        return false;
    }

    enum {TEMP_BUFFER_SIZE = 1};
    char temp_buffer[TEMP_BUFFER_SIZE] = {'0'};
    const size_t no_of_elements_read = fread(temp_buffer, sizeof(temp_buffer[0]), TEMP_BUFFER_SIZE, file);

    /*
        Check if despites of zero size file, we got an element to read,
        Which is also not the EOF sign. 
    */
    const bool lack_of_eof_in_temp_buffer = !feof(file);
    const bool is_virtual = no_of_elements_read && lack_of_eof_in_temp_buffer;

    fclose(file);

    return is_virtual;
}

static size_t calculate_no_of_lines(const File_Reader* file_reader)
{
    if (file_reader == NULL)
    {
        printf("Can't open file_reader\n");
        return 0;
    }

    // proceed empty file
    if (file_reader->buffer_size == 0)
    {
        return 0;
    }

    // if file exist there is at least one line
    size_t line_counter = 1;
    const char* ptr = file_reader->buffer;
    const char* ptr_to_last_elem = file_reader->buffer + file_reader->buffer_size;

   while (ptr < ptr_to_last_elem) 
    {
        if (*ptr == '\n')
        {
            ++line_counter;
        }
        ++ptr;
    }
    
    return line_counter;
}

static void file_reader_calculate_lines_offset(File_Reader* file_reader)
{
    if (file_reader == NULL)
    {
        printf("Can't open file_reader\n");
        return;
    }

    // allocate an extra element for the line_offset to mark the end of the array
    file_reader->line_offset =
        calloc(file_reader->no_of_lines + 1, sizeof(*file_reader->line_offset));

    char* current_position = file_reader->buffer;
    char* next_position = strchr(current_position, '\n');
    size_t current_line = 0;
    file_reader->line_offset[current_line] = 0;

    while (next_position != NULL)
    {
        current_position = next_position + 1;
        next_position = strchr(current_position, '\n');
        ++current_line;
        file_reader->line_offset[current_line] = (size_t)(current_position - file_reader->buffer);
    }
}

static size_t calculate_line_length(const File_Reader* file_reader, size_t line)
{
    if (file_reader == NULL)
    {
        return 0;
    }

    if (line > file_reader->no_of_lines || line < 1)
    {
        printf("Incorrect line number to get\n");
        return 0;
    }

    size_t line_length = 0;

    /*
        Example of line_lenght calculation:
            line 1: ab     offset[1] = 3
            line 2: cd     offset[2] = 6
            line 3: efg    offset[3] = 10
            line 4: h      offset[4] = 12
            line 5: ij     buffer_size = file_size + 1 = 14 + 1 = 15

            line_length(3) = 10(offset[3]) - 6(offset[2]) - 1(character \0) = 3
            line_length(5) = 15(buffer size) - 12(offset[4]) - 1(character \0) = 2
    */
    if (line != file_reader->no_of_lines)
    {
        line_length = file_reader->line_offset[line] - file_reader->line_offset[line - 1] - 1;
    }
    else
    {
        line_length = file_reader->buffer_size - file_reader->line_offset[line - 1] - 1;
    }

    return line_length;
}

char* file_reader_get_copy_of_line(const File_Reader* file_reader, size_t line)
{
    if (file_reader == NULL)
    {
        printf("Can't open given file_reader\n");
        return NULL;
    }

    if (file_reader->buffer_size == 0)
    {
        printf("Size of file_reader buffer content is 0\n");
        return NULL;
    }

    const size_t line_length = calculate_line_length(file_reader, line);

    if (line_length == 0)
    {
        return NULL;
    }

    // create buffer for specified line, add +1 for '\0' sign
    char* line_buffer = calloc(line_length + 1, sizeof(*line_buffer));
    if (line_buffer == NULL)
    {
        return NULL;
    }

    // copy content of line to new buffer
    memcpy(line_buffer, 
           &file_reader->buffer[file_reader->line_offset[line - 1]],
           line_length);
    
    // add null termination at the end of string
    line_buffer[line_length] = '\0';

    return line_buffer;
}

void file_reader_delete_copy_of_line(char* line_buffer)
{
    if (line_buffer == NULL)
    {
        return;
    }

    free(line_buffer);
}


