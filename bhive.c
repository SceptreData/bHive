#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bhive.h"

typedef struct job_s
{
    void (*func)(void *);
    void *arg;
    struct job_s *next;
} job_t;

struct bHive_s
{
    pthread_t   *workerBees;
    int         totalBees;
    int         activeBees;

    job_t       *head;
    job_t       *tail;
    int         numJobs;
    pthread_mutex_t hiveLock;
    pthread_cond_t availableJobs;

    int closing;
};

static void *newBee(void *p);

bHive_t *bHive_new( size_t numBees)
{
    bHive_t *newHive = malloc(sizeof(*newHive));
    if(newHive == NULL ){
        printf("Memory Error.\n");
        exit(1);
    }
    newHive->workerBees = malloc( numBees * sizeof(pthread_t));
    if(newHive->workerBees == NULL ){
        printf("Memory Error.\n");
        exit(1);
    }
    if (pthread_mutex_init( &(newHive->hiveLock), NULL) != 0 ){
        printf("Error initializing mutex.");
        exit(1);
    }
    if (pthread_cond_init( &(newHive->availableJobs), NULL) != 0 ){
        printf("Error initializing pthread Condition.");
        exit(1);
    }
    newHive->totalBees = numBees;
    newHive->activeBees = 0;
    newHive->head = NULL;
    newHive->tail = NULL;
    newHive->numJobs = 0;
    newHive->closing = 0;

    //Summon the bees!
    for (int i = 0; i < numBees; i++){
        if( pthread_create(&(newHive->workerBees[i]), NULL, newBee, (void *)newHive) != 0){
            printf("Error creating thread for Worker Bees!");
            exit(1);
        }
    }
    return newHive;
}

int bHive_addJob( bHive_t *hive, void (*func)(void *), void *arg)
{
    if( hive->numJobs >= MAX_JOBS ){
        printf("bHive Queue is currently Full.\n");
        return 0;
    }

    job_t *j = malloc(sizeof(*j));
    if ( j == NULL ){
        printf("Memory Error.\n");
        exit(1);
    }

    j->func = func;
    j->arg = arg;
    j->next = NULL;
    
    pthread_mutex_lock(&(hive->hiveLock));
    if(hive->closing){
        free(j);
        printf("Error creating job, Hive in shutdown process.\n");
        return -1;
    }
    //Add to hive Queue
    if (hive->head == NULL && hive->tail == NULL){
        hive->head = j;
        hive->tail = j;
    }
    else if (hive->head == hive->tail){
        hive->head->next = j;
        hive->tail = j;
    } else {
        hive->tail->next = j;
        hive->tail = j;
    }
    hive->numJobs++;

    // Alert the bees! There is work to be done!
    pthread_cond_signal(&(hive->availableJobs));
    pthread_mutex_unlock(&(hive->hiveLock));
   
   /* Return the number of available jobs left. */ 
    return (hive->numJobs - MAX_JOBS);
}
        

static job_t *bHive_getJob(bHive_t *hive)
{
    job_t *j = NULL;
    if( hive->head == NULL ){
        printf("Error, job Queue is empty.");
        return NULL;
    }
    if( hive->head == hive->tail ){
        j = hive->head;
        hive->head = NULL;
        hive->tail = NULL;
    } else {
        j = hive->head;
        hive->head = j->next;
    }
    hive->numJobs--;
    return j;
}

static void *newBee(void *p)
{
    bHive_t *hive = (bHive_t*)p;
    job_t *job;
    int wasActive = 0;
    while(1){

        pthread_mutex_lock(&(hive->hiveLock));

        if (wasActive){
            hive->activeBees--;
            wasActive = 0;
        }
        
        while( (hive->numJobs == 0) && (hive->closing == 0)){
            pthread_cond_wait(&(hive->availableJobs), &(hive->hiveLock));
        }

        if (hive->closing){
            break;
        }

        job = bHive_getJob(hive);
        hive->activeBees++;
        pthread_mutex_unlock(&(hive->hiveLock));

        job->func(job->arg);
        free(job);
        wasActive = 1;
    }
    hive->totalBees--;
    pthread_mutex_unlock(&(hive->hiveLock));
    pthread_exit(NULL);
    return NULL;
}

int bHive_destroy( bHive_t *hive )
{
    if ( hive == NULL ){
        printf("Trying to destroy NULL hive.\n");
        return 1;
    }

    pthread_mutex_lock(&(hive->hiveLock));
    
    if( hive->closing ){
        printf("Hive is already closing down\n");
        return 1;
    }

    hive->closing = 1;
    
    /* Wake up all the bees, they will see that the hive
       is closing and go into their shutdown routine. */
    pthread_cond_broadcast(&(hive->availableJobs));
    pthread_mutex_unlock(&(hive->hiveLock));

    // Now Join all the threads. Can use this for an error code?
    int r = 0;
    for (int i = 0; i < hive->totalBees; i++){
        r += pthread_join(hive->workerBees[i], NULL);
    }

    if( r != 0 ){
        printf("Error shutting down beeThreads!\n");
        return(1);
    }

    if( hive->numJobs > 0 || hive->head != NULL){
        printf("Killing hive with %d remaining jobs.\n", hive->numJobs);
        job_t *tmp = hive->head;
        while(tmp){
            hive->head = tmp->next;
            free(tmp->arg);
            free(tmp);
            tmp = hive->head;
        }
    }

    free(hive->workerBees);
    pthread_mutex_destroy(&(hive->hiveLock));
    pthread_cond_destroy(&(hive->availableJobs));

    free(hive);

    return 0;
}
