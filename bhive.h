#ifndef _BHIVE_H_
#define _BHIVE_H_

/*
 *   bHive v.01 2016
 *   David Bergeron
 *   Simple thread pool
 */

#include <unistd.h>
#include <pthread.h>

#define MAX_JOBS 10000
#define MAX_BEES 16

typedef struct bHive_s bHive_t;

bHive_t *bHive_new( size_t numBees );

/*
----------------------------------------------------------------------
bHive_addJob adds a job to the end of the job Queue.
----------------------------------------------------------------------
*/
int bHive_addJob( bHive_t *hive, void (*func)(void *), void *arg);

/*
----------------------------------------------------------------------
bHive_forceJob adds a job to the start of the job Queue.
    NOTE: Does not technically "Force" a job by ending processes elsewhere,
          it simply ensures the job is at the front of the queue to be 
          processed next.
----------------------------------------------------------------------
*/
int bHive_forceJob( bHive_t *hive, void (*func)(void *), void *arg);

int bHive_destroy( bHive_t *hive );

#endif
