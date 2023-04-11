#include <file_reader.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void file_reader_normal_file_test(void);
static void file_reader_virtual_file_test(void);
static void file_reader_empty_file_test(void);
static void file_reade_corner_cases_test(void);


int main(void)
{
    file_reader_normal_file_test();
    file_reader_virtual_file_test();
    file_reader_empty_file_test();
    file_reade_corner_cases_test();

    return 0;
}

/*
    Simulate normal file which has 5 lines with content described under "file content"
*/
static void file_reader_normal_file_test(void)
{
    const char* file_name = "example_file.txt";
    FILE* example_file = fopen(file_name, "w+");
    assert(example_file != NULL);

    const char* file_content =  "ab\n"
                                "cd\n"
                                "efg\n"
                                "h\n"
                                "ij";
    const size_t content_size = strlen(file_content);
    fwrite(file_content, sizeof(char), content_size, example_file);
    fclose(example_file);

    File_Reader* fr_normal = file_reader_new(file_name);
    assert(fr_normal != NULL);

    const size_t size_of_file_in_bytes = file_reader_get_file_size(fr_normal);
    assert(size_of_file_in_bytes > 0);

    const size_t no_of_lines = file_reader_get_no_of_lines(fr_normal);
    assert(no_of_lines == 5);

    const char* content_of_file = file_reader_get_file_buffer(fr_normal);
    assert(content_of_file != NULL);

    const char* copied_content_of_file = (const char*)file_reader_get_copy_of_file_buffer(fr_normal);
    assert(copied_content_of_file != NULL);

    const char* line_buf = (const char*)file_reader_get_copy_of_line(fr_normal, 2);
    assert(strcmp(line_buf, "cd") == 0);

    remove(file_name);
    file_reader_delete_copy_of_line((char*)line_buf);
    file_reader_delete_copy_of_file_buffer((char*)copied_content_of_file);
    file_reader_delete(fr_normal);
}

/*
    Testing virtual file like /proc/stat
*/
static void file_reader_virtual_file_test(void)
{
    File_Reader* fr_virtual = file_reader_new("/proc/stat");
    assert(fr_virtual != NULL);

    const size_t size_of_file_in_bytes = file_reader_get_file_size(fr_virtual);
    assert(size_of_file_in_bytes > 0);

    const size_t no_of_lines = file_reader_get_no_of_lines(fr_virtual);
    assert(no_of_lines > 8);  // certainly more than 8 lines for /proc/stat

    const char* content_of_file = file_reader_get_file_buffer(fr_virtual);
    assert(content_of_file != NULL);

    const char* copied_content_of_file = (const char*)file_reader_get_copy_of_file_buffer(fr_virtual);
    assert(copied_content_of_file != NULL);

    const char* line_buf = (const char*)file_reader_get_copy_of_line(fr_virtual, 2);
    char fist_4_of_line_buf[5] = {0}; // one more element in array for '\0'
    strncpy(fist_4_of_line_buf, line_buf, 4);    
    assert(strcmp(fist_4_of_line_buf, "cpu0") == 0);

    file_reader_delete_copy_of_line((char*)line_buf);    
    file_reader_delete_copy_of_file_buffer((char*)copied_content_of_file);
    file_reader_delete(fr_virtual);
}

/*
    Testing normal file like with no content
*/
static void file_reader_empty_file_test(void)
{
    const char* file_name = "example_file_empty.txt";
    FILE* example_file = fopen(file_name, "w+");
    assert(example_file != NULL);

    const char* file_content =  "";
    const size_t content_size = strlen(file_content);
    fwrite(file_content, sizeof(char), content_size, example_file);
    fclose(example_file);
    
    File_Reader* fr_empty = file_reader_new(file_name);
    assert(fr_empty == NULL);
    
    remove(file_name);
}

