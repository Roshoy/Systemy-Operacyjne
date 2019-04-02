#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <libgen.h>

time_t last_modification;
char* last_content_buffer;
int changes;
char* path;
char* archive_path;
int run = 1;

void rise_error(char* text){
    perror(text);
    exit(-1);
}

void rise_errno(){
    perror(NULL);
    exit(-1);
}

void signal_usr1(int signum){
    if(signum == SIGUSR1) {
        run = 0;
        printf("PID: %d is stopped\n", (int)getpid());
    }
    else if(signum == SIGUSR2){
        run = 1;
        printf("PID: %d is started\n", (int)getpid());
    }
    else if(signum == SIGINT){
        run = 2;
        printf("PID: %d is ended\n", (int)getpid());
    }
}

int update_content_buffer(){
    FILE* file = fopen(path,"r");
    if(!file)rise_error("Can't open file\n");
    if(0 != fseek(file, 0, SEEK_END))rise_error("Can't navigate in file\n");
    long length = ftell(file);
    if(0 != fseek(file, 0, SEEK_SET))rise_error("Can't navigate in file\n");
    free(last_content_buffer);
    last_content_buffer = calloc(length+1, sizeof(char));
    if(EOF == fread(last_content_buffer, sizeof(char), length, file))
        rise_error("Can't read file\n");
    if(EOF == fclose(file))rise_error("Can't read file\n");
    return 0;
}

int update_mod_date(){
    struct stat file_stat;
    if(-1 == lstat(path, &file_stat))rise_errno();
    last_modification = file_stat.st_mtime;
    return 0;
}

char* time_to_str(const time_t* time){
    char* buff = calloc(21, 1);
    if(!buff)rise_error("Can't allocate memory\n");
    if(0 == strftime(buff, 21, "_%Y-%m-%d_%H-%M-%S", localtime(time))){
        printf("Error while converting time to string!\n");
    }
    return buff;
}

time_t string_to_time(char* time_arg){
    struct tm tms = {0};
    char* time_format = "%d-%d-%d %d:%d:%d";
    if(EOF == sscanf(time_arg, time_format, &tms.tm_year, &tms.tm_mon, &tms.tm_mday,
        &tms.tm_hour, &tms.tm_min, &tms.tm_sec))
        printf("Error while parsing time to string\n");    
    tms.tm_mon--;
    tms.tm_year -= 1900;
    time_t res = mktime(&tms);    
    return res;
}

int save_content(){
    char* date_stamp = time_to_str(&last_modification);
    char* file_name = basename(path);
    char* new_path = calloc(strlen(archive_path)+strlen(date_stamp)+2 + strlen(file_name), 1);
    if(!new_path)rise_error("Can't allocate memory\n");
    strcat(new_path, archive_path);
    strcat(new_path, "/");
    strcat(new_path, file_name);
    strcat(new_path, date_stamp);

    FILE* new_file = fopen(new_path, "a+");
    if(!new_file)rise_error("Can't open file\n");
    fwrite(last_content_buffer, 1, strlen(last_content_buffer), new_file);
    free(date_stamp);
    free(new_path);
    fclose(new_file);
    return 0;
}

int file_was_modified(){
    struct stat file_stat;
    if(-1 == lstat(path, &file_stat))rise_errno();
    if(difftime(file_stat.st_mtime, last_modification) != 0) return 0;
    return 1;
}

int observe( unsigned int cycle_time){
    update_content_buffer();
    update_mod_date();
    
    clock_t cycle_start_time;
    clock_t check_time; 
    clock_t start_time = clock();
    double passed_time = 0;
    cycle_start_time = start_time;

    signal(SIGUSR1, signal_usr1);
    signal(SIGUSR2, signal_usr1);
    signal(SIGINT, signal_usr1);
    do{          
        if(run == 0){
            pause();
        }
        if(run == 2){
            break;
        }
        if(file_was_modified() == 0){
            save_content();
            update_content_buffer();
            update_mod_date();
            changes++;
        }        
        check_time = clock();
        double passed_time_cycle = (double)(check_time - 
            cycle_start_time)/CLOCKS_PER_SEC;
        passed_time += passed_time_cycle;
        double to_next_cycle = cycle_time - passed_time_cycle;

        if(to_next_cycle > 0){       
            sleep((unsigned int)to_next_cycle);
            passed_time_cycle += (unsigned int)to_next_cycle;
        }     
        
        check_time = clock();
        cycle_start_time = check_time;
    }while(1);
    return 0;
}


int main(int argc, char** argv){            
    changes = 0;
    if(argc < 3){
        perror("Not enough arguments\n");
        return -1;
    }
    archive_path = "./archive";
    last_content_buffer = malloc(1);
    if(!last_content_buffer)rise_error("Can't allocate memory\n");
    path = argv[1];
    observe(atoi(argv[2]));
    free(last_content_buffer);
    return changes;
}