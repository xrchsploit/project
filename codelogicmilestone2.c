// Initialize processes

void initializeProcesses(Process processes[], int numTraceFiles)
{
    int i, j;

    for (i = 0; i < numTraceFiles; i++)
    {
        processes[i].usedPageTableEntries = 0;

        for (j = 0; j < PAGE_TABLE_ENTRIES; j++)
        {
            processes[i].pageTable[j].valid = 0;
        }
    }
}



// Initialize phyiscal Pages
void initializePhysicalPages(PhysicalPage physicalPages[], unsigned int userPages)
{
    unsigned int i;

    for (i = 0; i < userPages; i++)
    {
        physicalPages[i].inUse = 0;
        physicalPages[i].ownerProcess = -1;
    }
}


// Convert Address - > Virtual page
unsigned int getVirtualPage(unsigned int address)
{
    return address >> 12;   // divide by 4096
}


// Find Free phyiscal page

int findFreePhysicalPage(PhysicalPage physicalPages[], unsigned int userPages)
{
    unsigned int i;

    for (i = 0; i < userPages; i++)
    {
        if (physicalPages[i].inUse == 0)
            return i;
    }

    return -1;
}



// Handle ONE memory access Core Logic

void handleAddressAccess(Process processes[], int processIndex,
                         unsigned int address,
                         PhysicalPage physicalPages[],
                         unsigned int userPages,
                         VMStats *vmStats)
{
    unsigned int virtualPage = getVirtualPage(address);
    int freePage;

    vmStats->virtualPagesMapped++;

    /* Page already mapped */
    if (processes[processIndex].pageTable[virtualPage].valid == 1)
    {
        vmStats->pageTableHits++;
        return;
    }

    /* Try to allocate from free pages */
    freePage = findFreePhysicalPage(physicalPages, userPages);

    if (freePage != -1)
    {
        processes[processIndex].pageTable[virtualPage].valid = 1;
        processes[processIndex].pageTable[virtualPage].physicalPage = freePage;

        processes[processIndex].usedPageTableEntries++;

        physicalPages[freePage].inUse = 1;
        physicalPages[freePage].ownerProcess = processIndex;

        vmStats->pagesFromFree++;
    }
    else
    {
        vmStats->totalPageFaults++;
    }
}


// Simulate virtual memory main loop


void simulateVirtualMemory(Process processes[], int numTraceFiles,
                           PhysicalPage physicalPages[],
                           unsigned int userPages,
                           VMStats *vmStats)
{
    FILE *fp;
    char line1[256], line2[256];

    unsigned int eip, dst, src, len;
    int i;

    for (i = 0; i < numTraceFiles; i++)
    {
        fp = fopen(processes[i].traceFileName, "r");

        if (fp == NULL)
        {
            printf("Error opening file %s\n", processes[i].traceFileName);
            continue;
        }

        while (fgets(line1, sizeof(line1), fp) &&
               fgets(line2, sizeof(line2), fp))
        {
            /* Parse EIP */
            sscanf(line1, "EIP (%u): %x", &len, &eip);

            handleAddressAccess(processes, i, eip,
                                physicalPages, userPages, vmStats);

            /* Parse dst/src */
            char dstData[16], srcData[16];

            sscanf(line2, "dstM: %x %8s srcM: %x %8s",
                   &dst, dstData, &src, srcData);

            if (strcmp(dstData, "--------") != 0)
            {
                handleAddressAccess(processes, i, dst,
                                    physicalPages, userPages, vmStats);
            }

            if (strcmp(srcData, "--------") != 0)
            {
                handleAddressAccess(processes, i, src,
                                    physicalPages, userPages, vmStats);
            }
        }

        fclose(fp);
    }
}