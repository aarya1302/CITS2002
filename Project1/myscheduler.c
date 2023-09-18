#include <stdio.h>
#include <string.h>
#include <stdbool.h>


// Define constants for process states
#define STATE_READY 0
#define STATE_RUNNING 1
#define STATE_BLOCKED_IO 2
#define STATE_SLEEPING 3
#define STATE_BLOCKED_WAIT 4
#define STATE_DONE 5

#define MAX_COMMANDS 32
#define MAX_CALLS 32
#define MAX_PROCESSES 20
#define MAX_DEVICES 10
#define MAX_QUEUE_SIZE 30 

const int READY_TO_RUNNING = 5;  // microseconds
const int RUNNING_TO_BLOCKED_TIME = 10;  // microseconds
const int BLOCKED_TO_READY_TIME = 10;  // microseconds
const int RUNNING_TO_READY_TIME = 10;  // microseconds
const int DATA_BUS_ACQUIRE_TIME = 20;  // microseconds

///////////////////////////////////////////////////////////////////////
// Define a structure to represent device information
struct DeviceInfo {
    char devicename[32];
    long long readspeed;
    long long writespeed;
};

long timeQ; // time quantum global variable
long num_cmds; // number of commands

// Define a data structure to represent a system call
struct SystemCall {
    unsigned int elapsed_time; // Elapsed time in microseconds
    char action[32];           // System call action (e.g., sleep, write, read)
    char target[32];           // Target (e.g., terminal, hd)
    unsigned int data_size;   // Data size (e.g., 2000B)
    unsigned int sleep; 
    char spawn_command[32];  
    unsigned int state;
    unsigned int time_remain;
};

// Define a data structure to represent a command
struct Command {
    char name[32];                  // Command name 
    struct SystemCall calls[MAX_CALLS];  // Array of system calls
     size_t num_calls; 

};
/////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////
// Creating the Queue ADT 

typedef struct {
    struct SystemCall* data[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

// Function to initialize a queue
void initQueue(Queue* queue) {
    queue->front = -1;
    queue->rear = -1;
    queue->size = 0;
}

// Function to check if the queue is empty
int isEmpty(Queue* queue) {
    return (queue->front == -1 && queue->rear == -1);
}

int isFull(Queue* queue) {
    return (queue->size == MAX_QUEUE_SIZE);
}
// Function to enqueue an element
void enqueue(Queue* queue, struct SystemCall* syscall) {
    if (isFull(queue)) {
        printf("Queue is full. Cannot enqueue.\n");
        return;
    }

    if (isEmpty(queue)) {
        queue->front = 0;
    }

    queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
    queue->data[queue->rear] = syscall;
    queue->size++;
}

// Function to dequeue an element
struct SystemCall* dequeue(Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty. Cannot dequeue.\n");
        return NULL;
    }

    struct SystemCall* dequeuedSyscall = queue->data[queue->front];
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    queue->size--;

    if (queue->size == 0) {
        queue->front = -1;
        queue->rear = -1;
    }

    return dequeuedSyscall;
}
// Function to display the queue

//////////////////////////////////////////////////////////////////////////////////////
// Use a fixed-size array for DeviceInfo structures (adjust the size accordingly)
struct DeviceInfo devices[MAX_DEVICES];
// Function to parse sysconfig file

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
   
    for (int i = 0; i < index; i++) {

    printf("Device Name: %s\n", devices[i].devicename);
    printf("Read Speed: %lld Bps\n", devices[i].readspeed);
    printf("Write Speed: %lld Bps\n", devices[i].writespeed);
    printf("Time quantum: %ldusec", timeQ);

    }

    // Close the file
    fclose(file);


}
///////////////////////////////////////////////////////////////////////////////////////
// Function to read and parse the command files
struct Command commands[MAX_COMMANDS];

void read_cmdfile(char filename[]){ 
    FILE *file = fopen(filename, "r"); // Replace "commands.txt" with your file name
    if (file == NULL) {

        perror("Error opening file");
    }


    char line[256];

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
            unsigned int sleep; 
            char spawn_command[32];



            size_t call_index = commands[current_command_index].num_calls;



            if (sscanf(line, "%uusecs %31s %31s %uB", &elapsed_time, action, target, &data_size) == 4) {
                // Handle regular system calls
                size_t call_index = commands[current_command_index].num_calls++;
                commands[current_command_index].calls[call_index].elapsed_time = elapsed_time;
                commands[current_command_index].calls[call_index].time_remain = elapsed_time;
                strcpy(commands[current_command_index].calls[call_index].action, action);
                strcpy(commands[current_command_index].calls[call_index].target, target);
                commands[current_command_index].calls[call_index].data_size = data_size;
            } else if (sscanf(line, "%uusecs %31s %uusecs", &elapsed_time, action, &sleep) == 3) {
                // Handle wait system calls
                size_t call_index = commands[current_command_index].num_calls++;
                commands[current_command_index].calls[call_index].elapsed_time = elapsed_time;
                 commands[current_command_index].calls[call_index].time_remain = elapsed_time;
                strcpy(commands[current_command_index].calls[call_index].action, action);
                commands[current_command_index].calls[call_index].sleep = sleep;
            } else if (sscanf(line, "%uusecs %31s %31s", &elapsed_time, action, spawn_command) == 3) {
                // Handle spawn system calls
                size_t call_index = commands[current_command_index].num_calls++;
                commands[current_command_index].calls[call_index].elapsed_time = elapsed_time;
                 commands[current_command_index].calls[call_index].time_remain = elapsed_time;
                strcpy(commands[current_command_index].calls[call_index].action, action);
                strcpy(commands[current_command_index].calls[call_index].spawn_command, spawn_command);
            } else if (sscanf(line, "%uusecs %31s", &elapsed_time, action) == 2) {
                // Handle system calls with no additional parameters
                size_t call_index = commands[current_command_index].num_calls++;
                commands[current_command_index].calls[call_index].elapsed_time = elapsed_time;
                 commands[current_command_index].calls[call_index].time_remain = elapsed_time;
                strcpy(commands[current_command_index].calls[call_index].action, action);
            } else if (sscanf(line, "%31s", command_name) == 1) {
                // Handle command name lines
                current_command_index = num_commands;
                num_commands++;
                num_cmds = (int)num_commands;

                // Store the command name
                strcpy(commands[current_command_index].name, command_name);
                commands[current_command_index].num_calls = 0; // Initialize the number of calls
            
            }

        }
    }
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




    // Close the file
    fclose(file);


}


