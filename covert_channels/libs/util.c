
#include "util.h"

CYCLES memaccess(ADDR_PTR addr)
{
    CYCLES rv;
    asm volatile("mov (%1), %0" : "+r"(rv) : "r"(addr) :);
    return rv;

}



/*
 * Loads from virtual address addr and measure the access time
 */
CYCLES measure_one_block_access_time(ADDR_PTR addr)
{
    CYCLES cycles;

    asm volatile("mov %1, %%r8\n\t"
            "lfence\n\t"
            "rdtsc\n\t"
            "mov %%eax, %%edi\n\t"
            "mov (%%r8), %%r8\n\t"
            "lfence\n\t"
            "rdtsc\n\t"
            "sub %%edi, %%eax\n\t"
    : "=a"(cycles) /*output*/
    : "r"(addr)
    : "r8", "edi");

    return cycles;
}

/* 
 * Returns Time Stamp Counter 
 */
extern inline __attribute__((always_inline))
CYCLES rdtscp(void) {
	CYCLES cycles;
	asm volatile ("rdtscp"
	: /* outputs */ "=a" (cycles));

	return cycles;
}

/* 
 * Gets the value Time Stamp Counter 
 */
inline CYCLES get_time() {
    return rdtscp();
}

/* Synchronizes at the overflow of a counter
 */
extern inline __attribute__((always_inline))
CYCLES cc_sync() {
    while((get_time() % CHANNEL_SYNC_TIMEMASK) > CHANNEL_SYNC_JITTER) {
        }
    return get_time();
}



/*
 * Flushes the cache block accessed by a virtual address out of the cache
 */
extern inline __attribute__((always_inline))
void clflush(ADDR_PTR addr)
{
    asm volatile ("clflush (%0)"::"r"(addr));
}


/*
 * Initializes the struct config
 */
void init_config(struct config *config)
{
	int offset = FILE_OFFSET;
	config->interval = CHANNEL_INTERVAL;
	char *filename = FILE_NAME;

	// Map file to virtual memory and extract the address at the file offset
	if (filename != NULL) {
		int inFile = open(filename, O_RDONLY);
		if(inFile == -1) {
			printf("Failed to Open File\n");
			exit(1);
		}

		void *mapaddr = mmap(NULL,FILE_SIZE,PROT_READ,MAP_SHARED,inFile,0);

		if (mapaddr == (void*) -1 ) {
			printf("Failed to Map Address\n");
			exit(1);
		}

		config->addr = (ADDR_PTR) mapaddr + offset;
	}

}

