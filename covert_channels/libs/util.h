#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


#ifndef UTIL_H_
#define UTIL_H_

#define ADDR_PTR uint64_t
#define CYCLES uint32_t



//The following parameters should be changed accordingly.

//The CPU core the sender is running on
#define sender_core 0 
//The CPU core the receiver/Spy of Prefetch+Prefetch/Prefetch+Reload is running on
#define receiver_core 1
//The CPU core Trojan in running on
#define receiver_helper_core 2 
//The number of rounds the receiver runs
#define ROUNDS 1000000
//The timing threshold for Prefetch+Prefetch
//Please see https://github.com/PittECEArch/AdversarialPrefetch#timing-characterization
#define PRE_HIT_LATENCY 100
//The timing threshold for Prefetch+Reload
#define LLC_S_LATENCY 70
//The temporal length of each iteration
#define CHANNEL_SYNC_TIMEMASK           0x7ff //Each iteration is 2000 cycles 
#define CHANNEL_INTERVAL                0x0003ff
#define CHANNEL_SYNC_JITTER             0x100
//The shared data used for data transmission 
#define FILE_NAME "shared"
#define FILE_OFFSET	0x0
#define FILE_SIZE	4096
#define CACHE_BLOCK_SIZE	64


struct config {
	ADDR_PTR addr;
	int interval;
};

CYCLES measure_one_block_access_time(ADDR_PTR addr);
CYCLES rdtscp(void);
CYCLES get_time();
CYCLES cc_sync();
CYCLES memaccess(ADDR_PTR addr);
 
void clflush(ADDR_PTR addr);

char *string_to_binary(char *s);


#endif
