#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <semaphore.h>
#include <limits.h>
#include "stack.c"

#define BUFF_SIZE 8 // buffer size
#define NDATA 1024  // Number of data to produce (and consume)


int nprod, ncons;       // Number of producers and consumers
pthread_mutex_t mutex;   // Mutex for the first/second half of the buffer
pthread_mutex_t inc[2];
sem_t empty;             // Semaphore to signal that the 1st/2nd half is empty
sem_t full;              // Semaphore to signal that the 1st/2nd half is full
sem_t done;

int buffer[BUFF_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};

queue* prod_tickets, *cons_tickets;

int prod_counter = 0, cons_counter = 0;



/* FUNCTIONS */


void* producer(void* arg);      // producer function
void* consumer(void* arg);      // consumer function
//void* exec_prod(void* param);
//void* exec_cons(void* param);
int produce();                  // produce a random number
void insert_item(int elem, int in);    // insert elem at buffer[index]
void remove_item(int out);     // remove item at buffer[index]
void signal(sem_t* sem);


int inserted = 0; // producer counter
int removed = 0; // consumer counter
int filled = 0;



void* producer(void* arg)
{
    int item, ticket;
    while(prod_tickets != NULL)
    {
        item = produce();
        ticket = queue_pop(&prod_tickets);
        if(ticket == -1)
            return NULL;
        sem_wait(&empty); // wait 
        //pthread_mutex_lock(&mutex);
        insert_item(item, ticket); //fill the buffer concurrently
        //pthread_mutex_unlock(&mutex);
        if(ticket%BUFF_SIZE == 7) {
            sem_wait(&done);
            signal(&full);
        }
            //printf("Ready to be consumed\n");
    }
    return NULL;
}

void* consumer(void* arg)
{   
    int ticket;
    while(cons_tickets != NULL)
    {
        ticket = queue_pop(&cons_tickets);
        if(ticket == -1)
            return NULL;
        sem_wait(&full);
        //pthread_mutex_lock(&mutin%BUFF_SIZE
        
        remove_item(ticket);
        //pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }
    return NULL;
}

void insert_item(int elem, int in)
{
    int k = in%BUFF_SIZE;
    int x = buffer[k];
    buffer[k] = elem;
    //(rand() - 2147483647 / 2) * (rand() % 2) + 1;
    printf("Buffer[%d] has been produced (P%d): ancient value: (%d)\n", k, in+1, x);
}

int produce()
{
    int it = rand(); // Produce an random item
    while(it < RAND_MAX/1000);
    return it;
}


void remove_item(int out)
{
    int consumed = buffer[out%BUFF_SIZE];
    buffer[out%BUFF_SIZE] = 0;
    printf("Buffer[%d] has been consumed (C%d): %d\n", out%BUFF_SIZE, out, consumed);
}

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