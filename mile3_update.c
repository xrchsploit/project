#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Group #3: Antonio Charfauros, Paul Castillo, Cain Green, Abrianna Garcia

//added defenitions for M3 to simplify
#define PAGE_SIZE 4096
#define TOTAL_VIRTUAL_PAGES 524288
#define MAX_FILES 3

//structs
//cache hit miss struct M3
typedef struct {
    double hitRate;
    double missRate;
    double cpi;
    unsigned int unusedBlocks;
    double unusedKB;
    double wastedCost;
} CacheResults;

// Argument struct
typedef struct {
    int cacheSize;
    int blockSize;
    int associativity;
    char *replacementPolicy;
    int physicalMemory;
    float percentUsed;
    int instructionsTimeSlice;
    int fileCount;
    char *files[MAX_FILES];
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

// Cache simulation structs
//M3 addition
typedef struct {
    int valid;
    unsigned int tag;
    unsigned int blockAddress;
} CacheLine;

typedef struct {
    CacheLine *lines;
    int nextReplace;
} CacheSet;

typedef struct {
    unsigned int totalCacheAccesses;
    unsigned int instructionBytes;
    unsigned int srcDstBytes;
    unsigned int cacheHits;
    unsigned int cacheMisses;
    unsigned int compulsoryMisses;
    unsigned int conflictMisses;
    unsigned long long totalCycles;
    unsigned int totalInstructions;
    unsigned int blocksEverUsed;
} CacheStats;

// Virtual memory sim struct
typedef struct {
    int systemPages;
    int pageAvailableUser;
    int virtualPagesMapped;
    int pageTableHits;
    int pagesFromFree;
    int totalPageFaults;
    ProcessPageTableUsage processes[MAX_FILES];
    int processCount;
    CacheStats cacheStats;
} VirtualMemoryResults;

// Function prototypes
void displayCacheInput(CacheInput input);
void printCacheCalculations(CacheCalculations calc);
void printMemoryCalculations(PhysicalMemoryCalculations memCalc);
void printVirtualMemSimRes(VirtualMemoryResults vmRes);

CacheCalculations cacheCalculations(CacheInput input);
PhysicalMemoryCalculations physicalMemoryCalculations(CacheInput input);
VirtualMemoryResults virtualMemSimRes(PhysicalMemoryCalculations memCalc, CacheInput input);

//Functions for cache sim
CacheResults calculateCacheResults(CacheStats stats, int totalBlocks, int blockSize, int overheadSizeBytes, double implementationMemoryKB, double cacheCost);
void initializeCache(CacheSet cache[], int totalRows, int associativity);
void freeCache(CacheSet cache[], int totalRows);
void processCacheAccess(CacheSet cache[], int totalRows, int associativity, int blockSize, char *replacementPolicy,
                        unsigned int physicalAddress, int accessSize, int isInstruction, int *seenBlock, int totalPhysicalBlocks, CacheStats *stats);

void simulateMemoryAccess(unsigned int virtualAddress, int accessSize, int isInstruction, int *pageTable, int *nextFreePhysicalPage,
                          PhysicalMemoryCalculations memCalc, CacheSet cache[], CacheCalculations cacheCalc, CacheInput input, int *seenBlock,
                          int totalPhysicalBlocks, int *usedEntries, VirtualMemoryResults *vmRes);
void printCacheSimulationResults(CacheStats stats, CacheResults result, CacheCalculations cacheCalc);

int log2n(int n);
//added M3 to help with cache calculation accuracy 
int powerOfTwo(int n);

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
            return 1;
    }

    // Cache sizes, block sizes, etc. need to be powers of 2. Added for M3
    if (!powerOfTwo(input.cacheSize) || !powerOfTwo(input.blockSize) ||
        !powerOfTwo(input.associativity) || !powerOfTwo(input.physicalMemory)) {
        printf("Error: cache size, block size, associativity, and physical memory should be powers of two.\n");
        return 1;
    }

    //Runs all results
    displayCacheInput(input);

    CacheCalculations cacheCalc = cacheCalculations(input);
    printCacheCalculations(cacheCalc);

    PhysicalMemoryCalculations memCalc = physicalMemoryCalculations(input);
    printMemoryCalculations(memCalc);

    VirtualMemoryResults vmRes = virtualMemSimRes(memCalc, input);
    printVirtualMemSimRes(vmRes);

    //added for M3
    CacheResults result = calculateCacheResults(
    vmRes.cacheStats,
    cacheCalc.totalBlocks,
    input.blockSize,
    cacheCalc.overheadSizeBytes,
    cacheCalc.implementationMemoryKB,
    cacheCalc.cacheCost
);

