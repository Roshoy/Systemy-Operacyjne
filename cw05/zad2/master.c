#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void rise_error(const char* mess){
    perror(mess);
    exit(1);
}

void rise_errno(){
    perror(NULL);
    exit(1);
}

int main(int argc, char **argv){
    if(argc < 2)rise_error("Too few arguments! \n Arguments: pipe_name\n");
    char* fifo_path = argv[1];
    int fifo = open(fifo_path, O_RDWR);
    if(fifo < 0){
        if(-1 == mkfifo(fifo_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)){
        }else{
            fifo = open(fifo_path, O_RDONLY);
            if(fifo < 0)rise_errno();
        }
    }    
    char* str = calloc(59, 1);
    while(0 < read(fifo, str, 58)){
        printf("%s\n", str);
    }
    close(fifo);
    return 0;
}