#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

int alive = 0;
pid_t child;

void print_timestamp(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1,
        tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void make_child(){
    child = fork();
    alive = 1;
    if(child == 0){
        if(-1 == execl("./date_script.sh","./date_script.sh",NULL)){
            printf("Nie udany execl\n");
            exit(-1);
        }
    }
    
}

void signalINT(int signum){
    if(alive)kill(child, SIGKILL);
    wait(NULL);
    exit(0);
}

void signalTSTP(int signum){    
    if(alive){
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
        kill(child, SIGKILL);
        wait(NULL);
        alive = 0;
    }else{
        printf("Kontynuacja\n");
        make_child();
    }        
}



int main(int argc, char **argv){
    signal(SIGTSTP, signalTSTP);
    signal(SIGINT, signalINT);
    make_child();
    while(1) {
        sleep(1);
    }
    return 0;
}