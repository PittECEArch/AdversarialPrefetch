#define _GNU_SOURCE
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define trojan_core 0
#define spy_core 2

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int id = 0; 

/* this is from Mastik by Yuval Yarom */
static inline uint32_t memaccesstime(void *v) {
  uint32_t rv;
  asm volatile("mfence\n"
               "lfence\n"
               "rdtscp\n"
               "mov %%eax, %%esi\n"
               "mov (%1), %%eax\n"
               "rdtscp\n"
               "sub %%esi, %%eax\n"
               : "=&a"(rv)
               : "r"(v)
               : "ecx", "edx", "esi");
  return rv;
}

static inline uint32_t rdtscp()
{
    uint32_t rv;
    asm volatile("rdtscp" : "=a"(rv)::"edx", "ecx");
    return rv;
}


void *troj_func(int *param) {
  int i = 0;
  int time;
  while (i < 100000) {
    pthread_mutex_lock(&lock);
    while (id != 0) {
      pthread_cond_wait(&cond, &lock);
    }
    //Trojan loads in every other iteration.
    if (i % 2 == 0) {
        time = memaccesstime(param);
        printf("accessed ");
    } else {
      printf("notaccessed ");
    }
    i++;
    id = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
  }
}

void *spy_func(int *param) {
  int i = 0;
  int time = 0;
  while (i < 100000) {
    pthread_mutex_lock(&lock);
    while (id != 1) {
      pthread_cond_wait(&cond, &lock);
    }
    //Spy prefetches and times the prefetch in every iteration.
    asm volatile("mfence");
    int t1 = rdtscp();
    __builtin_prefetch(param, 1, 3);
    asm volatile("mfence");
    int t2 = rdtscp();

    printf("%d\n", t2 - t1);
    i++;
    id = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
  }
}

int main(int argc, char *argv[]) {
  pthread_t trojan, spy;
  printf("%s\n", "Starting\n");

  char*file = "shared";
  int fd = open(file, O_RDONLY);
  if (fd < 0)
      return 0;

  char *mapaddr = mmap(0, 4096, PROT_READ, MAP_SHARED, fd, 0);
  close(fd);

  
  int time;
  time = memaccesstime(mapaddr);
  printf("main access time %d\n", time);
  time = memaccesstime(mapaddr);
  printf("main access time %d\n", time);


  pthread_attr_t attr_troj, attr_spy;
  pthread_attr_init(&attr_troj);
  pthread_attr_init(&attr_spy);

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(trojan_core, &mask);

  if (pthread_attr_setaffinity_np(&attr_troj, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: trojan");
  }

  CPU_ZERO(&mask);
  CPU_SET(spy_core, &mask);
  if (pthread_attr_setaffinity_np(&attr_spy, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: spy");
  }

  if (pthread_create(&trojan, &attr_troj, (void *)troj_func, mapaddr) != 0) {
    perror("pthread_create: trojan");
  }
  if (pthread_create(&spy, &attr_spy, (void *)spy_func, mapaddr) != 0) {
    perror("pthread_create: spy");
  }
  pthread_join(trojan, NULL);
  pthread_join(spy, NULL);
}
