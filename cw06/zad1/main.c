#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/types.h> 
#include <unistd.h>

void f();

int main(int argc, char **argv){
    char *s = "23 34 65";
    long int x;
    char *pEnd;
    x = strtol(s, &pEnd, 10);
    printf("%ld\n", x);
    while((x = strtol(pEnd, &pEnd, 10)) != 0L)printf("%ld\n", x);
    return 0;
}

void f(){
    printf("B\n");
}