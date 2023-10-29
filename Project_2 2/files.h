#include <string.h>

const char *extract_first_level_filename(const char* filename);
void copy_file(const char *source_path, const char *dest_path, bool preserve_attrs);