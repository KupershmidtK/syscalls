/*****************************************************
*
* MEMBLOCKTEST.C
*
*****************************************************/

#include <stdbool.h>
#include <linux/kernel.h>
#include <linux/memblockinfo.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define __NR_memblockinfo 454

long memblockinfo_syscall(struct memblockinfo* buffer, unsigned long len)
{
    return syscall(__NR_memblockinfo, buffer, len);
}

static void print_memblockinfo_type(struct memblockinfo_type* memtype) {
    printf("\n");
    printf("Name: %s\n", memtype->name);
    printf("Number of regions: %ld\n", memtype->cnt);
    printf("Size of the allocated array: %ld\n", memtype->max);
    printf("Size of all regions: %llu\n", memtype->total_size);


    printf("base\t\tsize\t\tflag\n");
    printf("----------------------------------------------------\n");
    
    struct memblockinfo_region* region = NULL;
    for (region = memtype->regions;
	     region < (memtype->regions +memtype->cnt);
	     region++) {

        char* flags = 
            region->flags == MEMBLOCKINFO_NONE
            ? "MEMBLOCK_NONE" :
            region->flags == MEMBLOCKINFO_HOTPLUG
            ? "MEMBLOCK_HOTPLUG" :
            region->flags == MEMBLOCKINFO_MIRROR
            ? "MEMBLOCK_MIRROR" :
            region->flags == MEMBLOCKINFO_NOMAP
            ? "MEMBLOCK_NOMAP" :
            region->flags == MEMBLOCKINFO_DRIVER_MANAGED
            ? "MEMBLOCK_DRIVER_MANAGED" :
            "NO FLAGS";

        printf(
            "%12llu\t%12llu\t%s\n",
            region->base,
            region->size,
            flags);
    }
} 

void print_memblockinfo(struct memblockinfo* memblock) {
    printf(">>> Struct MEMBLOCK information <<<\n\n");

    printf("Bottom Up: %s\n", memblock->bottom_up ? "TRUE" : "FALSE");
    printf("Current limit: %llu\n", memblock->current_limit);

    print_memblockinfo_type(&memblock->memory);
    print_memblockinfo_type(&memblock->reserved);
}

int main(int argc, char *argv[])
{
    struct memblockinfo memblock;

    long rc;
    rc = memblockinfo_syscall(&memblock, sizeof(memblock));

    if(rc < 0) {
        perror("System call is failed");
        return 1;
    }

    print_memblockinfo(&memblock);
    return 0;
}