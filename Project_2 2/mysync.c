#include "mysync.h"
#include "hashtable.h"
#include "files.h"
#include "directories.h"
#include "patterns.h"



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
void list_files(HashTable* table, const char *directory_path, bool show_all, int dir_index,  bool recursive, CommandOptions options) {

    DIR *dir = opendir(directory_path);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }


    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!options.all_files && entry->d_name[0] == '.') {
            // If not showing all and it's a hidden file, skip it
            continue;
        }

        // Construct the full path of the entry
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);

        const char* filename = extract_first_level_filename(full_path);
        if(options.num_ignore_patterns > 0 || options.num_only_patterns > 0 ){
            bool should_ignore = matches_pattern(filename, options.ignore_regex, options.num_ignore_patterns);

            bool should_only = options.num_only_patterns == 0 || (full_path, options.only_regex, options.num_only_patterns);

            if (should_ignore || !should_only) {
                continue; // Skip this file
            }       
        }

        struct stat entry_stat;
        if (stat(full_path, &entry_stat) == 0) {
            if (S_ISREG(entry_stat.st_mode)) {
                // If it's a regular file and not a hidden file (or show_all is true), process it
                if (options.all_files || entry->d_name[0] != '.') {
                    printf("%s\n", full_path);
                    // adds file to hash table 
                    insert_into_hashtable(table, full_path, entry_stat.st_mtime, dir_index);
                }
            } else if (recursive && S_ISDIR(entry_stat.st_mode)) {
                // If recursive flag is set and it's a directory, traverse it recursively
                list_files(table, full_path, show_all, dir_index, recursive, options);
            }
        }
    }

    closedir(dir);
}


int main(int argc, char *argv[]) {


    int opt;
    int num_only_files = 0; 

    CommandOptions options;
options.all_files = false;
options.ignore_patterns = NULL;
options.ignore_regex = NULL;
options.num_ignore_patterns = 0;
options.only_regex = NULL;
options.no_action = false;
options.only_patterns = NULL;
options.num_only_patterns = 0;
options.preserve_attrs = false;
options.recursive = false;
options.verbose = false;



    bool ignore_pattern = false;
    bool n_opt = false;
    bool only_pattern = false;
    // printf("pajfdos %i ", options.num_only_patterns);

    while ((opt = getopt(argc, argv, "ai:no:prv")) != -1)
    {

        switch (opt)
        {
        case 'a':
            options.all_files = true;
            break;
        case 'i':

            options.ignore_patterns = &optarg;
            ignore_pattern = true;
            
            options.num_ignore_patterns++;
            break;
        case 'n':
            options.no_action = true;
            options.verbose = true; // As per the specification
            break;
        case 'o':
            // Handle storing multiple patterns here as well

            options.only_patterns = &optarg;
            //options.num_only_patterns = num_only_files;
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


    if (ignore_pattern)
    {
        char *regex_pattern = glob2regex(optarg);
        // Reallocate memory for the regex patterns
        options.ignore_regex = realloc(options.ignore_regex, sizeof(regex_t) * (options.num_ignore_patterns));
        int ret = regcomp(&options.ignore_regex[options.num_ignore_patterns -1], regex_pattern, REG_EXTENDED);
        free(regex_pattern);
    }

if (options.only_patterns != NULL ) {
    char *regex_pattern = glob2regex(optarg);
    options.only_regex = realloc(options.only_regex, sizeof(regex_t) * (options.num_only_patterns));
    regcomp(&options.only_regex[options.num_only_patterns], regex_pattern, REG_NOSUB);
    free(regex_pattern);
}


    bool showall = options.all_files;
    const char* directories[] = {};

    // Ensure at least two directories are provided
    if (argc - optind < 2) {
        printf("Error: At least two directories must be specified.\n");
        usage();
    }
    HashTable table = {0}; // Initialize to zero
    int num_dir = argc - optind;



    // Ensure provided paths are readable directories
    for (int i = optind; i < argc; i++) {

        if (!is_readable_directory(argv[i]))
        {
            fprintf(stderr, "Error: '%s' is not a readable directory.\n", argv[i]);
            exit(EXIT_FAILURE);
        }
        else
        {

            int dir_index = i - optind;


            directories[dir_index] = argv[i];

            // all the files are listed and added to the has table
            list_files(&table, argv[i], showall, dir_index, options.recursive, options);
        }
    }



    // Print regular files for the first two directories

    //identify_and_process_files(&table, num_dir);

    n_opt = options.no_action;
    synchronize_directories(&table, n_opt, directories, num_dir, options.preserve_attrs);
    // For simplicity, assuming only files (not directories)
    // Also, not checking for newer files, just copying if -n is not set
    //print_hashtable(&table, num_dir);

    // Free the memory for hashtable entries
    free_hashtable(&table);

    // At the end of the main function, before return:

    for (int i = 0; i < options.num_ignore_patterns; i++) {
        regfree(&options.ignore_regex[i]);
    }
    free(options.ignore_regex);

    for (int i = 0; i < options.num_only_patterns; i++) {
        regfree(&options.only_regex[i]);
    }
    free(options.only_regex);



    return 0;
}
