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
    if(argc < 2)rise_error("Not enough arguments: needed command list file\n");
    char *line = NULL;
    size_t line_size = 0;
    FILE* commands_files = fopen(argv[1],"r");
    if(!commands_files)rise_error("Couldn't open file\n");
    while(-1 != getline(&line, &line_size, commands_files)){
        if(line)printf("%s\n",line);
        else rise_errno();
        line_size = strlen(line);
        char *line_to_parse = calloc(line_size, 1);
        if(line[line_size-1] == '\n')line_size--;
        strncat(line_to_parse,line,line_size);
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
                    dup2(fd[i*2-2],STDIN_FILENO);
                }
                if( i == commands_count-1){ 
                    close(fd[i*2]);
                }            
                    dup2(fd[i*2+1],STDOUT_FILENO);
                
                if(-1 == execvp(args[0],args))rise_errno();
            }else{
                if(i != 0){                
                    close(fd[i*2-2]);
                }                
                    close(fd[i*2+1]);            
                int r;
                wait(&r);
                if(r!=0)rise_error("Process terminated with error\n");
            }
            free(args);
        }

        char buff[300];
        int size = read(fd[2*commands_count - 2], buff, 300);
        next_arg = calloc(size+1, 1);
        strncpy(next_arg, buff, size);
        free(commands);
        if(next_arg){
            printf("\n\nResult\n\n");
            printf("%s\n",next_arg);
            free(next_arg);
        }
        
        free(line);
        line = NULL;
    }
    if(line){
        free(line);
    }
    return 0;
}