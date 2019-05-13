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

sem_t *sem_wght;
sem_t *sem_count;
sem_t *sem_global;
sem_t *sem_loader;
sem_t *sem_shutdown;
int shm_id;
void *queue_ptr;
int N = 10;
int C = 10;
void init();
void exit_fun();
int load_truck();
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
        if(load_truck() == 0)i--;
    }
    exit(0);
}

void init(){
    sem_wght = sem_open(SEM_WGHT, 0);
    sem_count = sem_open(SEM_COUNT, 0);
    sem_global = sem_open(SEM_GLOBAL, 0);
    sem_loader = sem_open(SEM_LOADER, 0);
    sem_shutdown = sem_open(SEM_SHUTDOWN, 0);
    if(!sem_wght || !sem_count || !sem_global || !sem_loader || !sem_shutdown){
        printf("Semaphore not created %s", strerror(errno));
        exit(1);
    }
    shm_id = shm_open(SHM_PATH, O_RDWR, PERMISIONS);
    if(shm_id == -1){
        printf("Shared memory not opened %s\n", strerror(errno));
        exit(1);
    }
    queue_ptr = mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE | PROT_EXEC, 
                     MAP_SHARED, shm_id, 0);
    if(queue_ptr == (void *)-1){
        printf("Shared memory not pointer getted %s\n", strerror(errno));
        exit(1);
    }
}

int load_truck(){
    int pckg_send = 0;
    int waiting = 0;
    QueueNode pckg;
    pckg.weight = rand()%N + 1;
    pckg.loader_id = getpid();
    time(&pckg.start_time);
    while(-1 == sem_trywait(sem_loader)){
        int sd = 1;
        sem_getvalue(sem_shutdown, &sd);
        if(sd == 0)exit(1);
        if(waiting == 0){
            printf("Waiting for sem 4\n");
            print_stamp();
            waiting = 1;
        }
    } // zaglodzenia ogarniane przez randomowy dostep
    // wymuszajacy na jednym ziomku wstawienie paczki kiedy dostep już uzyskał
    
    while(-1 == sem_trywait(sem_global)){
        int sd = 1;
        sem_getvalue(sem_shutdown, &sd);
        if(sd == 0)exit(1);
        if(waiting == 0){
            printf("Waiting for line\n");
            print_stamp();
            waiting = 1;
        }
    }
    
    int belt_wght;
    int belt_count;
    sem_getvalue(sem_wght, &belt_wght);
    sem_getvalue(sem_count, &belt_count);
    if(belt_wght >= pckg.weight && belt_count > 0){        
        //printf("<<<<<<<<<Opend SEM\n");
        for(int i=0; i<pckg.weight; i++)sem_wait(sem_wght);
        sem_wait(sem_count);
        waiting = 0;
        put_queue(queue_ptr, pckg);
        printf("Package sent\n");
        print_stamp();
        pckg_send = 1;
    }       
    sem_post(sem_global);
    sem_post(sem_loader);
    //printf("<<<<<<<<<Closed SEM\n");
    return pckg_send;
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
    sem_close(sem_wght);
    sem_close(sem_count);
    sem_close(sem_global);
    sem_close(sem_loader);
    sem_close(sem_shutdown);
    print_stamp();
    if(queue_ptr != NULL && queue_ptr != (void *)-1) munmap(queue_ptr, sizeof(Queue));
    printf("Closing loader\n");
}