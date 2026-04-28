// pseudo code structs

typedef struct {
    int valid;
    unsigned int tag;
    unsigned int blockAddress;
} CacheLine;

typedef struct {
    CacheLine *lines;
    int nextReplace;   // for round-robin
} CacheSet;

typedef struct {
    unsigned int totalCacheAccesses;
    unsigned int instructionBytes;
    unsigned int srcDstBytes;

    unsigned int cacheHits;
    unsigned int cacheMisses;
    unsigned int compulsoryMisses;
    unsigned int conflictMisses;

    unsigned int totalCycles;
    unsigned int totalInstructions;
    unsigned int blocksEverUsed;
} CacheStats;

// prototype block

void initializeCache(CacheSet cache[], int totalRows, int associativity);
unsigned int getBlockAddress(unsigned int physicalAddress, int blockSize);
unsigned int getCacheIndex(unsigned int blockAddress, int totalRows);
unsigned int getCacheTag(unsigned int blockAddress, int totalRows);

void processCacheAccess(CacheSet cache[], int totalRows, int associativity,
                        int blockSize, char *replacementPolicy,
                        unsigned int physicalAddress, int accessSize,
                        int isInstruction, CacheStats *stats);

void printCacheSimulationResults(CacheStats stats, int totalBlocks, int blockSize, double implementationMemoryKB, double cacheCost);


// inside main

CacheSet *cache;
CacheStats cacheStats = {0};

cache = malloc(sizeof(CacheSet) * totalRows);

if (cache == NULL) {
    printf("Error: could not allocate cache.\n");
    return 1;
}

initializeCache(cache, totalRows, associativity);


// after VM cache sim runs

printCacheSimulationResults(cacheStats, totalBlocks, blockSize, implementationMemoryKB, cacheCost);


// with funtions 

void initializeCache(CacheSet cache[], int totalRows, int associativity)
{
    int i, j;

    for (i = 0; i < totalRows; i++) {
        cache[i].lines = malloc(sizeof(CacheLine) * associativity);
        cache[i].nextReplace = 0;

        for (j = 0; j < associativity; j++) {
            cache[i].lines[j].valid = 0;
            cache[i].lines[j].tag = 0;
            cache[i].lines[j].blockAddress = 0;
        }
    }
}


// address decode block 

unsigned int getBlockAddress(unsigned int physicalAddress, int blockSize)
{
    return physicalAddress / blockSize;
}

unsigned int getCacheIndex(unsigned int blockAddress, int totalRows)
{
    return blockAddress % totalRows;
}

unsigned int getCacheTag(unsigned int blockAddress, int totalRows)
{
    return blockAddress / totalRows;
}


// main cache access block

void processCacheAccess(CacheSet cache[], int totalRows, int associativity,
                        int blockSize, char *replacementPolicy,
                        unsigned int physicalAddress, int accessSize,
                        int isInstruction, CacheStats *stats)
{
    unsigned int startBlock = physicalAddress / blockSize;
    unsigned int endBlock = (physicalAddress + accessSize - 1) / blockSize;

    if (isInstruction) {
        stats->instructionBytes += accessSize;
        stats->totalInstructions++;
        stats->totalCycles += 2;
    } else {
        stats->srcDstBytes += accessSize;
        stats->totalCycles += 1;
    }

    for (unsigned int block = startBlock; block <= endBlock; block++) {
        unsigned int index = getCacheIndex(block, totalRows);
        unsigned int tag = getCacheTag(block, totalRows);
        int hit = 0;
        int emptyLine = -1;

        stats->totalCacheAccesses++;

        for (int i = 0; i < associativity; i++) {
            if (cache[index].lines[i].valid &&
                cache[index].lines[i].tag == tag) {
                hit = 1;
                break;
            }

            if (!cache[index].lines[i].valid && emptyLine == -1) {
                emptyLine = i;
            }
        }

        if (hit) {
            stats->cacheHits++;
            stats->totalCycles += 1;
        } else {
            int replaceIndex;

            stats->cacheMisses++;
            stats->totalCycles += 4 * ((blockSize + 3) / 4);

            if (emptyLine != -1) {
                stats->compulsoryMisses++;
                replaceIndex = emptyLine;
                stats->blocksEverUsed++;
            } else {
                stats->conflictMisses++;

                if (strcmp(replacementPolicy, "rnd") == 0 ||
                    strcmp(replacementPolicy, "RND") == 0) {
                    replaceIndex = rand() % associativity;
                } else {
                    replaceIndex = cache[index].nextReplace;
                    cache[index].nextReplace =
                        (cache[index].nextReplace + 1) % associativity;
                }
            }

            cache[index].lines[replaceIndex].valid = 1;
            cache[index].lines[replaceIndex].tag = tag;
            cache[index].lines[replaceIndex].blockAddress = block;
        }
    }
}


// code currently proceses an address after page table lookup

processCacheAccess(cache, totalRows, associativity, blockSize, replacementPolicy, physicalAddress, accessSize, isInstruction, &cacheStats);

isInstruction = 1;  // for EIP
isInstruction = 0;  // for dstM/srcM

accessSize = instructionLength;  // EIP
accessSize = 4;                  // dstM/srcM

// print blocks

void printCacheSimulationResults(CacheStats stats, int totalBlocks,
                                 int blockSize, double implementationMemoryKB,
                                 double cacheCost)
{
    double hitRate = 0.0;
    double missRate = 0.0;
    double cpi = 0.0;
    unsigned int unusedBlocks;
    double unusedKB;
    double wastedCost;

    if (stats.totalCacheAccesses > 0) {
        hitRate = ((double)stats.cacheHits * 100.0) / stats.totalCacheAccesses;
        missRate = ((double)stats.cacheMisses * 100.0) / stats.totalCacheAccesses;
    }

    if (stats.totalInstructions > 0) {
        cpi = (double)stats.totalCycles / stats.totalInstructions;
    }

    unusedBlocks = totalBlocks - stats.blocksEverUsed;
    unusedKB = (unusedBlocks * blockSize) / 1024.0;
    wastedCost = (unusedKB / implementationMemoryKB) * cacheCost;

    printf("\n***** CACHE SIMULATION RESULTS *****\n\n");
    printf("Total Cache Accesses:   %u\n", stats.totalCacheAccesses);
    printf("--- Instruction Bytes:  %u\n", stats.instructionBytes);
    printf("--- SrcDst Bytes:       %u\n", stats.srcDstBytes);
    printf("Cache Hits:             %u\n", stats.cacheHits);
    printf("Cache Misses:           %u\n", stats.cacheMisses);
    printf("--- Compulsory Misses:  %u\n", stats.compulsoryMisses);
    printf("--- Conflict Misses:    %u\n", stats.conflictMisses);

    printf("\n***** *****  CACHE HIT & MISS RATE:  ***** *****\n\n");
    printf("Hit  Rate:              %.4f%%\n", hitRate);
    printf("Miss Rate:              %.4f%%\n", missRate);
    printf("CPI:                    %.2f Cycles/Instruction  (%u)\n",
           cpi, stats.totalCycles);
    printf("Unused Cache Space:     %.2f KB / %.2f KB = %.2f%%  Waste: $%.2f/chip\n",
           unusedKB, implementationMemoryKB,
           (unusedKB / implementationMemoryKB) * 100.0,
           wastedCost);
    printf("Unused Cache Blocks:    %u / %u\n", unusedBlocks, totalBlocks);
}