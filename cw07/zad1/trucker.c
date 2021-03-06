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
#include "line.h"

int BELT_CAP = 10;
int BELT_WGHT_LMT = 100;
int TRUCK_LMT = 200;

int truck_wgth;
int sem_id;
int shm_id;
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
    TRUCK_LMT = atoi(argv[1]);
    BELT_WGHT_LMT = atoi(argv[2]);
    BELT_CAP = atoi(argv[3]);
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
    key_t sem_key = ftok(getenv("HOME"), SEM_SEED);

    printf("FTOK: %d\n", sem_key);
    key_t shm_key = ftok(getenv("HOME"), SHM_SEED);
    printf("FTOK: %d\n", shm_key);
    sem_id = semget(sem_key, 4, IPC_CREAT | PERMISIONS);
    if(sem_id == -1){
        printf("Semaphore not created %s", strerror(errno));
        exit(1);
    }
    shm_id = shmget(shm_key, sizeof(Queue), IPC_CREAT | PERMISIONS );
    if(shm_id == -1){
        printf("Shared memory not created %s", strerror(errno));
        pause();
        exit(1);
    }
    printf("Starting values: %d %d\n", BELT_WGHT_LMT, BELT_CAP);
    int value = BELT_WGHT_LMT;
    if(-1 == semctl(sem_id, 0, SETVAL, BELT_WGHT_LMT)){
        printf("Sem 0 init failed: %s\n", strerror(errno));
    };
    value = BELT_CAP;
    if(-1 == semctl(sem_id, 1, SETVAL, BELT_CAP)){
        printf("Sem 1 init failed: %s\n", strerror(errno));
    };
    value = 1;
    if(-1 == semctl(sem_id, 2, SETVAL, 1)){
        printf("Sem 2 init failed: %s\n", strerror(errno));
    };
    if(-1 == semctl(sem_id, 3, SETVAL, 1)){
        printf("Sem 3 init failed: %s\n", strerror(errno));
    };
    //////debugging
    int waiting;
    printf("Belt wgth: %d\n Cpacity: %d\n", semctl(sem_id, 0, GETVAL), semctl(sem_id, 1, GETVAL));
    //////debugging

    if((queue_ptr = (Queue *)shmat(shm_id, NULL, 0)) == (void *)-1){
        printf("Shared memory not recived %s", strerror(errno));
        exit(1);
    }
    Queue *q = init_queue(BELT_WGHT_LMT, BELT_CAP);
    memcpy(queue_ptr, q, sizeof(Queue));
    delete_queue(q);    
}

void get_line_perm(){
    int waiting = 0;
    struct sembuf buff[1];
    buff[0].sem_num = 2;
    buff[0].sem_op = -1;
    buff[0].sem_flg = IPC_NOWAIT;

    int value;
   // printf("Belt wgth: %d\n Cpacity: %d\n", semctl(sem_id, 2, GETVAL, value), semctl(sem_id, 1, GETVAL, value));
    while(semctl(sem_id, 1, GETVAL, value) == BELT_CAP || -1 == semop(sem_id, buff, 1)){
        if(waiting == 0){
            waiting = 1;
            
            printf("Waiting for loading sem 3\n");
            print_stamp();
        }
    }
    printf("<<<<<<<<<Opend SEM\n");
    waiting = 0;
    load_truck();
    
    buff[0].sem_num = 2;
    buff[0].sem_op = 1;
    semop(sem_id, buff, 1);
    //printf("Belt wgth: %d\n Cpacity: %d\n", val, waiting);
    printf("<<<<<<<<<Closed SEM\n");
}

void load_truck(){
    if(queue_ptr->size > 0){
        
        struct sembuf buff[2];
        QueueNode *pckg = queue_ptr->items+queue_ptr->head;
        if(pckg->weight + truck_wgth > TRUCK_LMT){            
            unload_truck();
        }
    
        pckg = pop_queue(queue_ptr);
        
        //printf("Pckg wght: %d\n",pckg->weight);
        truck_wgth += pckg->weight;
        buff[0].sem_num = 0;
        buff[0].sem_op = pckg->weight;
        buff[0].sem_flg = 0;
        buff[1].sem_num = 1;
        buff[1].sem_op = 1;
        buff[1].sem_flg = 0;
        //printf("HEHE\n");
        int value;
        //printf("Belt wgth: %d\n Cpacity: %d\n", semctl(sem_id, 0, GETVAL), semctl(sem_id, 1, GETVAL));
        semop(sem_id, buff, 2);
        //printf("HEHE\n");
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
    if(sem_id > 0)semctl(sem_id, 0, IPC_RMID);
    if(queue_ptr != NULL && queue_ptr != (void *)-1){
        shmdt(queue_ptr);
    }
    if(shm_id > 0)shmctl(shm_id, IPC_RMID, NULL);
}

void sigint_h(int signum){
    //exit(0);
    int waiting = 0;
    struct sembuf buff[2];
    buff[0].sem_num = 2;
    buff[0].sem_op = -1;
    buff[0].sem_flg = IPC_NOWAIT;
    while(-1 == semop(sem_id, buff,1)){
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