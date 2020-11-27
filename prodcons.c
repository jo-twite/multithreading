#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <semaphore.h>
#include <limits.h>
#include "stack.c"

#define BUFF_SIZE 8 // buffer size
#define NDATA 128  // Number of data to produce (and consume)


int nprod, ncons;       // Number of producers and consumers
pthread_mutex_t mutex;   // Mutex for the first/second half of the buffer
pthread_mutex_t inc;
sem_t empty;             // Semaphore to signal that the 1st/2nd half is empty
sem_t full;              // Semaphore to signal that the 1st/2nd half is full


int buffer[BUFF_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};
queue* prod_tickets, *cons_tickets;



/* FUNCTIONS */


void* producer(void* arg);      // producer function
void* consumer(void* arg);      // consumer function
//void* exec_prod(void* param);
//void* exec_cons(void* param);
int produce();                  // produce a random number
int insert_item(int elem, int in);    // insert elem at buffer[in]
int remove_item(int out);     // remove item at buffer[out]
int increment(int *x);
void signal(sem_t* sem);


int inserted = 0; // producer counter
int removed = 0; // consumer counter
int filled = 0;

/**
 * Concurrent ticket production
 * Wait for the 
 * Parralel insertion
 * If current thread inserted the 8th element, signal consumers
 */
void* producer(void* arg)
{
    int item, ticket, rest;
    while(prod_tickets != NULL && (ticket = queue_pop(&prod_tickets))!= -1) // Concurrent ticket production
    {
        item = produce();                           // Produce the random number
        sem_wait(&empty); 
        //pthread_mutex_lock(&mutex);                 
        rest = insert_item(item, ticket);           // parrallel insertion in buffer
        //pthread_mutex_unlock(&mutex);               
        if(!rest) 
            signal(&full);
    }
    signal(&full);
    return NULL;
}

void* consumer(void* arg)
{   
    int ticket, rest;
    while(cons_tickets != NULL && (ticket = queue_pop(&cons_tickets)) != -1)
    {
        sem_wait(&full);
        //pthread_mutex_lock(&mutex);
        rest = remove_item(ticket);
        //pthread_mutex_unlock(&mutex);
        if(!rest) 
            signal(&empty);
    }
    return NULL;
}

int insert_item(int elem, int in)
{
    int k = in%BUFF_SIZE;
    int x = buffer[k];
    buffer[k] = elem;
    //(rand() - 2147483647 / 2) * (rand() % 2) + 1;
    printf("Buffer[%d] has been produced (ticket P%d): ancient value: (%d)\n", k, in, x);
    pthread_mutex_lock(&inc);
    k = increment(&inserted);
    pthread_mutex_unlock(&inc);
    return k;
}

int produce()
{
    int it = rand(); // Produce an random item
    while(it < RAND_MAX/1000);
    return it;
}


int remove_item(int out)
{
    int k = out%BUFF_SIZE;
    int consumed = buffer[k];
    buffer[k] = 0;
    printf("Buffer[%d] has been consumed (ticket C%d): %d\n", k, out+1, consumed);
    pthread_mutex_lock(&inc);
    k = increment(&removed);
    pthread_mutex_unlock(&inc);
    return k;
}

int increment(int* counter) {  return (++ *counter) % BUFF_SIZE; }

void signal(sem_t* sem)
{
    int signal[8] = {
        sem_post(sem), sem_post(sem),
        sem_post(sem), sem_post(sem), 
        sem_post(sem), sem_post(sem), 
        sem_post(sem), sem_post(sem) 
    };
}

//Ce main ne fonctionne que pour 1 producer et 1 consommateur, à généraliser
int main(int argc, char** argv)
{
    clock_t begin = clock();
    
    nprod = atoi(argv[1]); // number of producers
    ncons = atoi(argv[2]); // number of consumers

    if(nprod < 1 || ncons < 1 || nprod + ncons > 8) {
        printf("[Illegal arguments]] argv[1]: %s, argv[2]: %s\n", argv[1], argv[2]);
        return 1;
    }
    // We lose one consumer and one producer by executing consumer and producerS
    

    pthread_t thread_p[nprod];
    pthread_t thread_c[ncons];
    int i = 0;
    sem_init(&empty, 0, 8);
    sem_init(&full, 0, 0);
    init_queue(&prod_tickets, NDATA);
    init_queue(&cons_tickets, NDATA);

    for(; i < nprod; i++)
        pthread_create(&thread_p[i], NULL, producer, NULL);
    for(i = 0; i < nprod; i++)
        pthread_create(&thread_c[i], NULL, consumer, NULL);
    
    for(; i < nprod; i++)
        pthread_join(thread_p[i], NULL);
    for(i = 0; i < nprod; i++)
        pthread_join(thread_c[i], NULL);


    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("\n{%d, %d, %d, %d, %d, %d, %d, %d}\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    printf("\nExecution time: %f CLOCK PER SECOND\n\n", time_spent);
}