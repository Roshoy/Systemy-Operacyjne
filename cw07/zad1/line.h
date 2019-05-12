#ifndef BELT_H
#define BELT_H

#define WGTH_LMT_SEED 1
#define CAP_SEED 2
#define SHM_SEED 3
#define MAX_QUEUE 1024

struct QueueNode{
    int weight;
    time_t start_time;
    pid_t loader_id;
    int next;
}PckgInfo;

struct Queue{
    int head;
    int tail; // next of the last node
    QueueNode items[MAX_QUEUE];
    int wgth_lmt;
    int capacity;
    int size;
    int weight;
}Queue;

QueueNode *pop_queue(Queue *q);
int put_queue(Queue *q, QueueNode );


#endif BELT_H