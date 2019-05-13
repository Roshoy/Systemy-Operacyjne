#include "line.h"

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
    q->size--;
    return items + temp;    
}

int put_queue(Queue *q, QueueNode new_node){
    if(q->size == MAX_QUEUE)return -1;
    q->tail = (q->tail + 1)%MAX_QUEUE;
    q->items[q->tail] = new_node;
    q->size++;
    return q->tail;
}