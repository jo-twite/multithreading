#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t ticket;


typedef struct recursive {
    int value;
    struct recursive* next;
} node;

typedef struct list {
    node *head;
} queue;

void init_queue(queue** myqueue, int range)
{
    if (range < 1) return;
    *myqueue = (queue*) malloc(sizeof(queue));
    (*myqueue)->head = (node*) malloc(sizeof(node));
    node* current = (*myqueue)->head;
    for(int i = 1; i < range; i++)
    {
        current->next = malloc(sizeof(node));
        current->next->value = i;
        current = current->next;
    }
}

int queue_head(queue* q) 
{ 
    if(!q) return -1; 
    return q->head->value;
}

int queue_pop(queue** myqueue)
{
    pthread_mutex_lock(&ticket);
    node* q = (*myqueue)->head;
    if(q == NULL) return -1;
    int ret = q->value;
    (*myqueue)->head = (*myqueue)->head->next;
    free(q);
    q = NULL;
    pthread_mutex_unlock(&ticket);
    if((*myqueue)->head == NULL)
    {
        free((*myqueue)->head);
        (*myqueue)->head = NULL;
        free(*myqueue);
        *myqueue = NULL;
    }
    return ret;
}

/*
int main()
{
    queue* myqueue;
    init_queue(&myqueue, 10);
    for(node* current = myqueue->head; current != NULL; current = current->next)
        printf("%d ", current->value);
     printf("\n");
     return 0;
}*/