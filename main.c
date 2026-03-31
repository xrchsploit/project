#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Group #3: Antonio Charfauros, Paul Castillo, Cain Green, Abrianna Garcia

// Function prototype
void displayCacheInput(int cacheSize, int blockSize, int associativity, char* replacementPolicy, int physicalMemory, float percentUsed, int instructions, char* filename[], int fileNumber);
void printCacheCalculations( int totalBlocks, int tagBits, int indexBits, int totalRows, int overheadSizeBytes, double implementationMemoryKB, int implementationMemoryBytes, double cacheCost );
void cacheCalculations(int cacheSize, int blocksize, int associativity, int physicalMemory);
void physicalMemoryCalculations( int physicalMemory, float percentUsed, int fileCount );
void printMemoryCalculations( int physicalPages, int systemPages, int pteBits, int totalPageTableRam );
int log2n( int n );

// USAGE EXAMPLE: VMCacheSim.exe –s 512 –b 16 –a 4 –r rr –p 1024 –n 100 –u 75 -f Trace1.trc -f Trace2_4Evaluation.trc –f Corruption1.trc
// Must accept all flags including up to 3 '-f's to read 3 trace files, 20 flags

int main(int argc, char* argv[]){
    
    // Input Parameters
    int cacheSize, blockSize, associativity, physicalMemory, instructionsTimeSlice;
    char* replacementPolicy;
    float percentUsed;
    int fileCount;
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
            fileCount = 1;
            break;
        
        // Two files provided
        case 19:
            files[0] = argv[16];
            files[1] = argv[18];
            displayCacheInput(cacheSize, blockSize, associativity, replacementPolicy, physicalMemory, percentUsed, instructionsTimeSlice, files, 2);
            fileCount = 2;
            break;

        // Three files provided
        case 21:
        files[0] = argv[16];
        files[1] = argv[18];
        files[2] = argv[20];
        displayCacheInput(cacheSize, blockSize, associativity, replacementPolicy, physicalMemory, percentUsed, instructionsTimeSlice, files, 3);
        fileCount = 3;
        break;

        default:
            printf("Error: invalid number of arguments or too many files provided\n");
            exit(1);
    }  

    // Call the Cache and Physical Memory functions
    cacheCalculations( cacheSize, blockSize, associativity, physicalMemory );
    physicalMemoryCalculations( physicalMemory, percentUsed, fileCount );

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
    printf("Percent Memory Used by System:  %.1f %%\n", percentUsed);
    printf("Instructions / Time slice:      %d\n", instructions);

}

void cacheCalculations(int cacheSize, int blockSize, int associativity, int physicalMemory){

    // Declare unsigned integer cacheSizeBytes
    unsigned int cacheSizeBytes;

    // Multiply the cache size by 1024
    cacheSizeBytes = cacheSize * 1024;

    // Calculate the total amount of blocks and rows based off block size and associativity
    int totalBlocks = cacheSizeBytes / blockSize;
    int totalRows   = totalBlocks / associativity;

    // Calculate the number of offset and index bits
    int offsetBits = log2n(blockSize);
    int indexBits  = log2n(totalRows);

    // Calculate the number of physical address bits
    int physicalAddressBits = log2n(physicalMemory * 1024 * 1024);

    // Calculate how many bits for the tag
    int tagBits = physicalAddressBits - indexBits - offsetBits;

    // Calculate overhead bits
    int overheadBitsPerBlock = tagBits + 1;

    // Calculate the overhead size in total bytes
    int overheadSizeBytes = (totalBlocks * overheadBitsPerBlock) / 8;

    // Calculate the cost and memory bytes 
    int implementationMemoryBytes = cacheSizeBytes + overheadSizeBytes;
    double implementationMemoryKB = (double)implementationMemoryBytes / 1024.0;

    // Find the cost value
    double cacheCost = implementationMemoryKB * 0.07;

    // Send values to be printed by the print function
    printCacheCalculations( totalBlocks, tagBits, indexBits, totalRows, overheadSizeBytes, implementationMemoryKB, implementationMemoryBytes, cacheCost );

}

void physicalMemoryCalculations( int physicalMemory, float percentUsed, int fileCount ) {

    // Calculate the physical memory pages
    int physicalPages = ( physicalMemory * 1024 * 1024 ) / 4096;

    // Find the percent used
    float percent = percentUsed * 0.01f;
    int systemPages = percent * physicalPages;

    // Find the page table entry bits
    int pteBits = log2n( physicalPages ) + 1;

    // calculate the page table RAM
    int totalPageTableRam = ( 524288 * fileCount * pteBits ) / 8 ;

    // Print out the calculations
    printMemoryCalculations( physicalPages, systemPages, pteBits, totalPageTableRam );


}

void printMemoryCalculations( int physicalPages, int systemPages, int pteBits, int totalPageTableRam ) {

    // Display physical memory values
    printf( "\n***** Physical Memory Calculated Values *****\n\n" );
    printf( "%-31s %d\n","Number of Physical Pages:", physicalPages );
    printf( "%-31s %d\n","Number of Pages for System:", systemPages );
    printf( "%-31s %d bits\n","Size of Page Table Entry:", pteBits );
    printf( "%-31s %d bytes\n","Total RAM for Page Table(s):", totalPageTableRam );

}

void printCacheCalculations( int totalBlocks, int tagBits, int indexBits, int totalRows, int overheadSizeBytes, double implementationMemoryKB, int implementationMemoryBytes, double cacheCost ){

    // Display argument information in provided format
    printf( "\n***** Cache Calculated Values *****\n\n" );
    printf( "%-31s %d\n","Total # Blocks:", totalBlocks );
    printf( "%-31s %d bits\n","Tag Size:", tagBits );
    printf( "%-31s %d bits\n","Index Size:", indexBits );
    printf( "%-31s %d\n","Total # Rows:", totalRows );
    printf( "%-31s %d bytes\n","Overhead Size:", overheadSizeBytes );
    printf( "%-31s %.2lf KB (%d bytes)\n","Implementation Memory Size:", implementationMemoryKB, implementationMemoryBytes );
    printf( "%-31s $%.2lf @ $0.07 per KB\n","Cost:", cacheCost );
	
    
}

// Log function for some of the calculations
int log2n( int n ) {

	int x = 0;
	while ( n > 1 ) {
		n /= 2;
		x++;
	}
	return x;
}