int main(int argc, char *argv[]) {
    

   


    //Parsing command file and sys config files 
    read_sysconfig(argv[1]);
    read_cmdfile(argv[2]);
    
     // Initialize CPU clock speed and other system parameters
    int clockspeed = 200000000;
    int timequantum = timeQ; 
    int total_time = 0; 
    int cpu_time = 0; 
    float percentageCPU = 0; 

    // Maintain separate queues for different blocked states
    Queue ioBlockQueue, sleepBlockQueue, waitBlockQueue, readyQueue;
    initQueue(&ioBlockQueue);
    initQueue(&sleepBlockQueue);
    initQueue(&waitBlockQueue);
    initQueue(&readyQueue);

    bool processes_running = true;

    while (processes_running) {
        processes_running = false;

        for(int i = 0; i < num_cmds; ++i){

            struct Command curr_command = commands[i];
            int time_used = 0; 

            printf("///////////////////////// this is the current command : %s", curr_command.name);
             int num_calls = commands[i].num_calls;
            //Enqueueing first  processe for  command into the Ready Queue
            for(int j = 0; j < num_calls; ++j){ 


                struct SystemCall* process = &(curr_command.calls[j]);
                process->time_remain -= time_used;
                enqueue(&(readyQueue), process);
                printf("/////////%d time remainign", time_used);
                


                // Dequeueing and running each process in the Ready Queue until queue is empty

                while(!isEmpty(&readyQueue)){
                
                    struct SystemCall* ready_process = dequeue(&readyQueue);
                    if (ready_process->state == STATE_READY){
                        total_time += READY_TO_RUNNING;

                    }

                    ready_process->state = STATE_RUNNING; 

                    printf("This is the size of the ready q: %d \n",readyQueue.size);
                    printf("this is the action %s\n", ready_process->action);
                    printf("time remaining : %d\n", ready_process->time_remain);
                    printf("\n total time : %d\n", total_time); 
                    printf("Dequeued process: %s, state: %d\n", ready_process->action, ready_process->state);

                  
                    // Handling exceeding the time quantum by the process
                    if (ready_process->time_remain >= timequantum){

                        total_time += RUNNING_TO_READY_TIME+ timequantum;
                        ready_process->time_remain -= timequantum;
                        time_used += timequantum;

                        ready_process->state = STATE_READY;
                        enqueue(&readyQueue, ready_process);



                    // handling sleep process
                    }else{
                         if(ready_process->sleep > 0){
                            printf("sleep process\n");
                            total_time += RUNNING_TO_BLOCKED_TIME + ready_process->time_remain;
                            time_used += ready_process->time_remain;
                            ready_process->time_remain = 0;
                            
                            ready_process->state = STATE_SLEEPING;
                            enqueue(&sleepBlockQueue, ready_process);


                        }else if (strcmp( ready_process->action, "exit") == 0){
                            total_time += ready_process->time_remain + 1;
                            ready_process->time_remain = 0;
                            cpu_time += ready_process->elapsed_time;
                            ready_process->state = STATE_DONE;

                            break;
                        }
                    }
                     // enqueueing all sleep processes 
                    while(!isEmpty(&sleepBlockQueue)){
                        struct SystemCall* sleep_process = dequeue(&sleepBlockQueue);
                        if (sleep_process->sleep > BLOCKED_TO_READY_TIME){
                           total_time += sleep_process->sleep;

                       }else{
                            total_time += BLOCKED_TO_READY_TIME + 2;
                        }
                    
                        sleep_process->sleep = 0;
                        enqueue(&readyQueue, sleep_process);
                    
                    }
                   


                }

                
            }
        
        }
            processes_running = false;


          


    }
            
    

    int cpu_util = cpu_time/total_time *100;
    printf("total time : %d \n", total_time);
    printf("cpu time : %d \n", cpu_time);
    


    return 0;
}