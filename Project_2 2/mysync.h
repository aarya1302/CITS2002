#ifndef MYSYNC_H
#define MYSYNC_H
#include <libgen.h>  
#include <stdbool.h>
#include <regex.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "glob2regex.h"
#include <fcntl.h>
#include <unistd.h>


#define MAX_DIRECTORIES 100
#define BUFFER_SIZE 4096
#define MAX_REGEX 100

// Command Options struct
typedef struct {
    bool all_files;
    char **ignore_patterns;
    regex_t* ignore_regex;
    int num_ignore_patterns;
    regex_t* only_regex;
    bool no_action;
    char **only_patterns;
    int num_only_patterns;
    bool preserve_attrs;
    bool recursive;
    bool verbose;
} CommandOptions;


// Directory struct
typedef struct {
    char *path;
} Directory;

// File struct
typedef struct {
    char *name;
    char *path;
    size_t size;
    time_t modification_time;
} File;

#define TABLE_SIZE 1000 // You can change this depending on the expected number of files

typedef struct file_entry {
    char name[1024];       // file name with full path 
    time_t mod_time;   // Modification times for each directory
    unsigned int exists_in_dir;         // Bitset to track which directories the file exists in
    struct file_entry* next; 
} FileEntry;

typedef struct hashtable_bucket {
    FileEntry* head;      // head of the linked list in this bucket
} HashTableBucket;

typedef struct hashtable {
    HashTableBucket buckets[TABLE_SIZE];
} HashTable;



// Pattern struct
typedef struct {
    char *original_pattern;
    regex_t compiled_regex;
} Pattern;

// Function prototypes and other definitions can be added here.

#endif // MYSYNC_H
