#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

int stopped = 0;

void print_timestamp(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1,
        tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void au2(int signum){
    
    exit(0);
}

void au(int signum){
    
    if(!stopped){
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
        stopped = 1;
    }else{
        printf("Kontynuacja\n");
        stopped = 0;
    }        
}

int main(int argc, char **argv){
    signal(SIGTSTP, au);
    signal(SIGINT, au2);
        
    while(1) {
        if(!stopped)print_timestamp();
        else pause();
        sleep(1);
    }
    return 0;
}