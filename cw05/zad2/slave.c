#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

char* fifo_path = "./fifo";
int fifo;

int main(int argc, char **argv){
    int N = 10;
    printf("PID: %d\n", getpid());
    fifo = open(fifo_path, O_RDWR);
    if(fifo < 0){
        if(-1 == mkfifo(fifo_path,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)){
            printf("mkfifo failed\n");
        }else{
            fifo = open(fifo_path, O_RDONLY);
            printf("mkfifo win\n");
        }
    }else{
        printf("fifo opened without mkfifo\n");
    }
    
    char* str = calloc(40, 1);
    while(N>0){
        FILE* p_date = popen("date", "r");
        char *buff = calloc(40,1);
        read(p_date, buff, 40);
        close(p_date);
        dprintf(fifo, "PID: %7d Date: %s\n", getpid(), buff);
        N--;
    }
    while(0 < read(fifo, str, 15)){
        printf("%s\n", str);
    }
    close(fifo);



    return 0;
}