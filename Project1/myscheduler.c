#include <stdio.h>
#include <string.h>

// Define a structure to represent device information
struct DeviceInfo {
    char devicename[32];
    long long readspeed;
    long long writespeed;
};

long timeQ;



#define MAX_COMMANDS 32
#define MAX_CALLS 32

// Define a data structure to represent a system call
struct SystemCall {
    unsigned int elapsed_time; // Elapsed time in microseconds
    char action[32];           // System call action (e.g., sleep, write, read)
    char target[32];           // Target (e.g., terminal, hd)
    unsigned int data_size;   // Data size (e.g., 2000B)
    unsigned int wait; 
    char spawn_call[32];  
};

// Define a data structure to represent a command
struct Command {
    char name[32];                  // Command name (e.g., shortsleep)
    struct SystemCall calls[MAX_CALLS];  // Array of system calls
    size_t num_calls;               // Number of system calls
};


void read_sysconfig(char filename[]){
    FILE *file = fopen(filename, "r"); // Replace "input.txt" with your file name
    if (file == NULL) {
        perror("Error opening file");

    }

    char line[256];
    int numDevices = 0;

    // Count the number of devices in the file (excluding comment lines)
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] != '#' && strlen(line) > 1) {
            numDevices++;
        }
    }



    // Rewind the file to the beginning
    rewind(file);

    // Use a fixed-size array for DeviceInfo structures (adjust the size accordingly)
    #define MAX_DEVICES 10
    struct DeviceInfo devices[MAX_DEVICES];

    // Read and parse device information
    int index = 0;
    while (fgets(line, sizeof(line), file) != NULL && index < MAX_DEVICES) {
        if (line[0] != '#' && strlen(line) > 1) {
            char devicename[32];
            long long readspeed, writespeed;

            // Parse the line manually (assuming the format is consistent)
            // 3 is the number of information being extracted from each line 

            if (sscanf(line, "device %31s %lldBps %lldBps", devicename, &readspeed, &writespeed) == 3 ) {
                strcpy(devices[index].devicename, devicename);
                devices[index].readspeed = readspeed;
                devices[index].writespeed = writespeed;
                index++;
            }
            if(sscanf(line, "timequantum %ldusec", &timeQ) == 1){
                timeQ = timeQ;
            }
        }
    }
    // Close the file
    fclose(file);

    // Print the parsed device information
    for (int i = 0; i < index; i++) {

        printf("Device Name: %s\n", devices[i].devicename);
        printf("Read Speed: %lld Bps\n", devices[i].readspeed);
        printf("Write Speed: %lld Bps\n\n", devices[i].writespeed);
        printf("Time quantum: %ldusec", timeQ);

    }


}

void read_cmdfile(char filename[]){ 
    FILE *file = fopen(filename, "r"); // Replace "commands.txt" with your file name
    if (file == NULL) {

        perror("Error opening file");
        return 1;
    }


    char line[256];
    struct Command commands[MAX_COMMANDS];
;
    size_t num_commands = 0;
    size_t current_command_index = 0;

    // Read and parse the command file
    while (fgets(line, sizeof(line), file) != NULL) {

        if (line[0] != '#' && strlen(line) > 1) {
            unsigned int elapsed_time;
            char action[32];
            char target[32];
            unsigned int data_size;
            char command_name[32];
            unsigned int wait; 
            char spawn_call[32];



            size_t call_index = commands[current_command_index].num_calls;



            if (sscanf(line, "%uusecs %31s %31s %uB", &elapsed_time, action, target, &data_size) == 4) {
                // Handle regular system calls
                size_t call_index = commands[current_command_index].num_calls++;
                commands[current_command_index].calls[call_index].elapsed_time = elapsed_time;
                strcpy(commands[current_command_index].calls[call_index].action, action);
                strcpy(commands[current_command_index].calls[call_index].target, target);
                commands[current_command_index].calls[call_index].data_size = data_size;
            } else if (sscanf(line, "%uusecs %31s %uusecs", &elapsed_time, action, &wait) == 3) {
                // Handle wait system calls
                size_t call_index = commands[current_command_index].num_calls++;
                commands[current_command_index].calls[call_index].elapsed_time = elapsed_time;
                strcpy(commands[current_command_index].calls[call_index].action, action);
                commands[current_command_index].calls[call_index].wait = wait;
            } else if (sscanf(line, "%uusecs %31s %31s", &elapsed_time, action, spawn_call) == 3) {
                // Handle spawn system calls
                size_t call_index = commands[current_command_index].num_calls++;
                commands[current_command_index].calls[call_index].elapsed_time = elapsed_time;
                strcpy(commands[current_command_index].calls[call_index].action, action);
                strcpy(commands[current_command_index].calls[call_index].spawn_call, spawn_call);
            } else if (sscanf(line, "%uusecs %31s", &elapsed_time, action) == 2) {
                // Handle system calls with no additional parameters
                size_t call_index = commands[current_command_index].num_calls++;
                commands[current_command_index].calls[call_index].elapsed_time = elapsed_time;
                strcpy(commands[current_command_index].calls[call_index].action, action);
            } else if (sscanf(line, "%31s", command_name) == 1) {
                // Handle command name lines
                current_command_index = num_commands;
                num_commands++;

                // Store the command name
                strcpy(commands[current_command_index].name, command_name);
                commands[current_command_index].num_calls = 0; // Initialize the number of calls
            
            }

        }
    }

    // Close the file
    fclose(file);
}


int main(int argc, char *argv[]) {
    
    


    // Print the parsed commands and system calls

    for (size_t i = 0; i < num_commands; i++) {
        printf("Command Name: %s\n", commands[i].name);
        printf("Number of System Calls: %zu\n", commands[i].num_calls);

        for (size_t j = 0; j < commands[i].num_calls; j++) {
            printf("  Elapsed Time: %uusecs\n", commands[i].calls[j].elapsed_time);
            printf("  Action: %s\n", commands[i].calls[j].action);
            printf("  Target: %s\n", commands[i].calls[j].target);
            printf("  Data Size: %uB\n", commands[i].calls[j].data_size);
        }
    }

    return 0;
}