static void file_reade_corner_cases_test(void)
{
    /* 
        NULL pointer passed to file reader constructor
    */
    {
        File_Reader* fr_normal = file_reader_new(NULL);
        assert(fr_normal == NULL);
    }

    /* 
        One-liner with new line sign at the end, check:
            - function to calculate number of lines if correctly returns 1 in that case
            - accessing line out of scope
            - all other functionalities for that case
    */
    {
        const char* file_name = "example_oneLiner_file.txt";
        FILE* example_file = fopen(file_name, "w+");
        assert(example_file != NULL);

        const char* file_content =  "Something else\n";
        const size_t content_size = strlen(file_content);
        fwrite(file_content, sizeof(char), content_size, example_file);
        fclose(example_file);

        File_Reader* fr_oneLiner = file_reader_new(file_name);
        assert(fr_oneLiner != NULL);

        const size_t size_of_file_in_bytes = file_reader_get_file_size(fr_oneLiner);
        assert(size_of_file_in_bytes == 15);

        // despite of /n at the end, there is just one line
        const size_t no_of_lines = file_reader_get_no_of_lines(fr_oneLiner);
        assert(no_of_lines == 1);

        const char* content_of_file = file_reader_get_file_buffer(fr_oneLiner);
        assert(content_of_file != NULL);

        const char* copied_content_of_file = (const char*)file_reader_get_copy_of_file_buffer(fr_oneLiner);
        assert(copied_content_of_file != NULL);

        // first line should equals the content of whole file since it one-liner
        const char* line_buf = (const char*)file_reader_get_copy_of_line(fr_oneLiner, 1);
        assert(strcmp(line_buf, file_content) == 0);

        // check behaviour of passing line out of scope
        const char* line_buf_out_of_scope = (const char*)file_reader_get_copy_of_line(fr_oneLiner, 2);
        assert(line_buf_out_of_scope == NULL);

        remove(file_name);
        file_reader_delete_copy_of_line((char*)line_buf);
        file_reader_delete_copy_of_file_buffer((char*)copied_content_of_file);
        file_reader_delete(fr_oneLiner);
    }

    /* 
        One-liner without new line sign at the end.
        Same checks as before since all functions (except size of file)
        should behave the same for case without \n.
    */
    {
        const char* file_name = "example_oneLiner_noNewLine_file.txt";
        FILE* example_file = fopen(file_name, "w+");
        assert(example_file != NULL);

        const char* file_content =  "Something else";
        const size_t content_size = strlen(file_content);
        fwrite(file_content, sizeof(char), content_size, example_file);
        fclose(example_file);

        File_Reader* fr_oneLiner = file_reader_new(file_name);
        assert(fr_oneLiner != NULL);

        const size_t size_of_file_in_bytes = file_reader_get_file_size(fr_oneLiner);
        assert(size_of_file_in_bytes == 14);

        // despite of /n at the end, there is just one line
        const size_t no_of_lines = file_reader_get_no_of_lines(fr_oneLiner);
        assert(no_of_lines == 1);

        const char* content_of_file = file_reader_get_file_buffer(fr_oneLiner);
        assert(content_of_file != NULL);

        const char* copied_content_of_file = (const char*)file_reader_get_copy_of_file_buffer(fr_oneLiner);
        assert(copied_content_of_file != NULL);

        // first line should equals the content of whole file since it one-liner
        const char* line_buf = (const char*)file_reader_get_copy_of_line(fr_oneLiner, 1);
        assert(strcmp(line_buf, file_content) == 0);

        // check behaviour of passing line out of scope
        const char* line_buf_out_of_scope = (const char*)file_reader_get_copy_of_line(fr_oneLiner, 2);
        assert(line_buf_out_of_scope == NULL);

        remove(file_name);
        file_reader_delete_copy_of_line((char*)line_buf);
        file_reader_delete_copy_of_file_buffer((char*)copied_content_of_file);
        file_reader_delete(fr_oneLiner);
    }

    /*
        Empty lines inside of file.
        Check if these empty lines does not change the behaviour of main functionalities.
        Check if getting empty line is secured.
    */
    {
        const char* file_name = "example_emptyLinesInside_file.txt";
        FILE* example_file = fopen(file_name, "w+");
        assert(example_file != NULL);

        const char* file_content =  "abcd\n"
                                    "\n"
                                    "\n"
                                    "15";

        const size_t content_size = strlen(file_content);
        fwrite(file_content, sizeof(char), content_size, example_file);
        fclose(example_file);

        File_Reader* fr_emptyLinesInside = file_reader_new(file_name);
        assert(fr_emptyLinesInside != NULL);

        const size_t size_of_file_in_bytes = file_reader_get_file_size(fr_emptyLinesInside);
        assert(size_of_file_in_bytes == 9);

        const size_t no_of_lines = file_reader_get_no_of_lines(fr_emptyLinesInside);
        assert(no_of_lines == 4);

        const char* content_of_file = file_reader_get_file_buffer(fr_emptyLinesInside);
        assert(content_of_file != NULL);

        const char* copied_content_of_file = (const char*)file_reader_get_copy_of_file_buffer(fr_emptyLinesInside);
        assert(copied_content_of_file != NULL);

        const char* line_buf = (const char*)file_reader_get_copy_of_line(fr_emptyLinesInside, 4);
        assert(strcmp(line_buf, "15") == 0);

        // if you try to get empty line of buffer it should returns NULL pointer to line buffer
        const char* line_buf_empty = (const char*)file_reader_get_copy_of_line(fr_emptyLinesInside, 3);
        assert(line_buf_empty == NULL);

        remove(file_name);
        file_reader_delete_copy_of_line((char*)line_buf);
        file_reader_delete_copy_of_file_buffer((char*)copied_content_of_file);
        file_reader_delete(fr_emptyLinesInside);
    }
}
