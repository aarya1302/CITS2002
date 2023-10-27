#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//  you may need other standard header files


//  CITS2002 Project 1 2023
//  Student1:   23277271   AARYA BHORRA



//  myscheduler (v1.0)
//  Compile with:  cc -std=c11 -Wall -Werror -o myscheduler myscheduler.c


//  THESE CONSTANTS DEFINE THE MAXIMUM SIZE OF sysconfig AND command DETAILS
//  THAT YOUR PROGRAM NEEDS TO SUPPORT.  YOU'LL REQUIRE THESE //  CONSTANTS
//  WHEN DEFINING THE MAXIMUM SIZES OF ANY REQUIRED DATA STRUCTURES.

#define MAX_DEVICES                     4
#define MAX_DEVICE_NAME                 20
#define MAX_COMMANDS                    10
#define MAX_COMMAND_NAME                20
#define MAX_SYSCALLS_PER_PROCESS        40
#define MAX_RUNNING_PROCESSES           50
#define MAX_QUEUE_SIZE 30 

//  NOTE THAT DEVICE DATA-TRANSFER-RATES ARE MEASURED IN BYTES/SECOND,
//  THAT ALL TIMES ARE MEASURED IN MICROSECONDS (usecs),
//  AND THAT THE TOTAL-PROCESS-COMPLETION-TIME WILL NOT EXCEED 2000 SECONDS
//  (SO YOU CAN SAFELY USE 'STANDARD' 32-BIT ints TO STORE TIMES).

#define DEFAULT_TIME_QUANTUM            100

#define TIME_CONTEXT_SWITCH             5
#define TIME_CORE_STATE_TRANSITIONS     10
#define TIME_ACQUIRE_BUS                20

// Define constants for process states
#define STATE_READY 0
#define STATE_RUNNING 1
#define STATE_BLOCKED_IO 2
#define STATE_SLEEPING 3
#define STATE_BLOCKED_WAIT 4
#define STATE_DONE 5

//  ----------------------------------------------------------------------

#define CHAR_COMMENT                    '#'
// Define a structure to represent device information
struct DeviceInfo {
    char devicename[32];
    long long readspeed;
    long long writespeed;
};

long timeQ = DEFAULT_TIME_QUANTUM; // time quantum global variable
long num_cmds; // number of commands

// Define a data structure to represent a system call
struct SystemCall {
    unsigned int elapsed_time; // Elapsed time in microseconds
    char action[32];           // System call action (e.g., sleep, write, read)
    char target[32];           // Target (e.g., terminal, hd)
    unsigned int data_size;   // Data size (e.g., 2000B)
    unsigned int sleep; 
    char spawned_command[32];  
    unsigned int state;
    unsigned int time_remain;
};

