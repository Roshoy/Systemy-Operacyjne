#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int SIG_TO_SEND;
int CATCHER_PID;
int RECEIVED = 0;
int CATCH_REC = 0;
void receive_kill(int signum, siginfo_t *info, void* unused){
    if(signum == SIGUSR1 || signum == SIGRTMIN){                
        RECEIVED++;
        if(RECEIVED < SIG_TO_SEND)kill(CATCHER_PID, SIGUSR1);
        else kill(CATCHER_PID, SIGUSR2);
    }
    else if(signum == SIGUSR2 || signum == SIGRTMAX){
        printf("Otrzymano: %d\nWyslano: %d\n", RECEIVED, SIG_TO_SEND);
        exit(0);
    }
}

void with_kill(){
    struct sigaction act;
    act.sa_sigaction = receive_kill;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGUSR1);
    sigdelset(&act.sa_mask, SIGUSR2);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    kill(CATCHER_PID,SIGUSR1);
    while(1);//printf("SIG_REC = %d\n",RECEIVED);
    
}

void receive_sigqueue(int signum, siginfo_t *info, void* unused){
    union sigval value;
    if(signum == SIGUSR1){                
        RECEIVED++;
        if(RECEIVED < SIG_TO_SEND)sigqueue(CATCHER_PID, SIGUSR1,value);
        else sigqueue(CATCHER_PID, SIGUSR2,value);
    }
    else if(signum == SIGUSR2){
        CATCH_REC = info->si_value.sival_int;
        printf("Otrzymano: %d\nWyslano: %d\n", RECEIVED, SIG_TO_SEND);
        printf("Catcher otrzymal: %d\n", CATCH_REC);
        exit(0);
    }    
}

void with_sigqueue(){
    struct sigaction act;
    act.sa_sigaction = receive_sigqueue;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGUSR1);
    sigdelset(&act.sa_mask, SIGUSR2);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    union sigval value;
    sigqueue(CATCHER_PID, SIGUSR1,value);
    while(1);
    printf("Otrzymano: %d\nWyslano: %d\n", RECEIVED, SIG_TO_SEND);
    printf("Catcher otrzymal: %d\n", CATCH_REC);
}

void receive_rt(int signum, siginfo_t *info, void* unused){
    if(signum == SIGRTMIN){                
        RECEIVED++;
        if(RECEIVED < SIG_TO_SEND)kill(CATCHER_PID, SIGRTMIN);
        else kill(CATCHER_PID, SIGRTMAX);
    }
    else if(signum == SIGRTMAX){
        printf("Otrzymano: %d\nWyslano: %d\n", RECEIVED, SIG_TO_SEND);
        exit(0);
    }
}

void with_sigrt(){
    struct sigaction act;
    act.sa_sigaction = receive_rt;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGRTMIN);
    sigdelset(&act.sa_mask, SIGRTMAX);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN, &act, NULL);
    sigaction(SIGRTMAX, &act, NULL);
    kill(CATCHER_PID, SIGRTMIN);
    while(1);
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