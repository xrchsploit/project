#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Group #3: Antonio Charfauros, Paul Castillo, Cain Green, Abrianna Garcia

// Structs

// Arugment struct
typedef struct {
    int cacheSize;
    int blockSize;
    int associativity;
    char *replacementPolicy;
    int physicalMemory;
    float percentUsed;
    int instructionsTimeSlice;
    int fileCount;
    char *files[3];
} CacheInput;

// Cache calc struct
typedef struct {
    unsigned int cacheSizeBytes;
    int totalBlocks;
    int totalRows;
    int offsetBits;
    int indexBits;
    int physicalAddressBits;
    int tagBits;
    int overheadBitsPerBlock;
    unsigned int overheadSizeBytes;
    unsigned int implementationMemoryBytes;
    double implementationMemoryKB;
    double cacheCost;
} CacheCalculations;

// Physical mem calc struct
typedef struct {
    int physicalPages;
    float percent;
    int systemPages;
    int pteBits;
    int totalPageTableRam;
} PhysicalMemoryCalculations;

// Values for page table statistics
typedef struct {
    char *filename;
    int usedPageTableEntries;
    int pageTableWasted;
} ProcessPageTableUsage;


// Virtual memory sim struct
typedef struct {
    int systemPages;
    int pageAvailableUser;
    int virtualPagesMapped;
    int pageTableHits;
    int pagesFromFree;
    int totalPageFaults;
    ProcessPageTableUsage processes[3];
    int processCount;
} VirtualMemoryResults;


// Function prototypes
void displayCacheInput(CacheInput input);
void printCacheCalculations(CacheCalculations calc);
void printMemoryCalculations(PhysicalMemoryCalculations memCalc);
void printVirtualMemSimRes(VirtualMemoryResults vmRes);

CacheCalculations cacheCalculations(CacheInput input);
PhysicalMemoryCalculations physicalMemoryCalculations(CacheInput input);
VirtualMemoryResults virtualMemSimRes(PhysicalMemoryCalculations memCalc, CacheInput input);

int log2n(int n);


// USAGE EXAMPLE:
// VMCacheSim.exe -s 512 -b 16 -a 4 -r rr -p 1024 -n 100 -u 75 -f Trace1.trc -f Trace2_4Evaluation.trc -f Corruption1.trc

int main(int argc, char* argv[]) {

    CacheInput input;

    // Check argument count for minimum amount of arguments
    if (argc < 17) {
        printf("\nInsufficient amount of arguments.\n");
        printf("\nUsage: VMCacheSim.exe -s 512 -b 16 -a 4 -r rr -p 1024 -n 100 -u 75 -f Trace1.trc -f Trace2_4Evaluation.trc -f Corruption1.trc");
        printf("\n-s <cache size - KB>\n-b <block size>\n-a <associativity>\n-r <replacement policy>\n-p <physical memory - MB>\n-n <Instructions / Time Slice>\n-u <%% of physical mem used by OS>\n-f <trace file name> **Up to three**\n\n");
        return 1;
    }

    // Carve argument values from argv and convert from ascii to int
    input.cacheSize = atoi(argv[2]);
    input.blockSize = atoi(argv[4]);
    input.associativity = atoi(argv[6]);
    input.replacementPolicy = argv[8];
    input.physicalMemory = atoi(argv[10]);
    input.instructionsTimeSlice = atoi(argv[12]);
    input.percentUsed = atof(argv[14]);

    // Depending on how many files were provided we store the files
    switch (argc) {
        // One file provided
        case 17:
            input.files[0] = argv[16];
            input.fileCount = 1;
            break;

        // Two files provided
        case 19:
            input.files[0] = argv[16];
            input.files[1] = argv[18];
            input.fileCount = 2;
            break;
        // Three file provided
        case 21:
            input.files[0] = argv[16];
            input.files[1] = argv[18];
            input.files[2] = argv[20];
            input.fileCount = 3;
            break;

        default:
            printf("Error: invalid number of arguments or too many files provided\n");
            exit(1);
    }

    displayCacheInput(input);

    // Fill struct and print
    CacheCalculations cacheCalc = cacheCalculations(input);
    printCacheCalculations(cacheCalc);

    // Fill struct and print
    PhysicalMemoryCalculations memCalc = physicalMemoryCalculations(input);
    printMemoryCalculations(memCalc);

    // Fill struct and print, send off memCalc and input structs to help with sim
    VirtualMemoryResults vmRes = virtualMemSimRes(memCalc, input);
    printVirtualMemSimRes(vmRes);



    return 0;
}

