#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int SIG_TO_SEND;
int SENDER_PID;
int RECEIVED = 0;
int done = 0;

void receive(int signum, siginfo_t *info, void* unused){
    if(signum == SIGUSR1 || signum == SIGRTMIN)RECEIVED++;
    else if(signum == SIGUSR2 || signum == SIGRTMAX){
        SENDER_PID = info->si_pid;
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
    SIG_TO_SEND = RECEIVED;
}

void with_kill(){    
    receiving();
    for(int i=0; i<SIG_TO_SEND; i++){
        kill(SENDER_PID, SIGUSR1);
    }
    kill(SENDER_PID, SIGUSR2);    
    
}

void with_sigqueue(){
    receiving();
    union sigval value;
    for(int i=0; i<SIG_TO_SEND; i++){
        value.sival_int = i;
        if(-1 == sigqueue(SENDER_PID, SIGUSR1,value)){
            printf("Sigqueue error\n");
            perror(NULL);
            exit(0);
        }
    }
    value.sival_int = SIG_TO_SEND;
    sigqueue(SENDER_PID, SIGUSR2,value);
    
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
    SIG_TO_SEND = RECEIVED;
}

void with_sigrt(){   
    receiving_RT();
    for(int i=0; i<SIG_TO_SEND; i++){
        kill(SENDER_PID, SIGRTMIN);
    }
    kill(SENDER_PID, SIGRTMAX);        
}

int main(int argc, char **argv){
    printf("Catcher pid: %d\n",getpid());
    sigset_t new_mask;
    sigfillset(&new_mask);
    if(argc < 2){
        printf("Not enough arguments\n");
        return 1;
    }

    if(strcmp("KILL", argv[1]) == 0){
        //kill...
        sigdelset(&new_mask, SIGUSR1);
        sigdelset(&new_mask, SIGUSR2);
        sigprocmask(SIG_SETMASK, &new_mask, NULL);
        with_kill();
    }else if(strcmp("SIGQUEUE", argv[1]) == 0){
        //sigqueue...
        sigdelset(&new_mask, SIGUSR1);
        sigdelset(&new_mask, SIGUSR2);
        sigprocmask(SIG_SETMASK, &new_mask, NULL);
        with_sigqueue();
    }else if(strcmp("SIGRT", argv[1]) == 0){
        //sigrt...
        sigdelset(&new_mask, SIGRTMIN);
        sigdelset(&new_mask, SIGRTMAX);
        sigprocmask(SIG_SETMASK, &new_mask, NULL);
        with_sigrt();
    }
    return 0;
}