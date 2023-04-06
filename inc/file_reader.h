#ifndef FILE_READER_H
#define FILE_READER_H

#include <stddef.h>

#define VIRTUAL_FILE_BUFFER_IN_BYTES 2048
#define MAX_NO_OF_READ_ATTEMPTS 10

typedef struct File_Reader
{
    const char* name;         // file name
    size_t      no_of_lines;  // number of lines in file
    size_t*     line_offset;  // buffer for lines offset e.g.: line_offset[1] indicates starting index of line 2
    size_t      buffer_size;  // size of buffer (size of file + 1)
    char        buffer[];     // buffer which stores file content extended by '/0' sign 
} File_Reader;

File_Reader* file_reader_new(const char* file_name);
void         file_reader_delete(File_Reader* file_reader);
size_t       file_reader_get_file_size(const File_Reader* file_reader);
size_t       file_reader_get_no_of_lines(const File_Reader* file_reader);
const char*  file_reader_get_file_buffer(const File_Reader* file_reader);
char*        file_reader_get_copy_of_file_buffer(const File_Reader* file_reader);
void         file_reader_delete_copy_of_file_buffer(char* copy_buffer);
char*        file_reader_get_copy_of_line(const File_Reader* file_reader, size_t line);
void         file_reader_delete_copy_of_line(char* line_buffer);

#endif // FILE_READER_H