printCacheSimulationResults(vmRes.cacheStats, result, cacheCalc);

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
    
    //replacement policy
    if (strcmp(input.replacementPolicy, "rnd") == 0 || strcmp(input.replacementPolicy, "RND") == 0) {
    printf("Replacement Policy:             Random\n");
} else {
    printf("Replacement Policy:             Round Robin\n");
}

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

    // Calculate the overhead size in total bytes (changed to round up using +7 for M3)
    calc.overheadSizeBytes = (calc.totalBlocks * calc.overheadBitsPerBlock + 7) / 8;

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
    memCalc.systemPages = (int)(memCalc.percent * memCalc.physicalPages);

    // Get page table entry bits
    memCalc.pteBits = log2n(memCalc.physicalPages) + 1;

    // calculate the page table RAM (added the overhead +7 for M3)
    memCalc.totalPageTableRam = (TOTAL_VIRTUAL_PAGES * input.fileCount * memCalc.pteBits + 7) / 8;

    return memCalc;
}

//added for M3
CacheResults calculateCacheResults(CacheStats stats, int totalBlocks, int blockSize, int overheadSizeBytes, double implementationMemoryKB, double cacheCost) {

    CacheResults result;

    result.hitRate = 0.0;
    result.missRate = 0.0;
    result.cpi = 0.0;

    if (stats.totalCacheAccesses > 0) {
        result.hitRate = ((double)stats.cacheHits * 100.0) / stats.totalCacheAccesses;
        result.missRate = ((double)stats.cacheMisses * 100.0) / stats.totalCacheAccesses;
    }

    if (stats.totalInstructions > 0) {
        result.cpi = (double)stats.totalCycles / stats.totalInstructions;
    }

    double overheadPerBlock = (double)overheadSizeBytes / totalBlocks;

    result.unusedBlocks = totalBlocks - stats.blocksEverUsed;
    result.unusedKB = (result.unusedBlocks * (blockSize + overheadPerBlock)) / 1024.0;

    if (implementationMemoryKB > 0.0) {
        result.wastedCost = (result.unusedKB / implementationMemoryKB) * cacheCost;
    } else {
        result.wastedCost = 0.0;
    }

    return result;
}

