#include <file_reader.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void file_reader_normal_file_test(void);
static void file_reader_virtual_file_test(void);
static void file_reader_empty_file_test(void);


int main(void)
{

    file_reader_normal_file_test();
    file_reader_virtual_file_test();
    file_reader_empty_file_test();

    return 0;
}

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
    
    size_t content_size = strlen(file_content);
    fwrite(file_content, sizeof(char), content_size, example_file);
    fclose(example_file);

    File_Reader* fr_normal = file_reader_new(file_name);

    const size_t size_of_file_in_bytes = file_reader_get_file_size(fr_normal);
    const size_t no_of_lines = file_reader_get_no_of_lines(fr_normal);
    const char* content_of_file = file_reader_get_file_buffer(fr_normal);
    char* copied_content_of_file = file_reader_get_copy_of_file_buffer(fr_normal);
    char* line_buf = file_reader_get_copy_of_line(fr_normal, 2);

    assert(fr_normal != NULL);
    assert(size_of_file_in_bytes > 0);
    assert(no_of_lines == 5);
    assert(content_of_file != NULL);
    assert(copied_content_of_file != NULL);
    assert(strcmp(line_buf, "cd") == 0);

    file_reader_delete_copy_of_line(line_buf);
    file_reader_delete_copy_of_file_buffer(copied_content_of_file);
    file_reader_delete(fr_normal);
}

static void file_reader_virtual_file_test(void)
{
    File_Reader* fr_virtual = file_reader_new("/proc/stat");

    const size_t size_of_file_in_bytes = file_reader_get_file_size(fr_virtual);
    const size_t no_of_lines = file_reader_get_no_of_lines(fr_virtual);
    const char* content_of_file = file_reader_get_file_buffer(fr_virtual);
    char* copied_content_of_file = file_reader_get_copy_of_file_buffer(fr_virtual);
    char* line_buf = file_reader_get_copy_of_line(fr_virtual, 3);
    char fist_4_of_line_buf[5] = {0}; // one more element in array for '\0'
    strncpy(fist_4_of_line_buf, line_buf, 4);

    assert(fr_virtual != NULL);
    assert(size_of_file_in_bytes > 0);
    assert(no_of_lines > 10);  // certainly more than 10 lines
    assert(content_of_file != NULL);
    assert(copied_content_of_file != NULL);
    assert(strcmp(fist_4_of_line_buf, "cpu1") == 0);

    file_reader_delete_copy_of_line(line_buf);    
    file_reader_delete_copy_of_file_buffer(copied_content_of_file);
    file_reader_delete(fr_virtual);
}

static void file_reader_empty_file_test(void)
{
    const char* file_name = "example_file_empty.txt";
    FILE* example_file = fopen(file_name, "w+");
    assert(example_file != NULL);

    const char* file_content =  "";
    
    size_t content_size = strlen(file_content);
    fwrite(file_content, sizeof(char), content_size, example_file);
    fclose(example_file);
    
    File_Reader* fr_empty =
        file_reader_new(file_name);

    assert(fr_empty == NULL);
}
