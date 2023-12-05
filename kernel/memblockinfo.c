/***********************************************
*  
* MEMBLOCKINFO.C
*
************************************************/

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/memblock.h>
#include "memblockinfo.h"

static struct memblockinfo memblockinfo;

SYSCALL_DEFINE2(memblockinfo, void*, buffer, unsigned long, len)
{
    if(len < sizeof(struct memblockinfo))
        return -EINVAL;

        
    // ---------- copy memblock ---------------
    memblockinfo.bottom_up = memblock.bottom_up;
    memblockinfo.current_limit = memblock.current_limit;

    memblockinfo.memory.cnt = memblock.memory.cnt;
    memblockinfo.memory.max = memblock.memory.max;
    memblockinfo.memory.total_size = memblock.memory.total_size;
    strcpy(memblockinfo.memory.name, memblock.memory.name);

    memblockinfo.reserved.cnt = memblock.reserved.cnt;
    memblockinfo.reserved.max = memblock.reserved.max;
    memblockinfo.reserved.total_size = memblock.reserved.total_size;
    strcpy(memblockinfo.reserved.name, memblock.reserved.name);

    struct memblock_region* region = NULL;
    int i = 0;
    for_each_mem_region(region) {
        memblockinfo.memory.regions[i].flags = (enum memblockinfo_flags)region->flags;
        memblockinfo.memory.regions[i].base = region->base;
        memblockinfo.memory.regions[i].size = region->size;
        i++;
    }

    i = 0;
    for_each_reserved_mem_region(region) {
        memblockinfo.reserved.regions[i].flags = (enum memblockinfo_flags)region->flags;
        memblockinfo.reserved.regions[i].base = region->base;
        memblockinfo.reserved.regions[i].size = region->size;
        i++;
    }

    if (copy_to_user(buffer, &memblockinfo, sizeof(memblockinfo)))
        return -EFAULT;
    
    return 0;
}