// updated virtualMemSimResfor M3, accounts for cache simulation info.
VirtualMemoryResults virtualMemSimRes(PhysicalMemoryCalculations memCalc, CacheInput input) {
    // Virtual memory struct
    VirtualMemoryResults vmRes;

    CacheCalculations cacheCalc = cacheCalculations(input);

    CacheSet *cache = malloc(sizeof(CacheSet) * cacheCalc.totalRows);
    int totalPhysicalBlocks = (input.physicalMemory * 1024 * 1024) / input.blockSize;
    int *seenBlock = calloc(totalPhysicalBlocks, sizeof(int));

    int totalPageTableBytes = (TOTAL_VIRTUAL_PAGES * memCalc.pteBits + 7) / 8;
    int i;
    int nextFreePhysicalPage = memCalc.systemPages;

    if (cache == NULL || seenBlock == NULL) {
        printf("Error allocating cache memory.\n");
        exit(1);
    }


    // Initialize values for sim
    initializeCache(cache, cacheCalc.totalRows, input.associativity);

    vmRes.systemPages = memCalc.systemPages;
    vmRes.pageAvailableUser = memCalc.physicalPages - memCalc.systemPages;
    vmRes.virtualPagesMapped = 0;
    vmRes.pageTableHits = 0;
    vmRes.pagesFromFree = 0;
    vmRes.totalPageFaults = 0;
    vmRes.processCount = input.fileCount;
    memset(&vmRes.cacheStats, 0, sizeof(CacheStats));

    // Read in the trc files
    for (i = 0; i < input.fileCount; i++) {
        // FIle pointer for trc file we are looking at
        FILE *fp;

        // Instruction
        char instructionLine[256];
        // Data
        char dataLine[256];

        int *pageTable;
        int usedEntries;

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
        pageTable = malloc(sizeof(int) * TOTAL_VIRTUAL_PAGES);
        // Make sure the memory was allocated
        if (pageTable == NULL) {
            printf("Something went wrong allocating memory to: %s\n", input.files[i]);
            // Close out the file pointer
            fclose(fp);
            exit(1);
        }

        //Initialize table entries to 0
        for (int j = 0; j < TOTAL_VIRTUAL_PAGES; j++) {
            pageTable[j] = -1;
        }

        usedEntries = 0;
        
        // changed for M3, allows us to use sscanf to parse, avoid manual indexing, and ignore space savers
        while (fgets(instructionLine, sizeof(instructionLine), fp) != NULL) {

            int instructionLength;
            unsigned int instructionAddress;
            unsigned int dst;
            unsigned int src;
            char dstData[16];
            char srcData[16];

            // If EIP, skip
            if (strstr(instructionLine, "EIP") == NULL) {
                continue;
            }

            // If no data exit
            if (fgets(dataLine, sizeof(dataLine), fp) == NULL) {
                break;
            }

            //for M3, simulate instruction fetch 
            if (sscanf(instructionLine, "EIP (%d): %x", &instructionLength, &instructionAddress) == 2) {
                simulateMemoryAccess(instructionAddress, instructionLength, 1, pageTable, &nextFreePhysicalPage, memCalc,
                                     cache, cacheCalc, input, seenBlock, totalPhysicalBlocks, &usedEntries, &vmRes);
            }

            //ignore space savers, M3 
            dst = 0;
            src = 0;
            strcpy(dstData, "--------");
            strcpy(srcData, "--------");

            
            if (sscanf(dataLine, "dstM: %x %15s srcM: %x %15s", &dst, dstData, &src, srcData) == 4) {
                //destination write if valid, M3
                if (dst != 0 && strcmp(dstData, "--------") != 0) {
                    simulateMemoryAccess(dst, 4, 0, pageTable, &nextFreePhysicalPage, memCalc, cache, cacheCalc, input, 
                                         seenBlock, totalPhysicalBlocks, &usedEntries, &vmRes);
                }

                //source read if valid, M3
                if (src != 0 && strcmp(srcData, "--------") != 0) {
                    simulateMemoryAccess(src, 4, 0, pageTable, &nextFreePhysicalPage, memCalc, cache, cacheCalc, input, seenBlock,
                                         totalPhysicalBlocks, &usedEntries, &vmRes);
                }
            }
        }

        //store usage and calc wasted space, M3
        vmRes.processes[i].usedPageTableEntries = usedEntries;
        vmRes.processes[i].pageTableWasted =
            totalPageTableBytes - ((usedEntries * memCalc.pteBits + 7) / 8);

        //free mem. for process, M3
        free(pageTable);
        fclose(fp);
    }

    //free mem. when done, M3
    freeCache(cache, cacheCalc.totalRows);
    free(cache);
    free(seenBlock);

    // Return the struct to main for printing and next function
    return vmRes;
}

