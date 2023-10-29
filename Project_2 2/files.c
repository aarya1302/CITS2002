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
#include <sys/stat.h>
#include <utime.h>

void copy_file(const char *source_path, const char *dest_path, bool preserve_attrs) {
    FILE *source_file, *dest_file;
    char buffer[BUFFER_SIZE];
    size_t bytes;

    struct stat source_stat;

    // Open source file for reading
    source_file = fopen(source_path, "rb");
    if (source_file == NULL) {
        perror("Failed to open source file for reading");
        return;
    }

    // Get the source file's attributes
    if (preserve_attrs) {
        if (stat(source_path, &source_stat) != 0) {
            perror("Failed to get source file attributes");
            fclose(source_file);
            return;
        }
    }

    // Open destination file for writing
    dest_file = fopen(dest_path, "wb");
    if (dest_file == NULL) {
        perror("Failed to open destination file for writing");
        fclose(source_file);  // Close the source file before returning
        return;
    }

    // Read from source file and write to destination file
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, source_file)) > 0) {
        fwrite(buffer, 1, bytes, dest_file);
    }

    // Close the files
    fclose(source_file);
    fclose(dest_file);



    if (preserve_attrs) {
        // Set the destination file's modification time to match the source's
        struct utimbuf new_times;
        new_times.actime = source_stat.st_atime;  // access time
        new_times.modtime = source_stat.st_mtime; // modification time

        if (utime(dest_path, &new_times) != 0) {
            perror("Failed to set destination file modification time");
        }

        // Set the destination file's permissions to match the source's
        if (chmod(dest_path, source_stat.st_mode) != 0) {
            perror("Failed to set destination file permissions");
        }
    }
}


