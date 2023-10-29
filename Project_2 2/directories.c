

#include "mysync.h"
#include "files.h"

#include "directories.h"

void synchronize_directories(HashTable* table, bool n_option, const char** dirs, int num_dir, bool preserve) {

    for (int i = 0; i < TABLE_SIZE; i++) {
        FileEntry* current = table->buckets[i].head;
        char dest[1024];

        while (current) {
            const char* filename = extract_first_level_filename(current->name);  // Extracts the filename from the full path

            struct stat path_stat;
            if (stat(current->name, &path_stat) != 0) {
                perror("Failed to get path status");
                current = current->next;
                continue;  // Skip to the next entry
            }

            for (int j = 0; j < num_dir; j++) {
                snprintf(dest, sizeof(dest), "%s/%s", dirs[j], filename);

                if (S_ISDIR(path_stat.st_mode)) {
                    // Path is a directory
                    if (!(current->exists_in_dir & (1 << j))) {
                        mkdir(dest, 0755);  // Create the directory if it doesn't exist
                    }


                } else if (S_ISREG(path_stat.st_mode)) {
                    // Path is a file
                    if (current->exists_in_dir & (1 << j)) {
                        if (strstr(current->name, dirs[j]) == NULL) {
                            printf("Copying file from %s to %s\n", current->name, dest);
                            if(!n_option){
                                copy_file(current->name, dest, preserve);

                            }
                        }
                    } else {
                        printf("Copying file from %s to %s\n", current->name, dest);
                        if(!n_option){
                                copy_file(current->name, dest, preserve);

                        }
                    }
                }
            }

            current = current->next;
        }
    }
}
