#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

void rise_error(const char* mess){
    perror(mess);
    exit(1);
}

void rise_errno(){
    perror(NULL);
    exit(1);
}

int main(int argc, char **argv){
    if(argc < 3)rise_error("Too few arguments! \n Arguments: pipe_name data_send\n");
    char* fifo_path = argv[1];
    int N = atoi(argv[2]);
    srand(time(NULL)-getpid());
    printf("PID: %d\n", getpid());
    int fifo = open(fifo_path, O_RDWR);
    if(fifo < 0){
        if(-1 == mkfifo(fifo_path,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)){
            rise_errno();
        }else{
            fifo = open(fifo_path, O_RDONLY);
            if(fifo < 0)rise_errno();
        }
    }
    char *buff = calloc(40,1);
    while(N>0){
        FILE* p_date = popen("date", "r");
        if(!p_date)rise_errno();
        fread(buff, 40, 40, p_date);
        pclose(p_date);
        strtok(buff, "\n");
        int sec = rand()%4+2;
        dprintf(fifo, "PID:%7d Date: %-40s", getpid(), buff);
        sleep(sec);
        N--; 
    }
    free(buff);
    close(fifo);
    return 0;
}