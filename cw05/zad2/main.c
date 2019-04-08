#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int main(){
    int proc = 20;
    pid_t master = fork();
    if(master == 0){
        if(-1 == execl("./master", "./master", "./fifo",NULL)){
            printf("zjebales\n");
            exit(1);
        }
    }
    for(int i=0; i<proc; i++){
        pid_t slave = fork();
        if(slave == 0){
            if(-1 == execl("./slave", "./slave", "./fifo", "10", NULL)){
                printf("zjebales slave\n");
                exit(1);
            }
        }
    }
    for(int i=0; i<proc; i++){
        wait(NULL);
    }
    sleep(1);
    kill(master, SIGKILL);
    wait(NULL);

    return 0;
}