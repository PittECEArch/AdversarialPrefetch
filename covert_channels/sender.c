#define _GNU_SOURCE
#include "util.h"
#include "math.h"
#include "sched.h"
#include "pthread.h"

void send_bit (bool bit, struct config *config)
{
	CYCLES start_t = cc_sync();
    asm volatile("lfence");
    int i = 0;
    
	if (bit) {
		while ((get_time() - start_t) < config->interval) {
            //send the bit at the very beginning of the interval
            if(i == 0)
            {
                memaccess(config->addr);
            }
            i++;
		}	

	} else {
		while (get_time() - start_t < config->interval) {
            i++;
        }
	}

}

void* sender_func(void* param) {
    printf("thead launched\n");
	struct config config;
	init_config(&config);

    //access the target data to make sure prefetchw works later.
    CYCLES access_time = measure_one_block_access_time(config.addr);
    access_time = measure_one_block_access_time(config.addr);

    int mm = 0;
	while (1) {
        //send "1" and "0" alternatively
        if (mm %2 == 0)
            send_bit(true, &config);
        else 
            send_bit(false, &config);
        mm ++;
	}
}



int main(int argc, char *argv[]) {
  pthread_t sender;
  printf("%s\n", "Starting\n");

  pthread_attr_t attr_sender;
  pthread_attr_init(&attr_sender);

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(sender_core, &mask);
  if (pthread_attr_setaffinity_np(&attr_sender, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: sender");
  }

  printf("created thread\n");

  if (pthread_create(&sender, &attr_sender, (void *)sender_func, NULL) !=
      0) {
    perror("pthread_create: sender");
  }
  pthread_join(sender, NULL);
}







