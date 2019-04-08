#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

char* fifo_path = "./fifo";
int fifo;
void child_function(){
    const char *some = "Some nice shit\n";
    //int s = open(fifo_path, O_WRONLY);
    if(fifo < 0)printf("child didn't open\n");
    write(fifo, some, 15);
    close(fifo);
}

int main(int argc, char **argv){
    fifo = open(fifo_path, O_RDWR);
    printf("fifo opened withour mkfifo\n");
    if(fifo < 0){
        if(-1 == mkfifo(fifo_path,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)){
            printf("mkfifo failed\n");
        }else{
            fifo = open(fifo_path, O_RDONLY);
            printf("mkfifo win\n");
        }
    }else{
        printf("fifo opened withour mkfifo\n");
    }
    
    char* str = calloc(40, 1);
        while(0 < read(fifo, str, 15)){
        printf("%s\n", str);
    }
    close(fifo);



    return 0;
}