void displayCacheInput(CacheInput input) {

    printf("Cache Simulator - CS 3853 - Team #03\n\n");
    printf("Trace File(s):\n");

    switch (input.fileCount) {
        case 1:
            printf("   %s\n", input.files[0]);
            break;

        case 2:
            printf("   %s\n   %s\n", input.files[0], input.files[1]);
            break;

        case 3:
            printf("   %s\n   %s\n   %s\n", input.files[0], input.files[1], input.files[2]);
            break;
    }

    printf("\n***** Cache Input Parameters *****\n\n");
    printf("Cache Size:                     %d KB\n", input.cacheSize);
    printf("Block Size:                     %d bytes\n", input.blockSize);
    printf("Associativity:                  %d\n", input.associativity);
    printf("Replacement Policy:             Round Robin\n");
    printf("Physical Memory:                %d MB\n", input.physicalMemory);
    printf("Percent Memory Used by System:  %.1f %%\n", input.percentUsed);
    printf("Instructions / Time slice:      %d\n", input.instructionsTimeSlice);
}

CacheCalculations cacheCalculations(CacheInput input) {
    CacheCalculations calc;

    // Declare unsigned integer cacheSizeBytes
    calc.cacheSizeBytes = input.cacheSize * 1024;

    // Calculate the total amount of blocks and rows based off block size and associativity
    calc.totalBlocks = calc.cacheSizeBytes / input.blockSize;
    calc.totalRows = calc.totalBlocks / input.associativity;
    
    // Calculate the number of offset and index bits
    calc.offsetBits = log2n(input.blockSize);
    calc.indexBits = log2n(calc.totalRows);
    
    // Calculate the number of physical address bits
    calc.physicalAddressBits = log2n(input.physicalMemory) + 20;
    
    // Calculate how many bits for the tag
    calc.tagBits = calc.physicalAddressBits - calc.indexBits - calc.offsetBits;
    
    // Calculate overhead bits
    calc.overheadBitsPerBlock = calc.tagBits + 1;
    
    // Calculate the overhead size in total bytes
    calc.overheadSizeBytes = (calc.totalBlocks * calc.overheadBitsPerBlock) / 8;
    
    // Calculate the cost and memory bytes 
    calc.implementationMemoryBytes = calc.cacheSizeBytes + calc.overheadSizeBytes;
    calc.implementationMemoryKB = (double)calc.implementationMemoryBytes / 1024.0;
    
    // Find the cost value
    calc.cacheCost = calc.implementationMemoryKB * 0.07;

    return calc;
}

PhysicalMemoryCalculations physicalMemoryCalculations(CacheInput input) {
    // Declare structure
    PhysicalMemoryCalculations memCalc;

    // Calculate the physical memory pages
    memCalc.physicalPages = input.physicalMemory * 256;

    // Find the percent used
    memCalc.percent = input.percentUsed * 0.01f;
    memCalc.systemPages = memCalc.percent * memCalc.physicalPages;

    // Get page table entry bits
    memCalc.pteBits = log2n(memCalc.physicalPages) + 1;
    
    // calculate the page table RAM
    memCalc.totalPageTableRam = (524288 * input.fileCount * memCalc.pteBits) / 8;

    return memCalc;
}

