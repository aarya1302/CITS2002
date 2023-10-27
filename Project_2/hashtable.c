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

    unsigned int index = hash_function(filename);


    FileEntry* current = table->buckets[index].head;
    const char *extracted_filename = extract_first_level_filename(filename);
    while (current)
    {

        const char *extracted_current = extract_first_level_filename(current->name);
        if (strcmp(extracted_current, extracted_filename) == 0)
        {

            current->exists_in_dir |= (1 << dir_index); // Set the bit for the directory

            if (mod_time > current->mod_time)
            {
                // storing the path of the file that's the most recent

                strncpy(current->name,filename, sizeof(current->name));

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
   new_entry->exists_in_dir = (1 << (dir_index));

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

void print_hashtable(HashTable* table, int num_dir) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        FileEntry* current_entry = table->buckets[i].head;
        while (current_entry != NULL) {
            printf("Filename: %s\n", current_entry->name);
            
            // Print modification times for each directory


            // Print which directories the file exists in using the bitset
            printf("Exists in directories: ");
            for (int j = 0; j < num_dir; j++) {
                if (current_entry->exists_in_dir & (1 << j)) {
                    printf("%d ", current_entry->exists_in_dir);
                }
            }
            printf("\n");
            
            current_entry = current_entry->next; // Move to the next entry in the linked list
        }
    }
}
