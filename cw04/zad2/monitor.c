#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <signal.h>

char* list_path;
char* additional_arg;
int parent_pid;
pid_t* child_pids;
char** child_paths;
int childs_running;
int files_opened = 0;
int end = 0;
FILE* file;

enum COMMAND{
    LIST = 0,
    STOP_PID = 1,
    STOP_ALL = 2,
    START_PID = 3,
    START_ALL = 4,
    END = 5, 
    UNDEF = -1
};

void rise_error(char* text){
    perror(text);
    exit(-1);
}

void rise_errno(){
    perror(NULL);
    exit(-1);
}

int close_monitors(){    
    for(int i=0; i<childs_running; i++){
        free(child_paths[i]);
    }
    free(child_paths);
    while(childs_running>0){
        int returned;
        int child_pid = (int)wait(&returned);
        if(WIFEXITED(returned)){
            if(WEXITSTATUS(returned) == -1)
                printf("Proces %d ukończył się z błędem\n", child_pid);
            else 
                printf("Proces %d utworzyl %d kopii pliku\n", child_pid, WEXITSTATUS(returned));
        }else{
            printf("WIERD END: %d %d\n", returned/256, child_pid);
        }
        childs_running--;
    }
    
    free(child_pids);
    return 0;
}

void end_all(int signum){
    for(int i=0; i<files_opened; i++){
        kill(child_pids[i], SIGINT);
    }
    close_monitors();
    fclose(file);
    exit(0);
}

enum COMMAND command_parser(char* scan){
    if(strcmp(scan, "LIST") == 0){
        //List...
        for(int i=0; i<files_opened; i++){
            printf("pid: %5d file: %s\n",child_pids[i],child_paths[i]);
        }
        return LIST;
    }else if(strcmp(scan, "STOP") == 0){
        char* tmp = calloc(4,1);
        scanf("%s", tmp);
        if(strcmp(tmp, "PID") == 0){
            int i;
            scanf("%d", &i);
            //stop pid...
            if(i>0)kill(i,SIGUSR1);
            return STOP_PID;
        }else if(strcmp(tmp, "ALL") == 0){
            //stop all...
            for(int i=0; i<files_opened; i++){
                kill(child_pids[i],SIGUSR1);
            }            
            return STOP_ALL;
        }
        free(tmp);
    }else if(strcmp(scan, "START") == 0){
        char* tmp = calloc(4,1);
        scanf("%s", tmp);
        if(strcmp(tmp, "PID") == 0){
            //start pid...
            int i;
            scanf("%d", &i);
            if(i>0)kill(i,SIGUSR2);
            return START_PID;
        }else if(strcmp(tmp, "ALL") == 0){
            //start all...
            for(int i=0; i<files_opened; i++){
                kill(child_pids[i],SIGUSR2);
            }
            return START_ALL;
        }
        free(tmp);
    }else if(strcmp(scan, "END") == 0){
        //end...
        end_all(0);        
        return END;
    }
    return UNDEF;
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
    char* buff = realpath(new_path, NULL);
    free(new_path);
    return buff;
}

void init_child_list(FILE *list){
    int files = 0;
    char monitored_file[200];
    int cycle_time;
    while(fscanf(list, "%s %d", monitored_file, &cycle_time) > 0){
        files++;
    }
    fseek(list,0,SEEK_SET);
    child_pids = calloc(files, sizeof(pid_t));
    child_paths = calloc(files, sizeof(char*));
    for( int i=0; i<files; i++){
        child_paths[i] = calloc(201, 1);
    }
}

int create_new_monitor(FILE *list){    
    init_child_list(list);
    char monitored_file[200];
    int cycle_time; 
    char cycle_time_str[10];
    int childs = 0;
    while((int)getpid() == parent_pid && fscanf(list, "%s %d", monitored_file, &cycle_time) > 0){
        char* file_path = create_path(monitored_file);
        sprintf(cycle_time_str, "%d", cycle_time);
        childs++;        
        pid_t child = fork();
        if(child == 0){
            if(-1 == execl("./monitor_buff", "./monitor_buff", file_path, cycle_time_str, NULL)){
                perror(NULL);
                exit(-1);
            }
        }else if(child == -1){
            printf("Fork failed\n");
        }else{
            child_pids[files_opened] = child;
            strcpy(child_paths[files_opened],file_path);
            files_opened++;
        }
        free(file_path);   
    }    
    return childs;
}

int main(int argc, char** argv){
    //argv[1] - list_path
    //argv[2] - work_time
    //argv[3] - tryb
    additional_arg = NULL;
    if(argc < 2)return -1;
    parent_pid = (int)getpid();
    list_path = argv[1];
    file = fopen(list_path, "r");    
    if(!file)rise_error("Can't open list file\n");
    childs_running = create_new_monitor(file);
    command_parser("LIST");
    signal(SIGINT, end_all);
    while(end == 0){
        char scan[5];
        printf("end = %d\n", end);
        scanf("%s", scan);
        if(command_parser(scan) == END)break;
    }
        
    //close_monitors();
    fclose(file);
    return 0;    
}