VirtualMemoryResults virtualMemSimRes(PhysicalMemoryCalculations memCalc, CacheInput input) {
    // Virtual memory struct
    VirtualMemoryResults vmRes;

    int i;
    // Virt page #
    int totalVirtualPages = 524288;
    
    // Calc PT size
    int totalPageTableBytes = (totalVirtualPages * memCalc.pteBits) / 8;

    // Initialize values for sim
    vmRes.systemPages = memCalc.systemPages;
    vmRes.pageAvailableUser = memCalc.physicalPages - memCalc.systemPages;
    vmRes.virtualPagesMapped = 0;
    vmRes.pageTableHits = 0;
    vmRes.pagesFromFree = 0;
    vmRes.totalPageFaults = 0;
    vmRes.processCount = input.fileCount;


    // Read in the trc files
    for(i = 0; i < input.fileCount; i++){
        // FIle pointer for trc file we are looking at
        FILE *fp;
        
        // Instruction
        char instructionLine[256];
        // Data
        char dataLine[256];

        int *pageTable;
        int freePages;
        int usedEntries;
        unsigned int instructionAddress;
        unsigned int dst;
        unsigned int src;
        int instructionLength;

        // Open trc file
        vmRes.processes[i].filename = input.files[i];
        vmRes.processes[i].usedPageTableEntries = 0;
        vmRes.processes[i].pageTableWasted = 0;

        // Open input file for read with fopen, exit on failure
        fp = fopen(input.files[i], "r");
        if (fp == NULL) {
            printf("Invalid file path: %s exiting...\n", input.files[i]);
            exit(1);
        }

        // Allocat memory for virtual pages with malloc, exit on failure
        pageTable = malloc(sizeof(int) * totalVirtualPages);
        // Make sure the memory was allocated
        if (pageTable == NULL) {
            printf("Something went wrong allocating memory to: %s\n", input.files[i]);
            // Close out the file pointer
            fclose(fp);
            exit(1);
        }

        // Initialize the table entries to zero
        for (instructionLength = 0; instructionLength < totalVirtualPages; instructionLength++) {
            pageTable[instructionLength] = -1;
        }

        // Free pages is amount that is available
        freePages = vmRes.pageAvailableUser;
        // Init used as 0 for now
        usedEntries = 0;

        // Loop through each line until EOF
        while (fgets(instructionLine, sizeof(instructionLine), fp) != NULL) {
            char lengthStr[3];
            char addressStr[9];
            char dstStr[9];
            char srcStr[9];
            unsigned int startPage;
            unsigned int endPage;
            unsigned int page;

            // If EIP, skip
            if (strncmp(instructionLine, "EIP", 3) != 0) {
                continue;
            }

            // If no data exit
            if (fgets(dataLine, sizeof(dataLine), fp) == NULL) {
                break;
            }

            // Find instruction length and get address by offsets
            lengthStr[0] = instructionLine[5];
            lengthStr[1] = instructionLine[6];
            lengthStr[2] = '\0';

            addressStr[0] = instructionLine[10];
            addressStr[1] = instructionLine[11];
            addressStr[2] = instructionLine[12];
            addressStr[3] = instructionLine[13];
            addressStr[4] = instructionLine[14];
            addressStr[5] = instructionLine[15];
            addressStr[6] = instructionLine[16];
            addressStr[7] = instructionLine[17];
            addressStr[8] = '\0';

            // Get strings in hex to number form
            sscanf(lengthStr, "%x", &instructionLength);
            sscanf(addressStr, "%x", &instructionAddress);

            // Find where instruction is
            startPage = instructionAddress / 4096;
            endPage = (instructionAddress + instructionLength - 1) / 4096;

            // Go through each page in access
            for (page = startPage; page <= endPage; page++) {
                // Page touch and increment
                vmRes.virtualPagesMapped++;
                
                // See if page mapped in PT
                if (pageTable[page] == 1) {
                    // Increment if mapped
                    vmRes.pageTableHits++;
                }
                else if (freePages > 0) {
                    // Page available so map it and decrement free
                    pageTable[page] = 1;
                    freePages--;
                    usedEntries++;
                    vmRes.pagesFromFree++;
                }
                // No pages left case
                else {
                    vmRes.totalPageFaults++;
                }
            }

            // Get dest and src addresses and convert them to numerical form
            dstStr[0] = dataLine[6];
            dstStr[1] = dataLine[7];
            dstStr[2] = dataLine[8];
            dstStr[3] = dataLine[9];
            dstStr[4] = dataLine[10];
            dstStr[5] = dataLine[11];
            dstStr[6] = dataLine[12];
            dstStr[7] = dataLine[13];
            dstStr[8] = '\0';

            srcStr[0] = dataLine[34];
            srcStr[1] = dataLine[35];
            srcStr[2] = dataLine[36];
            srcStr[3] = dataLine[37];
            srcStr[4] = dataLine[38];
            srcStr[5] = dataLine[39];
            srcStr[6] = dataLine[40];
            srcStr[7] = dataLine[41];
            srcStr[8] = '\0';

            sscanf(dstStr, "%x", &dst);
            sscanf(srcStr, "%x", &src);

            // Check for dst validity
            if (dst != 0) {
                // Get page number and page end
                startPage = dst / 4096;
                endPage = (dst + 4 - 1) / 4096;

                // Check for cross boundary write
                for (page = startPage; page <= endPage; page++) {
                    vmRes.virtualPagesMapped++; // increment mapped page

                    // Check if page mapped
                    if (pageTable[page] == 1) {
                        vmRes.pageTableHits++;
                    } // Make sure theres still space to map
                    else if (freePages > 0) {
                        pageTable[page] = 1;
                        freePages--;
                        usedEntries++;
                        vmRes.pagesFromFree++;
                    }
                    // Not mapped and no pages
                    else {
                        vmRes.totalPageFaults++;
                    }
                }
            }

            // Process src if valid
            if (src != 0) {
                // Find the starting page for the source read and end page for src read
                startPage = src / 4096;
                endPage = (src + 4 - 1) / 4096;

                // Go through every page touched by the source read
                for (page = startPage; page <= endPage; page++) {
                    vmRes.virtualPagesMapped++;

                    // Count the virtual page access
                    if (pageTable[page] == 1) {
                        vmRes.pageTableHits++;
                    }
                    // Check if the page is already mapped
                    else if (freePages > 0) {
                        pageTable[page] = 1;
                        freePages--;
                        usedEntries++;
                        vmRes.pagesFromFree++;
                    }
                    // Fault
                    else {
                        vmRes.totalPageFaults++;
                    }
                }
            }
        }

        // Store the amount of used page table entries for this process
        vmRes.processes[i].usedPageTableEntries = usedEntries;
        // Calculate page waste
        vmRes.processes[i].pageTableWasted = totalPageTableBytes - ((usedEntries * memCalc.pteBits) / 8);

        free(pageTable);
        fclose(fp);

    }

    

    // Return the struct to main for printing and next function
    return vmRes;
}



