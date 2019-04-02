#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int SIG_TO_SEND;
int CATCHER_PID;
int RECEIVED = 0;
int done = 0;
int CATCH_REC = 0;

void receive(int signum, siginfo_t *info, void* unused){
    if(signum == SIGUSR1 || signum == SIGRTMIN){
        RECEIVED++;
    }
    else if(signum == SIGUSR2 || signum == SIGRTMAX){
        CATCH_REC = info->si_value.sival_int;
        done = 1;
    }
}


void receiving(){
    struct sigaction act;
    act.sa_sigaction = receive;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGUSR1);
    sigdelset(&act.sa_mask, SIGUSR2);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    while(done == 0){
        pause();
    }
    printf("Otrzymano: %d\nWyslano: %d\n", RECEIVED, SIG_TO_SEND);
}

void with_kill(){
    for(int i=0; i<SIG_TO_SEND; i++){
        if(-1 == kill(CATCHER_PID, SIGUSR1)){
            printf("Kill error\n");
            perror(NULL);
            exit(0);
        }
    }
    kill(CATCHER_PID, SIGUSR2);
    receiving();
}

void with_sigqueue(){
    union sigval value;
    for(int i=0; i<SIG_TO_SEND; i++){
        if(-1 == sigqueue(CATCHER_PID, SIGUSR1,value)){
            printf("Sigqueue error\n");
            perror(NULL);
            exit(0);
        }
    }
    sigqueue(CATCHER_PID, SIGUSR2,value);
    receiving();
    printf("Catcher otrzymal: %d\n", CATCH_REC);
}

void receiving_RT(){
    struct sigaction act;
    act.sa_sigaction = receive;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGRTMIN);
    sigdelset(&act.sa_mask, SIGRTMAX);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN, &act, NULL);
    sigaction(SIGRTMAX, &act, NULL);
    while(done == 0){
        pause();
    }
    printf("Otrzymano: %d\nWyslano: %d\n", RECEIVED, SIG_TO_SEND);
}

void with_sigrt(){
    for(int i=0; i<SIG_TO_SEND; i++){
        if(-1 == kill(CATCHER_PID, SIGRTMIN)){
            printf("Kill error\n");
            perror(NULL);
            exit(0);
        }
    }
    kill(CATCHER_PID, SIGRTMAX);
    receiving_RT();
}

int main(int argc, char **argv){
    sigset_t new_mask;
    sigfillset(&new_mask);
    if(argc < 4){
        printf("Not enough arguments\n");
        return 1;
    }
    SIG_TO_SEND = atoi(argv[2]);
    CATCHER_PID = atoi(argv[1]);
    if(strcmp("KILL", argv[3]) == 0){
        //kill...
        sigdelset(&new_mask, SIGUSR1);
        sigdelset(&new_mask, SIGUSR2);
        sigprocmask(SIG_SETMASK, &new_mask, NULL);
        with_kill();
    }else if(strcmp("SIGQUEUE", argv[3]) == 0){
        //sigqueue...
        sigdelset(&new_mask, SIGUSR1);
        sigdelset(&new_mask, SIGUSR2);
        sigprocmask(SIG_SETMASK, &new_mask, NULL);
        with_sigqueue();
    }else if(strcmp("SIGRT", argv[3]) == 0){
        //sigrt...
        sigdelset(&new_mask, SIGRTMIN);
        sigdelset(&new_mask, SIGRTMAX);
        sigprocmask(SIG_SETMASK, &new_mask, NULL);
        with_sigrt();
    }
    return 0;
}