#ifndef FILE_READER_H
#define FILE_READER_H

#include <stddef.h>

// forward declaration
typedef struct File_Reader File_Reader;

File_Reader* file_reader_new(const char* file_name);
void         file_reader_delete(File_Reader* file_reader);
size_t       file_reader_get_file_size(const File_Reader* file_reader);
size_t       file_reader_get_no_of_lines(const File_Reader* file_reader);
const char*  file_reader_get_file_buffer(const File_Reader* file_reader);
char*        file_reader_get_copy_of_file_buffer(const File_Reader* file_reader);
void         file_reader_delete_copy_of_file_buffer(char* copy_buffer);
char*        file_reader_get_copy_of_line(const File_Reader* file_reader, const size_t line);
void         file_reader_delete_copy_of_line(char* line_buffer);

#endif // FILE_READER_H