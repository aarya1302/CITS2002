#include <stdio.h>
#include <string.h>

// Define a structure to represent device information
struct DeviceInfo {
    char devicename[32];
    long long readspeed;
    long long writespeed;
};

long timeQ;


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

int main(int argc, char *argv[]) {

    read_sysconfig(argv[1]);

    return 0;
}