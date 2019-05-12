#include "line.h"

Queue *init_queue(){
    struct Queue q;
    q = 
}

QueueNode *pop_queue(Queue *q){
    if(q->size == 0)return NULL;
    int temp = q->head;
    return items + temp;    
}

int put_queue(Queue *q, QueueNode new_node){
    if(q->size == MAX_QUEUE)return -1;
    q->tail = (q->tail + 1)%MAX_QUEUE;
    q->items[q->tail] = new_node;
    return q->tail;
}