#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/types.h> 
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "line.h"

int BELT_CAP = 10;
int BELT_WGHT_LMT = 100;
int TRUCK_LMT = 200;

sem_t *sem_wght;
sem_t *sem_count;
sem_t *sem_global;
sem_t *sem_loader;
sem_t *sem_shutdown;
int shm_id;

int truck_wgth;

Queue *queue_ptr;
void init();
void exit_fun();
void load_truck();
void get_line_perm();
void unload_truck();
void print_stamp();
void sigint_h(int signum);

int main(int argc, char **argv){
    if(argc < 4){
        printf("Not enough arguments\n");
        exit(1);
    }
    BELT_CAP = atoi(argv[3]);
    TRUCK_LMT = atoi(argv[1]);
    BELT_WGHT_LMT = atoi(argv[2]);
    

    atexit(exit_fun);

    signal(SIGINT, sigint_h);

    init();
    while(1){
        sleep(3);
        get_line_perm();
    }
    truck_wgth = 0;
    exit(0);
}

void init(){
    sem_wght = sem_open(SEM_WGHT, O_CREAT, PERMISIONS, BELT_WGHT_LMT);
    sem_count = sem_open(SEM_COUNT, O_CREAT, PERMISIONS, BELT_CAP);
    sem_global = sem_open(SEM_GLOBAL, O_CREAT, PERMISIONS, 1);
    sem_loader = sem_open(SEM_LOADER, O_CREAT, PERMISIONS, 1);
    sem_shutdown = sem_open(SEM_SHUTDOWN, O_CREAT, PERMISIONS, 1);
    if(!sem_wght || !sem_count || !sem_global || !sem_loader || !sem_shutdown){
        printf("Semaphore not created %s", strerror(errno));
        exit(1);
    }

    shm_id = shm_open(SHM_PATH, O_RDWR | O_CREAT | O_TRUNC, PERMISIONS);
    if(ftruncate(shm_id, sizeof(Queue)) == -1){
        printf("Can't truncate shared memory: %s", strerror(errno));
        exit(1);
    }
    queue_ptr = mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE | PROT_EXEC, 
                     MAP_SHARED, shm_id, 0);
    if(queue_ptr == (void *)-1){
        printf("Shared memory not recived %s", strerror(errno));
        exit(1);
    }
    Queue *q = init_queue(BELT_WGHT_LMT, BELT_CAP);
    memcpy(queue_ptr, q, sizeof(Queue));
    delete_queue(q);    
}

void get_line_perm(){
    int waiting = 0;

    int value;
    sem_getvalue(sem_wght, &value);
    while(value == BELT_CAP || -1 == sem_trywait(sem_global)){
        if(waiting == 0){
            waiting = 1;            
            printf("Waiting for loading sem 3\n");
            print_stamp();
        }
    }
    printf("<<<<<<<<<Opend SEM\n");
    waiting = 0;
    load_truck();
    sem_post(sem_global);
    // if(semctl(sem_id, 0, GETVAL) == -1)exit(1);

    // if(semctl(sem_id, 1, GETVAL) == -1)exit(1);
    //printf("Belt wgth: %d\n Cpacity: %d\n", val, waiting);
    printf("<<<<<<<<<Closed SEM\n");
}

void load_truck(){
    if(queue_ptr->size > 0){
        QueueNode *pckg = queue_ptr->items+queue_ptr->head;
        if(pckg->weight + truck_wgth > TRUCK_LMT){            
            unload_truck();
        }
    
        pckg = pop_queue(queue_ptr);
        
        //printf("Pckg wght: %d\n",pckg->weight);
        truck_wgth += pckg->weight;
        for(int i=0; i<pckg->weight; i++)sem_post(sem_wght);
        sem_post(sem_count);
        time_t end_time;
        time(&end_time);
        double time_diff = difftime(end_time, pckg->start_time);
        printf("Delivered from: %ld\nAfter: %f s\n", pckg->loader_id, time_diff);    
        print_stamp();
    }
}

void unload_truck(){
    printf("Not more space in truck\nUnloading a truck\n");
    print_stamp();
    sleep(1); // czy inny random trwania rozladowania
    truck_wgth = 0;
    printf("Empty truck has arrived my Lord!\n");
    print_stamp();
}

void print_stamp(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    printf("%04d-%02d-%02d %02d:%02d:%02d.%06d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec);
}

void exit_fun(){    
    printf("Hej pa pa \n");
    sem_close(sem_wght);
    sem_close(sem_count);
    sem_close(sem_global);
    sem_close(sem_loader);
    sem_close(sem_shutdown);
    sem_unlink(SEM_WGHT);
    sem_unlink(SEM_COUNT);
    sem_unlink(SEM_GLOBAL);
    sem_unlink(SEM_LOADER);
    sem_unlink(SEM_SHUTDOWN);
    
    
    
    if(queue_ptr != NULL && queue_ptr != (void *)-1){
        munmap(queue_ptr, sizeof(Queue));
    }
    if(shm_id > 0)shm_unlink(SHM_PATH);
}

void sigint_h(int signum){
   // exit(0);
    sem_wait(sem_shutdown);
    //exit(0);
    int waiting = 0;
    while(-1 == sem_trywait(sem_global)){
        if(waiting == 0){
            waiting = 1;
            printf("Waiting for loading\n");
            print_stamp();
        }
    }
    waiting = 0;
    while(queue_ptr->size > 0){
        load_truck();
        sleep(3);
    }
    exit(0);
}