#define _GNU_SOURCE
#include "util.h"
#include "math.h"
#include "sched.h"
#include "pthread.h"



/*
 * Detects a bit by measuring the access time of the load from config->addr
 * within the clock length of config->interval.
 *
 * Detect a bit 1 if hits LLC E/M (remote L1)
 * Detect a bit 0 if hits LLC S
 *
 * Within one iteration, the operations from sender, Trojan (receiver_helper), Spy (receiver) are ordered by the amount of rdtscp() each thread runs before the operation.
 * This is a weak ordering, and the amount of rdtscp() each thread needs (the value of i) may need to be adjusted on each processor.
 */

void detect_bit(struct config *config, int index, int* result)
{
    int i = 0;
    CYCLES access_time; 

	// Sync with sender
	CYCLES start_t = cc_sync();
    asm volatile("lfence");
   
    while ((get_time() - start_t) < config->interval) {
        if (i == 2){
            access_time = measure_one_block_access_time(config->addr); 
            result[index] = (access_time > LLC_S_LATENCY);
        }
        i++;
	}
}


void prefetch_bit(struct config *config)
{
    int i = 0;

	// Sync with sender
	CYCLES start_t = cc_sync();
    asm volatile("lfence");
   
    while ((get_time() - start_t) < config->interval) {
       if (i >  5){
            __builtin_prefetch(config->addr, 1, 3);
        }
        i++;
	}
}




void* receiver_func( void* param)
{
	// Initialize config and local variables
	struct config config;
	init_config(&config);
    int counter = 0;
    int result[ROUNDS];

    //access the target addr a few times to ensure that prefetchw works later
    CYCLES access_time = measure_one_block_access_time(config.addr);
    access_time = measure_one_block_access_time(config.addr);
    access_time = measure_one_block_access_time(config.addr);
	
	while (counter < ROUNDS) {
        asm volatile("lfence");
		detect_bit(&config, counter, result);
        counter++;
	}
    
    for(int a = 0; a < ROUNDS; a++)
        printf("%d\n", result[a]);

	printf("Receiver finished\n");
}

void* helper_func( void* param)
{
	// Initialize config and local variables
	struct config config;
	init_config(&config);
    int counter = 0;

    //access the target addr a few times to ensure that prefetchw works later
    CYCLES access_time = measure_one_block_access_time(config.addr);
    access_time = measure_one_block_access_time(config.addr);
    access_time = measure_one_block_access_time(config.addr);

	while (counter < ROUNDS) {
        asm volatile("lfence");
		prefetch_bit(&config);
        counter++;
	}

	printf("Helper finished\n");
}



int main(int argc, char *argv[]) {
  pthread_t receiver, receiver_helper;
  printf("%s\n", "Starting\n");

  pthread_attr_t attr_receiver;
  pthread_attr_t attr_helper;
  pthread_attr_init(&attr_receiver);
  pthread_attr_init(&attr_helper);

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(receiver_core, &mask);
  if (pthread_attr_setaffinity_np(&attr_receiver, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: receiver");
  }

  CPU_ZERO(&mask);
  CPU_SET(receiver_helper_core, &mask);
  if (pthread_attr_setaffinity_np(&attr_helper, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: helper");
  }
  printf("created thread\n");

  if (pthread_create(&receiver, &attr_receiver, (void *)receiver_func, NULL) !=
      0) {
    perror("pthread_create: receiver");
  }
  if (pthread_create(&receiver_helper, &attr_helper, (void *)helper_func, NULL) !=
      0) {
    perror("pthread_create: helper");
  }

  pthread_join(receiver, NULL);
  pthread_join(receiver_helper, NULL);
}



