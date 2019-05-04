#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/types.h> 
#include <unistd.h>

void f();

int main(int argc, char **argv){
    int d;
    int s = scanf("%d",&d);
    printf("\n%d\n%d\n", d, s);
    return 0;
}

void f(){
    printf("B\n");
}