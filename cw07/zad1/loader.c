#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/types.h> 
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "line.h"

int sem_id;
int shm_id;
void *queue_ptr;
int N = 10;
int C = 10;
void init();
void exit_fun();
void load_truck();
void print_stamp();

void sigint_h(int signum);

int main(int argc, char **argv){
    if(argc < 3){
        printf("Not enough arguments\n");
        exit(1);
    }
    C = atoi(argv[1]);
    N = atoi(argv[2]);
    srand(time(NULL));
    atexit(exit_fun);
    init();
    for(int i=0; C == -1 || i<C; i++){
        sleep(rand()%3); //random
        load_truck();
    }
    exit(0);
}

void init(){
    key_t sem_key = ftok(getenv("HOME"), SEM_SEED);
    printf("FTOK: %d\n", sem_key);
    key_t shm_key = ftok(getenv("HOME"), SHM_SEED);
    sem_id = semget(sem_key, 4, PERMISIONS);
    if(sem_id == -1){
        printf("Semaphore not created %s\n", strerror(errno));
        exit(1);
    }
    shm_id = shmget(shm_key, sizeof(Queue), PERMISIONS);
    if(shm_id == -1){
        printf("Shared memory not opened %s\n", strerror(errno));
        exit(1);
    }

    if((queue_ptr = shmat(shm_id, NULL, 0)) == (void *)-1){
        printf("Shared memory not pointer getted %s\n", strerror(errno));
        exit(1);
    }
}

void load_truck(){
    int waiting = 0;
    QueueNode pckg;
    pckg.weight = rand()%N + 1;
    pckg.loader_id = getpid();
    time(&pckg.start_time);
    
    struct sembuf buff[3];
    buff[0].sem_num = 3;
    buff[0].sem_op = -1;
    buff[0].sem_flg = IPC_NOWAIT;
    int val;
    while(-1 == semop(sem_id, buff,1)){
        if(semctl(sem_id, 3, GETVAL, val) == -1)exit(1);
        if(waiting == 0){
            printf("Waiting for sem 4\n");
            print_stamp();
            waiting = 1;
        }
    } // zaglodzenia ogarniane przez randomowy dostep
    // wymuszajacy na jednym ziomku wstawienie paczki kiedy dostep już uzyskał


    buff[0].sem_num = 0;
    buff[0].sem_op = -pckg.weight; // random val, masa paczki
    buff[0].sem_flg = IPC_NOWAIT;
    buff[1].sem_num = 1;
    buff[1].sem_op = -1;
    buff[1].sem_flg = IPC_NOWAIT;
    buff[2].sem_num = 2;
    buff[2].sem_op = -1;
    buff[2].sem_flg = IPC_NOWAIT;
    
    while(-1 == semop(sem_id, buff, 3)){
        int val2;
        if(semctl(sem_id, 3,GETVAL) == -1)exit(1);
        if(waiting == 0){
            printf("Waiting for line\n");
            print_stamp();
            waiting = 1;
        }
    }

    printf("<<<<<<<<<Opend SEM\n");
    waiting = 0;
    put_queue(queue_ptr, pckg);
    printf("Package sent\n");
    print_stamp();
    buff[0].sem_num = 2;
    buff[0].sem_op = 1;
    buff[1].sem_num = 3;
    buff[1].sem_op = 1;
    semop(sem_id, buff, 2);

    printf("<<<<<<<<<Closed SEM\n");
    
}

void print_stamp(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    printf(" >%04d-%02d-%02d %02d:%02d:%02d.%06d\n >Loader PID: %d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, getpid());
}

void exit_fun(){
    
    print_stamp();
    if(queue_ptr != NULL && queue_ptr != (void *)-1) shmdt(queue_ptr);
    printf("Closing loader\n");
}