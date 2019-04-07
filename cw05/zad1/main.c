#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void rise_errno(){
    perror(NULL);
    exit(-1);
}

void rise_error(const char *mess){
    perror(mess);
    exit(2);
}

int argument_count(const char* line, const char* delim){
    char* buff = strdup(line);
    int i=0;
    char* word = strtok(buff, delim);    
    while(word){
        i++;
        word = strtok(NULL, delim);
    }
    return i;
}

int main(int argc, char **argv){
    const char *line = "cat file.txt | head -7 | tail -5";
    char *line_to_parse = strdup(line);
    //break line_to_parse into array of commands
    int commands_count = argument_count(line_to_parse, "|");
    char **commands = calloc(commands_count, sizeof(char*));
    if(commands <= 0)return 0;
    commands[0] = strtok(line_to_parse, "|");
    for(int i=1; i<commands_count; i++){
        commands[i] = strtok(NULL,"|");
    }
    //break each command and execute it
    int *fd = calloc(2*commands_count, sizeof(int));
    char *next_arg = NULL;
    for(int i=0; i<commands_count; i++){
        int args_count = argument_count(commands[i], " ");
        char **args = calloc(args_count+1, sizeof(char*));
        if(args_count <= 0)rise_error("Too short command\n");
        args[0] = strtok(commands[i], " ");
        for(int j=1; j<args_count; j++){
            args[j] = strtok(NULL, " ");
        }
        args[args_count] = NULL;
/*
mother 
       0. command 1v
                  0* 1. comm 3v  <-overwrite input and output
                             2* ... v  <-overwrite input and output
                                    .
                                    .
                                    .
                                    * ... (2cc - 1)v  <-overwrite input and output        
                                          (2cc - 2)* mother  <-reads


*/
        pipe(fd+i*2);
        pid_t child = fork();
        if(child == 0){
            if(i != 0){
                printf("in\n");
                dup2(fd[i*2-2],STDIN_FILENO);
            }
            if( i == commands_count-1){ 
                printf("close\n");
                close(fd[i*2]);
            }
            
                printf("out\n");
                dup2(fd[i*2+1],STDOUT_FILENO);
            
            if(-1 == execvp(args[0],args))rise_errno();
        }else{
            int r;
            printf("out\n");
            wait(&r);
            if(r!=0)rise_error("Process terminated with error\n");
            printf("out\n");//exit(0);
        }
        free(args);
        printf("LOOP\n");
    }

    char buff[300];
    int size = read(fd[2*commands_count - 2], buff, 300);
    if(next_arg)free(next_arg);
    next_arg = calloc(size+1, 1);
    strncpy(next_arg, buff, size);
    free(commands);
    if(next_arg){
        printf("\n\nResult\n\n");
        printf("%s\n",next_arg);
        free(next_arg);
    }
    
    return 0;
}