#ifndef BELT_H
#define BELT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#define SEM_WGHT "/semwght"
#define SEM_COUNT "/semcount"
#define SEM_GLOBAL "/semglobal"
#define SEM_LOADER "/semloader"
#define SEM_SHUTDOWN "/semshutdown"
#define SHM_PATH "/shmpath"

#define SEM_SEED 1
#define SHM_SEED 2
#define PERMISIONS 0666
#define MAX_QUEUE 1024

typedef struct QueueNode{
    int weight;
    time_t start_time;
    pid_t loader_id;
    int next;
}QueueNode;

typedef struct Queue{
    int head;
    int tail; // next of the last node
    QueueNode items[MAX_QUEUE];
    int wgth_max;
    int cap_max;
    int size;
    int weight;
}Queue;


Queue *init_queue(int wght, int cap){
    Queue *q = malloc(sizeof(Queue));
    q->wgth_max = wght;
    q->cap_max = cap;
    q->size = 0;
    q->weight = 0;
    q->head = 0;
    q->tail = 0;
    return q;
}

void delete_queue(Queue *q){
    free(q);
}

QueueNode *pop_queue(Queue *q){
    if(q->size == 0)return NULL;
    int temp = q->head;
    q->head = (q->head + 1)%MAX_QUEUE;
    q->size--;
    q->weight -= (q->items + temp)->weight;
    return q->items + temp;    
}

int put_queue(Queue *q, QueueNode new_node){
    if(q->size == MAX_QUEUE)return -1;
    q->items[q->tail] = new_node;
    q->tail = (q->tail + 1)%MAX_QUEUE;
    
    q->weight += new_node.weight;
    q->size++;
    return q->tail;
}

#endif