//added for M3 for cache access, page tables, and the cycle
void simulateMemoryAccess(unsigned int virtualAddress, int accessSize, int isInstruction, int *pageTable, int *nextFreePhysicalPage,
                          PhysicalMemoryCalculations memCalc, CacheSet cache[], CacheCalculations cacheCalc, CacheInput input, int *seenBlock,
                          int totalPhysicalBlocks, int *usedEntries, VirtualMemoryResults *vmRes) {
    //
    unsigned int startPage = virtualAddress / PAGE_SIZE;
    unsigned int endPage = (virtualAddress + accessSize - 1) / PAGE_SIZE;
    unsigned int page;

    // add cycles and track bytes
    if (isInstruction) {
        vmRes->cacheStats.instructionBytes += accessSize;
        vmRes->cacheStats.totalInstructions++;
        vmRes->cacheStats.totalCycles += 2;
    } else {
        vmRes->cacheStats.srcDstBytes += accessSize;
        vmRes->cacheStats.totalCycles += 1;
    }

    //process page
    for (page = startPage; page <= endPage; page++) {
        unsigned int pageOffsetStart;
        unsigned int bytesThisPage;
        unsigned int physicalAddress;
        int physicalPage;

        //skip if invalid
        if (page >= TOTAL_VIRTUAL_PAGES) {
            continue;
        }

        //count
        vmRes->virtualPagesMapped++;

        //check if page has been mapped
        if (pageTable[page] != -1) {
            vmRes->pageTableHits++;
        } else {
            //map page
            if (*nextFreePhysicalPage < memCalc.physicalPages) {
                pageTable[page] = *nextFreePhysicalPage;
                (*nextFreePhysicalPage)++;
                (*usedEntries)++;
                vmRes->pagesFromFree++;
            } else {
                // no free pages: then count page fault & add penalty
                vmRes->totalPageFaults++;
                vmRes->cacheStats.totalCycles += 100;

                if (vmRes->pageAvailableUser > 0) {
                    pageTable[page] = memCalc.systemPages +
                        (int)(page % (unsigned int)vmRes->pageAvailableUser);
                    (*usedEntries)++;
                } else {
                    pageTable[page] = 0;
                }
            }
        }

        physicalPage = pageTable[page];

        pageOffsetStart = 0;
        if (page == startPage) {
            pageOffsetStart = virtualAddress % PAGE_SIZE;
        }

        //find offset
        if (startPage == endPage) {
            bytesThisPage = accessSize;
        } else if (page == startPage) {
            bytesThisPage = PAGE_SIZE - pageOffsetStart;
        } else if (page == endPage) {
            bytesThisPage = (virtualAddress + accessSize - 1) % PAGE_SIZE + 1;
        } else {
            bytesThisPage = PAGE_SIZE;
        }

        physicalAddress = (unsigned int)(physicalPage * PAGE_SIZE + pageOffsetStart);

        //send access to cache sim
        processCacheAccess(cache, cacheCalc.totalRows, input.associativity, input.blockSize, input.replacementPolicy,
                           physicalAddress, (int)bytesThisPage, 0, seenBlock, totalPhysicalBlocks, &vmRes->cacheStats);
    }
}

//initialize cache M3
void initializeCache(CacheSet cache[], int totalRows, int associativity) {
    int i, j;

    for (i = 0; i < totalRows; i++) {
        //allocate memory
        cache[i].lines = malloc(sizeof(CacheLine) * associativity);
        //init. replacement for round robin
        cache[i].nextReplace = 0;

        if (cache[i].lines == NULL) {
            printf("Error allocating cache set.\n");
            exit(1);
        }

        //empty cache
        for (j = 0; j < associativity; j++) {
            cache[i].lines[j].valid = 0;
            cache[i].lines[j].tag = 0;
            cache[i].lines[j].blockAddress = 0;
        }
    }
}

//free memory for cache M3
void freeCache(CacheSet cache[], int totalRows) {
    int i;

    for (i = 0; i < totalRows; i++) {
        free(cache[i].lines);
    }
}

