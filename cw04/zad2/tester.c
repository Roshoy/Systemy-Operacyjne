#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void rise_error(char* text){
    perror(text);
    exit(-1);
}

void rise_errno(){
    perror(NULL);
    exit(-1);
}

int append(FILE *file, const int bytes, const int sec){
    char* random = calloc(bytes+1, 1);
    if(!random)rise_error("Can't allocate memory\n");
    for(int i=0; i<bytes; i++){
        random[i] = (char)(rand()%26 + 65);
    }
    time_t t = time(NULL);
    struct tm times = *localtime(&t);

    fprintf(file, "%d %d %04d-%02d-%02d %02d:%02d:%02d %s\n", getpid(), sec, times.tm_year + 1900, times.tm_mon + 1,
        times.tm_mday, times.tm_hour, times.tm_min, times.tm_sec, random);
    free(random);
    return 0;
}

int main(int argc, char **argv){
    srand(time(NULL));
    char* path = argv[1];
    
    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    int bytes = atoi(argv[4]);

    do{          
        //////////////here it works
        FILE *file = fopen(path, "a+");
        if(!file)rise_error("Can't open file\n");
        int sec = rand()%(pmax - pmin) + pmin;
        append(file, bytes, sec);
        fclose(file);
        //////////////here it ends work       
        sleep(sec);
    }while(1);
    return 0;
}