#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototype
void displayCacheInput(int cacheSize, int blockSize, int associativity, char* replacementPolicy, int physicalMemory, float percentUsed, int instructions, char* filename[], int fileNumber);


// USAGE EXAMPLE: VMCacheSim.exe –s 512 –b 16 –a 4 –r rr –p 1024 –n 100 –u 75 -f Trace1.trc -f Trace2_4Evaluation.trc –f Corruption1.trc
// Must accept all flags including up to 3 '-f's to read 3 trace files, 20 flags

int main(int argc, char* argv[]){
    
    // Input Parameters
    int cacheSize, blockSize, associativity, physicalMemory, instructionsTimeSlice;
    char* replacementPolicy;
    float percentUsed;
    char* files[3];

    // Check argument count for minimum amount of arguments
    if(argc < 17){
        printf("\nInsufficient amount of arguments.\n");
        printf("\nUsage: VMCacheSim.exe -s 512 -b 16 -a 4 -r rr -p 1024 -n 100 -u 75 -f Trace1.trc -f Trace2_4Evaluation.trc -f Corruption1.trc");
        printf("\n-s <cache size - KB>\n-b <block size>\n-a <associativity>\n-r <replacement policy>\n-p <physical memory - MB>\n-n <Instructions / Time Slice\n-u <%% of physical mem used by OS>\n-f <trace file name> **Up to three**\n\n");
        return 1;
    }

    // Carve argument values from argv and convert from ascii to int
    cacheSize = atoi(argv[2]);
    blockSize = atoi(argv[4]);
    associativity = atoi(argv[6]);
    replacementPolicy = argv[8];
    physicalMemory = atoi(argv[10]);
    instructionsTimeSlice = atoi(argv[12]);
    percentUsed = atoi(argv[14]);

    // Depending on how many files were provided we send off the files for calculations
    switch(argc){
        // One file provided
        case 17:
            files[0] = argv[16];
            displayCacheInput(cacheSize, blockSize, associativity, replacementPolicy, physicalMemory, percentUsed, instructionsTimeSlice, files, 1);
            break;
        
        // Two files provided
        case 19:
            files[0] = argv[16];
            files[1] = argv[18];
            displayCacheInput(cacheSize, blockSize, associativity, replacementPolicy, physicalMemory, percentUsed, instructionsTimeSlice, files, 2);
            break;

        // Three files provided
        case 21:
        files[0] = argv[16];
        files[1] = argv[18];
        files[2] = argv[20];
        displayCacheInput(cacheSize, blockSize, associativity, replacementPolicy, physicalMemory, percentUsed, instructionsTimeSlice, files, 3);
        break;

        default:
            printf("Error: invalid number of arguments or too many files provided\n");
            exit(1);
    }  

    return 0;

}

void displayCacheInput(int cacheSize, int blockSize, int associativity, char* replacementPolicy, int physicalMemory, float percentUsed, int instructions, char* filename[], int fileNumber){

    printf("Cache Simulator - CS 3853 - Team #03\n");
    printf("Trace File(s):\n");

        // Depending on how many files were provided, show the necessary result
    switch(fileNumber){
        case 1:
        printf("   %s\n", filename[0]);
        break;

        case 2:
        printf("   %s\n   %s \n", filename[0], filename[1]);
        break;
        
        case 3:
        printf("   %s\n   %s \n   %s\n", filename[0], filename[1], filename[2]);
        break;
    }

    // Display argument information in provided format
    printf("\n***** Cache Input Parameters *****\n\n");
    printf("Cache Size:                     %d KB\n", cacheSize);
    printf("Block Size:                     %d bytes\n", blockSize);
    printf("Associativity:                  %d\n", associativity);
    printf("Replacement Policy:             Round Robin\n");
    printf("Physical Memory:                %d MB\n", physicalMemory);
    printf("Percent Memory Used by System:  %.2f %%\n", percentUsed);
    printf("Instructions / Time slice:      %d\n", instructions);

}

void cacheCalculations(){


}