#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>

char* list_path;
int work_time;
int parent_pid;
int buff_monitor;
long max_mem;
long max_sec_cpu;

void rise_error(char* text){
    perror(text);
    exit(-1);
}

void rise_errno(){
    perror(NULL);
    exit(-1);
}

char* create_path(char* file_path){
    if(!file_path)return NULL;
    long list_length = strlen(list_path);
    int i = list_length-1;
    for(; i>=0; i--){
        if(list_path[i] == '/'){
            break;
        }
    }
    i++;
    char* new_path = calloc(strlen(file_path) + 1 + i, 1);
    if(!new_path)rise_error("Can't allocate memory\n");
    strncat(new_path, list_path, i);
    strcat(new_path, file_path);
    return new_path;
}

int create_new_monitor(FILE *list){    
    char monitored_file[200];
    int cycle_time; 
    char cycle_time_str[10];
    char work_time_str[10];
    int files = 0;
    sprintf(work_time_str,"%d", work_time);
    while((int)getpid() == parent_pid && fscanf(list, "%s %d", monitored_file, &cycle_time) > 0){
        char* file_path = create_path(monitored_file);
        sprintf(cycle_time_str, "%d", cycle_time);
        files++;        
        int child = (int)fork();
        if(child == 0){
            struct rlimit* cur_limit = malloc(sizeof(struct rlimit));
            if(!cur_limit)rise_error("Can't allocate memory\n");
            cur_limit->rlim_cur = max_sec_cpu;
            cur_limit->rlim_max = max_sec_cpu;
            if(-1 == setrlimit(RLIMIT_CPU,cur_limit))rise_errno();
            cur_limit->rlim_cur = max_mem;
            cur_limit->rlim_max = max_mem;
            if(-1 == setrlimit(RLIMIT_AS,cur_limit))rise_errno();
            free(cur_limit);
            if(buff_monitor == 1){
                if(-1 == execl("./monitor_buff", "./monitor_buff", file_path, work_time_str, cycle_time_str, NULL)){
                    perror(NULL);
                    exit(-1);
                }
            }else{
                if(-1 == execl("./monitor_exec", "./monitor_exec", file_path, work_time_str, cycle_time_str, NULL)){
                    perror(NULL);
                    exit(-1);
                }
            }

        }else if(child == -1){
            printf("Fork failed\n");
        }
        free(file_path);   
    }    
    return files;
}

int close_monitors(int files_monitored){
    
    while(files_monitored>0){
        struct rusage start_usage;
        if (getrusage(RUSAGE_CHILDREN, &start_usage)) rise_errno();
        int returned;
        int child_pid = (int)wait(&returned);

        if(WIFEXITED(returned)){
            if(WEXITSTATUS(returned) == -1)
                printf("Proces %d ukończył się z błędem\n", child_pid);
            else 
                printf("Proces %d utworzyl %d kopii pliku\n", child_pid, WEXITSTATUS(returned));
        }else{
            printf("Proces %d został przerwany, przez brak pamięci lub przekroczenie czasu procesora\n", child_pid);
        }
        struct rusage end_usage;
        if (getrusage(RUSAGE_CHILDREN, &end_usage)) rise_errno();
        int us = end_usage.ru_utime.tv_sec - start_usage.ru_utime.tv_sec;
        int uu = end_usage.ru_utime.tv_usec - start_usage.ru_utime.tv_usec;
        int ss = end_usage.ru_stime.tv_sec - start_usage.ru_stime.tv_sec;
        int su = end_usage.ru_stime.tv_usec - start_usage.ru_stime.tv_usec;
        if(su < 0){ss--; su += 1000000;}
        if(uu < 0){us--; uu += 1000000;}
        printf("PID: %d  CPU_USER: %d.%06d s  CPU_SYSTEM: %d.%06d s\n",
                child_pid, us,uu,ss,su);
        files_monitored--;
    }
    return 0;
}

int main(int argc, char** argv){
    //argv[1] - list_path
    //argv[2] - work_time
    //argv[3] - tryb
    if(argc < 6)return -1;
    parent_pid = (int)getpid();
    list_path = argv[1];
    work_time = atoi(argv[2]);
    buff_monitor = atoi(argv[3]);
    max_sec_cpu = atoi(argv[4]);
    max_mem = atoi(argv[5])*1048576; //B to MB
    FILE* file = fopen(list_path, "r");    
    if(!file)rise_error("Can't open list file\n");
    int files_monitored = create_new_monitor(file);
    close_monitors(files_monitored);
    fclose(file);
    return 0;    
}