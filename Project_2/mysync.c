#include "mysync.h"
#include "hashtable.h"
#include "files.h"



// Function to display the usage of the program
void usage() {
    printf("Usage: ./mysync [options] directory1 directory2 [directory3 ...]\n");
    printf("Options:\n");
    printf("  -a        Consider all files for synchronization.\n");
    printf("  -i        Ignore filenames matching the pattern.\n");
    printf("  -n        Identify but do not synchronize files.\n");
    printf("  -o        Only consider filenames matching the pattern.\n");
    printf("  -p        Preserve file modification time and permissions.\n");
    printf("  -r        Recursively process directories.\n");
    printf("  -v        Display verbose output.\n");
    exit(EXIT_FAILURE);
}


#define BUFFER_SIZE 4096

void copy_file(const char *source_path, const char *dest_path) {
    FILE *source_file, *dest_file;
    char buffer[BUFFER_SIZE];
    size_t bytes;

    // Open source file for reading
    source_file = fopen(source_path, "rb");
    if (source_file == NULL) {
        perror("Failed to open source file for reading");
        return;
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
}


// Function to check if the given path is a readable directory
bool is_readable_directory(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return false;  // Couldn't get the status of the path
    }

    // Check if it's a directory
    if (!S_ISDIR(path_stat.st_mode)) {
        return false;
    }

    // Check if the directory is readable
    if (access(path, R_OK) != 0) {
        return false;  // Directory is not readable
    }

    return true;
}

// lists files and adds them to hash table
void list_files(HashTable* table, const char *directory_path, bool show_all, int dir_index) {
    DIR *dir = opendir(directory_path);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            // If not showing all and it's a hidden file, skip it
            continue;
        }

        // Construct the full path of the entry
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);

        struct stat entry_stat;
        if (stat(full_path, &entry_stat) == 0 && S_ISREG(entry_stat.st_mode)) {
            // If it's a regular file, print its name
            printf("%s\n", full_path);
            // adds file to hash table 
            insert_into_hashtable(table, full_path, entry_stat.st_mtime, dir_index);
        }
    }

    closedir(dir);
}
void synchronize_directories(HashTable* table, bool n_option, const char* dir1, const char* dir2 ) {
    if (n_option) return; // Don't perform any copying if -n option is provided

    for (int i = 0; i < TABLE_SIZE; i++) {
        FileEntry* current = table->buckets[i].head;
        while (current) {
            // Check the bitset to determine which directories the file exists in
            bool exists_in_dir1 = current->exists_in_dir & (1 << 0);
            bool exists_in_dir2 = current->exists_in_dir & (1 << 1);

            
            printf("dir1 : %s \n", dir1);
            printf("dir2 : %s \n", dir2);
            // Copy the file from dir1 to dir2
            char dest[1024];  
            const char* filename = extract_first_level_filename(current->name);  // Extracts the filename from the full path

            if (exists_in_dir1 && !exists_in_dir2)
            {


                // Build the destination path
                snprintf(dest, sizeof(dest), "%s/%s", dir2, filename);
                printf("this is the destination : %s \n", dest);

                copy_file(current->name, dest);

            }
            else if (!exists_in_dir1 && exists_in_dir2)
            {
                printf("in dir two \n");
                snprintf(dest, sizeof(dest), "%s/%s", dir1, filename);
                printf("this is the destination : %s \n", dest);

                copy_file(current->name, dest);
                // Copy the file from dir2 to dir1
                // ... [Your copy logic here]
            }
            else if (exists_in_dir1 && exists_in_dir2)
            {
                printf("exists in both %s \n", current->name);
                if(strstr(current->name, dir1) != NULL){
                    printf("newest one is in dir 1 %s\n", dir1);
                    snprintf(dest, sizeof(dest), "%s/%s", dir2, filename);
                    printf("this is the destination : %s \n", dest);


                }
                else
                {
                    printf("newest one is in dir2 %s\n", dir2);
                    snprintf(dest, sizeof(dest), "%s/%s", dir1, filename);
                    printf("this is the destination : %s \n", dest);

                    
                }
                printf("Copying from: %s to %s\n", current->name, dest);

                copy_file(current->name, dest);
                // Both directories have the file. Check mod_time to decide which one is newer
                // If both files have the same mod_time, no copying is needed
            }

            current = current->next;
        }
    }
}




int main(int argc, char *argv[]) {

    int opt;
    CommandOptions options = {false, NULL, 0, false, NULL, 0, false, false, false};

    while ((opt = getopt(argc, argv, "ai:no:prv")) != -1) {
        switch (opt) {
            case 'a':
                options.all_files = true;
                break;
            case 'i':
                // You might want to handle storing multiple patterns here
                options.ignore_patterns = &optarg;
                options.num_ignore_patterns++;
                break;
            case 'n':
                options.no_action = true;
                options.verbose = true;  // As per the specification
                break;
            case 'o':
                // Handle storing multiple patterns here as well
                options.only_patterns = &optarg;
                options.num_only_patterns++;
                break;
            case 'p':
                options.preserve_attrs = true;
                break;
            case 'r':
                options.recursive = true;
                break;
            case 'v':
                options.verbose = true;
                break;
            default:
                usage();
        }
    }

    bool showall = options.all_files;
    int num_dir = argc - optind;

    // Ensure at least two directories are provided
    if (argc - optind < 2) {
        printf("Error: At least two directories must be specified.\n");
        usage();
    }
    HashTable table = {0}; // Initialize to zero

    // Ensure provided paths are readable directories
    for (int i = optind; i < argc; i++) {
        if (!is_readable_directory(argv[i])) {
            fprintf(stderr, "Error: '%s' is not a readable directory.\n", argv[i]);
            exit(EXIT_FAILURE);
        }else{
            printf("\nFiles in directory: %s\n", argv[i]);
            int dir_index = i - optind;

            list_files(&table, argv[i], showall, dir_index);


        }
    }


    // Print regular files for the first two directories

    //identify_and_process_files(&table, num_dir);

    const char *dir1 = argv[optind];
    const char *dir2 = argv[optind + 1];
    printf("this are the directories: dir1 %s , dir 2, %s \n", dir1, dir2);

    synchronize_directories(&table, options.no_action, dir1, dir2);
    // For simplicity, assuming only files (not directories)
    // Also, not checking for newer files, just copying if -n is not set
    print_hashtable(&table, num_dir);

    // Free the memory for hashtable entries
    free_hashtable(&table);

    return 0;


    return 0;
}