//simulates cache access
void processCacheAccess(CacheSet cache[], int totalRows, int associativity, int blockSize, char *replacementPolicy,
                        unsigned int physicalAddress, int accessSize, int isInstruction, int *seenBlock,
                        int totalPhysicalBlocks, CacheStats *stats) {
    unsigned int startBlock;
    unsigned int endBlock;
    unsigned int block;

    (void)isInstruction;

    //avoid inv. access sizes
    if (accessSize <= 0) {
        return;
    }

    // which cache blocks touches the access
    startBlock = physicalAddress / blockSize;
    endBlock = (physicalAddress + accessSize - 1) / blockSize;

    //process necessary blocks
    for (block = startBlock; block <= endBlock; block++) {
        unsigned int index;
        unsigned int tag;
        int hit = 0;
        int emptyLine = -1;
        int replaceIndex = 0;
        int j;

        //skip if outside mem. 
        if (block >= (unsigned int)totalPhysicalBlocks) {
            continue;
        }

        //calculate index and tag
        index = block % totalRows;
        tag = block / totalRows;

        stats->totalCacheAccesses++;

        //check lines for hit or empty space
        for (j = 0; j < associativity; j++) {
            if (cache[index].lines[j].valid &&
                cache[index].lines[j].tag == tag) {
                hit = 1;
                break;
            }

            if (!cache[index].lines[j].valid && emptyLine == -1) {
                emptyLine = j;
            }
        }

        if (hit) {
            //increment cache hit
            stats->cacheHits++;
            stats->totalCycles += 1;
        } else {
            //increment cache miss
            stats->cacheMisses++;
            stats->totalCycles += 4 * ((blockSize + 3) / 4);

            //check if cumpolsory or conflict
            if (!seenBlock[block]) {
                stats->compulsoryMisses++;
                seenBlock[block] = 1;
                stats->blocksEverUsed++;
            } else {
                stats->conflictMisses++;
            }

            //update cache with new block
            if (emptyLine != -1) {
                replaceIndex = emptyLine;
            } else if (strcmp(replacementPolicy, "rnd") == 0 ||
                       strcmp(replacementPolicy, "RND") == 0) {
                replaceIndex = rand() % associativity;
            } else {
                replaceIndex = cache[index].nextReplace;
                cache[index].nextReplace =
                    (cache[index].nextReplace + 1) % associativity;
            }

            cache[index].lines[replaceIndex].valid = 1;
            cache[index].lines[replaceIndex].tag = tag;
            cache[index].lines[replaceIndex].blockAddress = block;
        }
    }
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
        percentUsed = ((double)vmRes.processes[i].usedPageTableEntries / TOTAL_VIRTUAL_PAGES) * 100.0;
        printf("[%d] %s:\n", i, vmRes.processes[i].filename);
        printf("\tUsed Page Table Entries: %d   ( %.2f%% )\n",
               vmRes.processes[i].usedPageTableEntries, percentUsed);
        printf("\tPage Table Wasted: %d bytes\n\n", vmRes.processes[i].pageTableWasted);
    }

    printf("\n");
}

void printCacheSimulationResults(CacheStats stats, CacheResults result, CacheCalculations cacheCalc) {

    printf("\n***** CACHE SIMULATION RESULTS *****\n\n");

    printf("%-28s %u\n", "Total Cache Accesses:", stats.totalCacheAccesses);
    printf("%-28s %u\n", "--- Instruction Bytes:", stats.instructionBytes);
    printf("%-28s %u\n", "--- SrcDst Bytes:", stats.srcDstBytes);

    printf("%-28s %u\n", "Cache Hits:", stats.cacheHits);
    printf("%-28s %u\n", "Cache Misses:", stats.cacheMisses);
    printf("%-28s %u\n", "--- Compulsory Misses:", stats.compulsoryMisses);
    printf("%-28s %u\n", "--- Conflict Misses:", stats.conflictMisses);

    printf("\n***** *****  CACHE HIT & MISS RATE:  ***** *****\n\n");

    printf("%-28s %.4f%%\n", "Hit Rate:", result.hitRate);
    printf("%-28s %.4f%%\n", "Miss Rate:", result.missRate);

    printf("%-28s %.2f Cycles/Instruction (%llu)\n", "CPI:", result.cpi, stats.totalCycles);

    double percentUnused = (result.unusedKB / cacheCalc.implementationMemoryKB) * 100.0;

    printf("%-28s %.2f KB / %.2f KB = %.2f%%  Waste: $%.2f/chip\n", "Unused Cache Space:", result.unusedKB,
           cacheCalc.implementationMemoryKB, percentUnused, result.wastedCost);

    printf("%-28s %u / %d\n", "Unused Cache Blocks:", result.unusedBlocks, cacheCalc.totalBlocks);
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

int powerOfTwo(int n) {
    if (n <= 0) {
        return 0;
    }

    return (n & (n - 1)) == 0;
}
