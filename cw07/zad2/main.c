#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <wait.h>

pid_t *loaders;
int number_of_loaders;

void sigint_h(int signum){
    for(int i=0; i<number_of_loaders; i++){
        kill(SIGINT, loaders[i]);
    }
    exit(0);
}

void exit_fun(){
    for(int i=0; i<number_of_loaders; i++){
        wait(NULL);
    }
    free(loaders);
}

int main(int argc, char **argv) {
    
    if (argc < 2){
        printf("Not enough arguments\n");
        exit(1);
    }
    atexit(exit_fun);
    signal(SIGINT, sigint_h);
    char *number_of_cycles = "-1";
    char *max_wght;
    if(argc >= 2){
        number_of_loaders = atoi(argv[1]);
        if(argc > 2){
            number_of_cycles = argv[2];
        }
        if(argc > 3){
            max_wght = argv[3];
        }
    }

    loaders = calloc(number_of_loaders, sizeof(pid_t));
    for(int i=0; i<number_of_loaders; i++){
        pid_t loader = fork();
        if(loader == 0){
            execl("./loader", "loader", number_of_cycles, max_wght, (char *) NULL);
            return 0;
        }else{
            printf("New loader created, PID: %d\n", loader);
            loaders[i] = loader;
        }
    }

    while(loaders)pause();
    exit(0);

}