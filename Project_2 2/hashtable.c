#include "mysync.h"
#include "hashtable.h"
#include "files.h"

unsigned int hash_function(const char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % TABLE_SIZE;
}


void insert_into_hashtable(HashTable* table, const char* filename, time_t mod_time, int dir_index) {
    unsigned int index = hash_function(filename);  // Hash the full path

    FileEntry* current = table->buckets[index].head;


    while (current) {
        if (strcmp(current->name, filename) == 0) {  // Compare the full path
            current->exists_in_dir |= (1 << dir_index); // Set the bit for the directory

            if (mod_time > current->mod_time) {
                // If the new file is more recent, update the path and modification time
                strncpy(current->name, filename, sizeof(current->name));
                current->mod_time = mod_time;
            }
            return;
        }
        current = current->next;
    }

    // If we reached here, insert a new file entry
    FileEntry *new_entry = malloc(sizeof(FileEntry));
    strncpy(new_entry->name, filename, sizeof(new_entry->name));
    new_entry->mod_time = mod_time;
    new_entry->exists_in_dir = (1 << dir_index);
    new_entry->next = table->buckets[index].head;
    table->buckets[index].head = new_entry;

}


void identify_and_process_files(HashTable* table, int num_directories) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        FileEntry* current = table->buckets[i].head;

        while (current) {
            int dir_count = 0;
            int newest_dir = -1;
            time_t newest_time = 0;

            for (int j = 0; j < num_directories; j++) {
                if (current->exists_in_dir & (1 << j))
                {

                    dir_count++;
                    if (current->mod_time > newest_time) {
                        newest_time = current->mod_time;
                        newest_dir = j;
                    }
                }
            }

            if (dir_count == 1) {
                printf("File %s only exists in directory %d.\n", current->name, newest_dir + 1);
            } else if (dir_count > 1) {
                printf("File %s is newest in directory %d.\n", current->name, newest_dir + 1);
            }

            current = current->next;
        }
    }
}
void free_hashtable(HashTable* table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        FileEntry* current = table->buckets[i].head;
        while (current) {
            FileEntry* temp = current;
            current = current->next;
            free(temp);
        }
        table->buckets[i].head = NULL;  // Set the head to NULL after freeing all entries in this bucket
    }
}



