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

unsigned int work_time;

time_t last_modification;
char* last_content_buffer;
int changes;
char* path;
char* archive_path;


void rise_error(char* text){
    perror(text);
    exit(-1);
}

void rise_errno(){
    perror(NULL);
    exit(-1);
}

// void print_usage(){
//     struct rusage* all_usage = malloc(sizeof(struct rusage));
//     if(!all_usage)rise_error("Can't allocate memory\n");
//     if(0!=getrusage(RUSAGE_SELF, all_usage))rise_errno();
//     printf("PID: %d  CPU_USER: %ld.%06ld s  CPU_SYSTEM: %ld.%06ld s\n",getpid(), 
//         all_usage->ru_utime.tv_sec,all_usage->ru_utime.tv_usec, 
//         all_usage->ru_stime.tv_sec,all_usage->ru_stime.tv_usec);
// }

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

    do{          
        //////////////here it works
        if(file_was_modified() == 0){
            save_content();
            update_content_buffer();
            update_mod_date();
            changes++;
        }
        
        //////////////here it ends work
        check_time = clock();
        double passed_time_cycle = (double)(check_time - 
            cycle_start_time)/CLOCKS_PER_SEC;
        passed_time += passed_time_cycle;
        double to_work_end = work_time - passed_time;
        double to_next_cycle = cycle_time - passed_time_cycle;

        if(to_work_end > 0 && to_next_cycle > 0){
            if(to_work_end < to_next_cycle){
                return 0;
            }            
            sleep((unsigned int)to_next_cycle);
            passed_time_cycle += (unsigned int)to_next_cycle;
        }     
        
        check_time = clock();
        do{            
            time_t buff = clock();
            passed_time_cycle += (double)(buff-check_time)/CLOCKS_PER_SEC;
            check_time = buff;            
        }while(passed_time_cycle < cycle_time);
        passed_time += passed_time_cycle;        
        cycle_start_time = check_time;
    }while(passed_time < work_time);
    return 0;
}


int main(int argc, char** argv){            
    changes = 0;
    archive_path = "./archive";
    if(argc < 4){
        perror("Not enough arguments\n");
        return -1;
    }
    work_time = atoi(argv[2]);
    last_content_buffer = malloc(1);
    if(!last_content_buffer)rise_error("Can't allocate memory\n");
    path = argv[1];
    observe(atoi(argv[3]));
    free(last_content_buffer);
    return changes;
}