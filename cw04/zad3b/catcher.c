#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int SIG_TO_SEND;
int SENDER_PID = 0;
int RECEIVED = 0;

void receive_kill(int signum, siginfo_t *info, void* unused){
    SENDER_PID = info->si_pid;
    if(signum == SIGUSR1 || signum == SIGRTMIN){
        RECEIVED++;
        kill(SENDER_PID, SIGUSR1);        
    }
    else if(signum == SIGUSR2 || signum == SIGRTMAX){
        kill(SENDER_PID, signum);
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

    while(1);
}

void receive_sigqueue(int signum, siginfo_t *info, void* unused){
    SENDER_PID = info->si_pid;
    union sigval value;
    if(signum == SIGUSR1 || signum == SIGRTMIN){
        RECEIVED++;
        value.sival_int = RECEIVED;
        sigqueue(SENDER_PID, SIGUSR1,value);
    }
    else if(signum == SIGUSR2 || signum == SIGRTMAX){
        value.sival_int = RECEIVED;
        sigqueue(SENDER_PID, SIGUSR2,value);
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
    while(1);    
}

void receive_rt(int signum, siginfo_t *info, void* unused){
    SENDER_PID = info->si_pid;
    if(signum == SIGRTMIN){
        RECEIVED++;
        kill(SENDER_PID, SIGRTMIN);
    }
    else if(signum == SIGRTMAX){
        kill(SENDER_PID, SIGRTMAX);
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
    while(1);
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