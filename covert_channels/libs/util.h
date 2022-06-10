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

#define sender_core 0
#define receiver_core 1

#define rounds 1000000

#define PRE_MISS_LATENCY 100


#define CHANNEL_DEFAULT_INTERVAL        0x000170
#define CHANNEL_SYNC_TIMEMASK           1400
#define CHANNEL_SYNC_JITTER             0x100

#define DEFAULT_FILE_NAME "shared"
#define DEFAULT_FILE_OFFSET	0x0
#define DEFAULT_FILE_SIZE	4096
#define CACHE_BLOCK_SIZE	64
#define MAX_BUFFER_LEN	1024


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

char *conv_char(char *data, int size, char *msg);

void init_config(struct config *config);

#endif