// Calculation functions above 

// Print functions below



void printMemoryCalculations(PhysicalMemoryCalculations memCalc) {
    printf("\n***** Physical Memory Calculated Values *****\n\n");
    printf("%-31s %d\n", "Number of Physical Pages:", memCalc.physicalPages);
    printf("%-31s %d\n", "Number of Pages for System:", memCalc.systemPages);
    printf("%-31s %d bits\n", "Size of Page Table Entry:", memCalc.pteBits);
    printf("%-31s %d bytes\n", "Total RAM for Page Table(s):", memCalc.totalPageTableRam);
}

void printCacheCalculations(CacheCalculations calc) {
    printf("\n\n***** Cache Calculated Values *****\n\n");
    printf("%-31s %d\n", "Total # Blocks:", calc.totalBlocks);
    printf("%-31s %d bits\n", "Tag Size:", calc.tagBits);
    printf("%-31s %d bits\n", "Index Size:", calc.indexBits);
    printf("%-31s %d\n", "Total # Rows:", calc.totalRows);
    printf("%-31s %u bytes\n", "Overhead Size:", calc.overheadSizeBytes);
    printf("%-31s %.2lf KB (%u bytes)\n", "Implementation Memory Size:", calc.implementationMemoryKB, calc.implementationMemoryBytes);
    printf("%-31s $%.2lf @ $0.07 per KB\n", "Cost:", calc.cacheCost);
}


void printVirtualMemSimRes(VirtualMemoryResults vmRes) {
    int i;
    double percentUsed;

    printf("\n***** VIRTUAL MEMORY SIMULATION RESULTS *****\n\n");
    printf("Physical Pages Used By SYSTEM: %d\n", vmRes.systemPages);
    printf("Pages Available to User:\t%d\n\n", vmRes.pageAvailableUser);
    printf("Virtual Pages Mapped:\t\t%d\n", vmRes.virtualPagesMapped);
    printf("\t------------------------------\n");
    printf("\tPage Table Hits:\t%d\n\n", vmRes.pageTableHits);
    printf("\tPages from Free:\t%d\n\n", vmRes.pagesFromFree);
    printf("\tTotal Page Faults:\t%d\n\n\n", vmRes.totalPageFaults);
    printf("Page Table Usage Per Process:\n");
    printf("------------------------------\n");

    // Loop through trace files and print the results we found
    for (i = 0; i < vmRes.processCount; i++) {
        // Get percent used
        percentUsed = ((double)vmRes.processes[i].usedPageTableEntries / 524288.0) * 100.0;
        printf("[%d] %s:\n", i, vmRes.processes[i].filename);
        printf("\tUsed Page Table Entries: %d   ( %.2f%% )\n",
               vmRes.processes[i].usedPageTableEntries, percentUsed);
        printf("\tPage Table Wasted: %d bytes\n\n", vmRes.processes[i].pageTableWasted);
    }

    printf("\n");
}


// Log function for some of the calculations
int log2n(int n) {
    int x = 0;
    while (n > 1) {
        n /= 2;
        x++;
    }
    return x;
}
