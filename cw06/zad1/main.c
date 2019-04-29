#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/types.h> 
#include <unistd.h>

void f();

int main(int argc, char **argv){
    printf("A\n");
    atexit(f);
    printf("C\n");
    return 0;
}

void f(){
    printf("B\n");
}