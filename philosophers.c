#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <semaphore.h>

#define NCYCLES 100

int NTHREAD;
int counter = 0;

pthread_mutex_t mutex;
pthread_mutex_t* sticks;
sem_t* put;
int* monitor;

void* philosophe(void* arg);
void manual_event_init();
int update(int index);


int main(int argc, char** argv)
{
    NTHREAD = atoi(argv[1]);
    if(NTHREAD < 2 || NTHREAD > 2* get_nprocs()) {
        printf("Illegal arguments: %d.\n", NTHREAD);
        exit(-1);
    }
    
    pthread_t* thread = (pthread_t*) malloc(NTHREAD * sizeof(pthread_t));
    manual_event_init();

    int i = 0;    
    for(; i < NTHREAD; i++)
        pthread_create(&thread[i], NULL, philosophe, (void*) &i);
    for(i = 0; i < NTHREAD; i++)
        pthread_join(thread[i],NULL);
    return 0;
}

void* philosophe(void* arg)
{
    int left = *((int*) arg);
    int right = (left + 1)%NTHREAD;
    
    int l = (left = 0)? NTHREAD-1 : left-1;
    int updated[2];

    while(counter < NCYCLES * NTHREAD) 
    {
        printf("Philosopher %d thinking .\n", left);
        sem_wait(&put[left]);

        pthread_mutex_lock(&sticks[left]);
        pthread_mutex_lock(&sticks[right]);

        counter ++;
        printf("Philosopher %d is eating ..\n", left+1);
        updated[0] = update(l);
        updated[1] = update(right);

        pthread_mutex_unlock(&sticks[left]);
        pthread_mutex_unlock(&sticks[right]);
        if(updated[0])
            sem_post(&put[l]);
        if(updated[1])
            sem_post(&put[right]);
        //printf("Philosopher %d finished eating ...\n", left+1);
    }
    return NULL;
}

int update(int index) 
{
    int k;
    if ((k = (++ monitor[index])) == 2)
        monitor[index] = 0;
    return k;
}


/**
  * Create a monitor of {0, ... ,0}
  * 
  * 
  */
void manual_event_init()
{
    monitor = (int*) calloc(NTHREAD, sizeof(int));

    put = (sem_t*) malloc(NTHREAD * sizeof(sem_t));
    int i;
    if(NTHREAD == 2) { 
        sem_init(&put[0], 0, 1); sem_init(&put[1], 0, 0); 
    }
    else if(NTHREAD%2) {  // Odd and > 2
        for(i = 1; i < NTHREAD; i+= 2) {
            sem_init(&put[i-1], 0, 0);
            sem_init(&put[i], 0, 1);
        }
        sem_init(&put[i], 0, 0);
    }
    else { // Pair and > 2
        for(i = 0; i < NTHREAD; i+=2){
            sem_init(&put[i], 0, 1);
            sem_init(&put[i+1], 0, 0); 
        }
    }
}