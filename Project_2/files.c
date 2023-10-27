#include "mysync.h"
#include "files.h"
#include <string.h>

const char* extract_first_level_filename(const char* path) {
    // Find the first '/'
    const char* first_slash = strchr(path, '/');


    // If found, take everything after it till the next '/'
    if (first_slash) {
        const char* next_slash = strchr(first_slash + 1, '/');
        if (next_slash) {
            // Copy the characters between the two slashes
            static char buffer[1024];
            strncpy(buffer, first_slash + 1, next_slash - first_slash - 1);
            buffer[next_slash - first_slash - 1] = '\0';
            return buffer;
        }
        return first_slash + 1;
    }
    return path;  // return the whole path if no '/' is found
}


