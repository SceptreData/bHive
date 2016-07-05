#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "bhive.h"

#define NUM_BEES 8
#define NUM_TASKS 10000

#define MUTEX_L(m) (pthread_mutex_lock(m))
#define MUTEX_U(m) (pthread_mutex_unlock(m))

int jobsCompleted = 0;
pthread_mutex_t lock;

void dumbFunc( void *p );
void forceTest( void *p );

int main(void)
{
    pthread_mutex_init(&(lock), NULL);

    bHive_t *hive = bHive_new(NUM_BEES);
    assert(hive);

    for (int i = 0; i < NUM_TASKS; i++){
        bHive_addJob(hive, &dumbFunc, NULL);
    }

    time_t start_time = time(NULL);
    bHive_forceJob(hive, &forceTest, NULL);
    bHive_forceJob(hive, &forceTest, NULL);

    while(jobsCompleted <= NUM_TASKS){
        usleep(1000);
    }
    double total_time = difftime(time(NULL), start_time);

    int r = bHive_destroy(hive);
    assert(r == 0);
    printf("Tasks Completed: %d \n", jobsCompleted);
    printf("Time elapsed: %.2f seconds\n", total_time);
    return 0;
}

void dumbFunc( void *p )
{
    usleep(5000);
    MUTEX_L(&(lock));
    jobsCompleted++;
    MUTEX_U(&(lock));
 }

void forceTest( void *p )
{
    printf("Yeah this thing shot right to the front didn't it!\n");
    MUTEX_L(&(lock));
    jobsCompleted++;
    MUTEX_U(&(lock));
}
