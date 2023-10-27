#ifndef HASHTABLE_H
#define HASHTABLE_H

// Function declarations and other definitions
void insert_into_hashtable(HashTable* table, const char* filename, time_t mod_time, int dir_index);
void identify_and_process_files(HashTable *table, int num_dir);
void free_hashtable(HashTable *table);
void print_hashtable(HashTable *table, int num_dir);
const char *extract_first_level_filename(const char *path);
#endif // HASHTABLE_H