// Define a data structure to represent a command
struct Command {
    char name[32];                  // Command name 
    struct SystemCall calls[MAX_SYSCALLS_PER_PROCESS];  // Array of system calls
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
    while (fgets(line, sizeof(line), file) != NULL && index < MAX_DEVICES+1) {
        if (line[0] != '#' && strlen(line) > 1) {
            char devicename[32];
            long long readspeed, writespeed;
            long timequantum;

            // Parse the line manually (assuming the format is consistent)
            // 3 is the number of information being extracted from each line 

            if (sscanf(line, "device %31s %lldBps %lldBps", devicename, &readspeed, &writespeed) == 3 ) {
                strcpy(devices[index].devicename, devicename);
                devices[index].readspeed = readspeed;
                devices[index].writespeed = writespeed;
                index++;
            }
            if(sscanf(line, "timequantum %ldusec", &timequantum) == 1){
                if(timequantum > 0){
                    timeQ = timequantum;
                }
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
struct Command commands[MAX_COMMANDS];

void read_commands( char filename[]){
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
                strcpy(commands[current_command_index].calls[call_index].spawned_command, spawn_command);
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
}

//  ----------------------------------------------------------------------
int clockspeed = 200000000;
int total_time = 0; 
int cpu_time = 0; 
float percentageCPU = 0; 

// Maintain separate queues for different blocked states
Queue ioBlockQueue, sleepBlockQueue, waitBlockQueue, readyQueue;

struct Command find_command(char name[]){

    struct Command found_cmd;

    for (int i = 0; i < num_cmds; i++){

        if(strcmp(commands[i].name, name) == 0){
            found_cmd = commands[i];
        }
        
    }
    return found_cmd;

}
void execute_commands(struct Command curr_command)
{
    int time_used = 0; 

    printf("///////////////////////// this is the current command : %s", curr_command.name);
    int num_calls = curr_command.num_calls;
     //Enqueueing first  processe for  command into the Ready Queue
    for(int j = 0; j < num_calls; ++j){ 
                struct SystemCall* process = &(curr_command.calls[j]);
                process->time_remain -= time_used;
                enqueue(&(readyQueue), process);

                // Dequeueing and running each process in the Ready Queue until queue is empty
                    int count = 0; 
                
                while(!isEmpty(&readyQueue)){
                
                    struct SystemCall* ready_process = dequeue(&readyQueue);


                    if (ready_process->state == STATE_READY){
                        total_time += TIME_CONTEXT_SWITCH;
                        printf("ready to running time: %d total time %d\n", TIME_CONTEXT_SWITCH, total_time);

                    }
                    ready_process->state = STATE_RUNNING; 
                    printf("/////////%d time used\n", time_used);
                    printf("This is the size of the ready q: %d \n",readyQueue.size);
                    printf("this is the action %s\n", ready_process->action);
                    printf("time remaining : %d\n", ready_process->time_remain);
                    printf("total time : %d <------ \n", total_time); 
                    printf("state of process %d\n", ready_process->state);
                    printf("this is timeQ %ld \n", timeQ);

                    // calculating the time 

                    if(ready_process->time_remain > 0){
                         if (ready_process->time_remain >= timeQ){
                            printf("TIME Q BEING EXCEEDED  \n");
                            total_time += TIME_CORE_STATE_TRANSITIONS +timeQ;
                            ready_process->time_remain -= timeQ;
                            time_used += timeQ;
                            printf("time q exceeded+ Running to ready time %d + TQ : %d total time \n",TIME_CORE_STATE_TRANSITIONS,  total_time);

                            ready_process->state = STATE_READY;
                            enqueue(&readyQueue, ready_process);


                        } else { 
                            total_time += ready_process->time_remain ;
                            time_used += ready_process->time_remain ;
                            ready_process->time_remain = 0;
                            printf("exec of process %d; total time %d  \n", ready_process->time_remain,  total_time);

                        }
                    }
                    
                    // Executing actions
                    
                    if(ready_process->time_remain <= 0 && ready_process->state == STATE_RUNNING){
                        if (strcmp( ready_process->action, "exit") == 0){ 
                            

                            total_time += ready_process->time_remain + 1;
                            ready_process->time_remain = 0;
                            cpu_time += ready_process->elapsed_time;

                            printf("exiting process; %dtime remaining + 1 : %d total time \n",ready_process->time_remain, total_time);

                            break;
                        }else{
                         if(ready_process->sleep > 0){


                            total_time += TIME_CORE_STATE_TRANSITIONS + 1;

                            time_used += ready_process->time_remain;
                            printf("sleep process %d time remain + %d running blocked: %d total time \n",ready_process->time_remain, TIME_CORE_STATE_TRANSITIONS, total_time);
                            
                            ready_process->state = STATE_SLEEPING;

                            enqueue(&sleepBlockQueue, ready_process);
                            printf("sleep queue size : %d \n", sleepBlockQueue.size);


                        }else if (strcmp( ready_process->action, "spawn") == 0) {

                            struct Command spawned = find_command(ready_process->spawned_command);

                            total_time += TIME_CORE_STATE_TRANSITIONS ;

                            printf("%d spawned process before exec + %d running to ready time: %d total time \n",ready_process->time_remain, TIME_CORE_STATE_TRANSITIONS, total_time);


                            ready_process->time_remain = 0;

                            execute_commands(spawned);
                            printf("spawned process after exec : %d total time \n", total_time);



                        }else if (strcmp(ready_process->action, "wait") == 0){



                            total_time += TIME_CORE_STATE_TRANSITIONS + 1;


                            printf("%d wait process before exec + %d running to blocked time: %d total time \n",ready_process->time_remain, TIME_CORE_STATE_TRANSITIONS, total_time);

                            
                            ready_process->state = STATE_BLOCKED_WAIT;
                            enqueue(&waitBlockQueue, ready_process);
                            

                        }
                         

    
                        
                        }   

                    }



                        if((!isEmpty(&sleepBlockQueue)) == 1){

                            struct SystemCall* sleep_process = dequeue(&sleepBlockQueue);
                            if (sleep_process->sleep > TIME_CORE_STATE_TRANSITIONS){
                                total_time += sleep_process->sleep;
                                printf("%d sleep process; %d total time \n",ready_process->sleep,total_time);

                            }else{
                                    total_time += TIME_CORE_STATE_TRANSITIONS + 1;
                                    printf("%d sleep process; %d total time \n", TIME_CORE_STATE_TRANSITIONS, total_time);
                            }
                            sleep_process->state = STATE_DONE;
                    
                            sleep_process->sleep = 0;

                    
                        }
                        if(!isEmpty(&waitBlockQueue)){
                             struct SystemCall* wait_process = dequeue(&waitBlockQueue);
                            wait_process->state = STATE_READY;

                            total_time += TIME_CORE_STATE_TRANSITIONS;
                            printf("wait blocked process:+= %d blocked to ready time: %d  + 1 total time \n", TIME_CORE_STATE_TRANSITIONS, total_time);


                            
                    
                        }   


                }
                

                
            }
}

//  ----------------------------------------------------------------------

int main(int argc, char *argv[])
{
//  ENSURE THAT WE HAVE THE CORRECT NUMBER OF COMMAND-LINE ARGUMENTS
    if(argc != 3) {
        printf("Usage: %s sysconfig-file command-file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

//  READ THE SYSTEM CONFIGURATION FILE
    read_sysconfig( argv[1]);

//  READ THE COMMAND FILE
    read_commands(argv[2]);

    
    // initializing the queues
    initQueue(&ioBlockQueue);
    initQueue(&sleepBlockQueue);
    initQueue(&waitBlockQueue);
    initQueue(&readyQueue);



//  EXECUTE COMMANDS, STARTING AT FIRST IN command-file, UNTIL NONE REMAIN
    execute_commands(commands[0]);
    double cpu = cpu_time;

    int percentage = ((float)cpu_time / total_time) * 100;


//  PRINT THE PROGRAM'S RESULTS
    printf("measurements  %i  %d\n", total_time, percentage);

    exit(EXIT_SUCCESS);
}

//  vim: ts=8 sw=4