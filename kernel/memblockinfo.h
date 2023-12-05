#ifndef _MEMSYSDATA_H_
#define _MEMSYSDATA_H_

enum memblockinfo_flags {
	MEMBLOCKINFO_NONE		= 0x0,	/* No special request */
	MEMBLOCKINFO_HOTPLUG	= 0x1,	/* hotpluggable region */
	MEMBLOCKINFO_MIRROR		= 0x2,	/* mirrored region */
	MEMBLOCKINFO_NOMAP		= 0x4,	/* don't add to kernel direct mapping */
	MEMBLOCKINFO_DRIVER_MANAGED = 0x8	/* always detected via a driver */
};

struct memblockinfo_region {
	long long unsigned base;
	long long unsigned size;
	enum memblockinfo_flags flags;
};

struct memblockinfo_type {
	unsigned long cnt;
	unsigned long max;
	long long unsigned total_size;
	struct memblockinfo_region regions[128];
	char name[32];
};

struct memblockinfo {
    bool bottom_up;  /* is bottom up direction? */
	long long unsigned current_limit;
	struct memblockinfo_type memory;
	struct memblockinfo_type reserved;
};

#endif