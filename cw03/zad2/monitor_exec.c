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
char* path;
int changes;
char* archive_path;

void rise_error(char* text){
    perror(text);
    exit(-1);
}

void rise_errno(){
    perror(NULL);
    exit(-1);
}

int update_mod_date(){
    struct stat file_stat;
    if(-1 == lstat(path, &file_stat))rise_errno();
    last_modification = file_stat.st_mtime;
    return 0;
}

char* time_to_str(const time_t* time){
    char* buff = calloc(21, 1);
    if(buff == NULL)rise_error("Can't allocate memory\n");
    if(0 == strftime(buff, 21, "_%Y-%m-%d_%H-%M-%S", localtime(time))){
        //rise_error("Error while converting time to string!");
        printf("Error while converting time to string!\n");
    }
    return buff;
}

time_t string_to_time(char* time_arg){
    struct tm tms = {0};
    char* time_format = "%d-%d-%d %d:%d:%d";
    if(EOF == sscanf(time_arg, time_format, &tms.tm_year, &tms.tm_mon, &tms.tm_mday,
        &tms.tm_hour, &tms.tm_min, &tms.tm_sec))
        rise_error("Error while parsing time to string\n");    
    tms.tm_mon--;
    tms.tm_year -= 1900;
    time_t res = mktime(&tms);    
    return res;
}

int file_was_modified(){
    struct stat file_stat;
    if(-1 == lstat(path, &file_stat))rise_errno();
    if(difftime(file_stat.st_mtime, last_modification) != 0) return 0;
    return 1;
}

int observe( unsigned int cycle_time){
    update_mod_date();

    clock_t cycle_start_time;
    clock_t check_time; 
    clock_t start_time = clock();
    double passed_time = 0;
    cycle_start_time = start_time;

    do{          
        //////////////here it works
        if(file_was_modified() == 0){
            
            char* date_stamp = time_to_str(&last_modification);
            char* file_name = basename(path);
            char* new_path = calloc(strlen(archive_path)+strlen(date_stamp)+2 + strlen(file_name), 1);
            if(!new_path)rise_error("Can't allocate memory\n");
            strcat(new_path, archive_path);
            strcat(new_path, "/");
            strcat(new_path, file_name);
            strcat(new_path, date_stamp);

            int child = (int)fork();
            if(child == -1)rise_errno();
            if(child != 0){
                int returned;
                wait(&returned);
                if(WIFEXITED(returned) && WEXITSTATUS(returned) != 0){
                    rise_error("cp command failed\n");
                }
            }else{                 
                if(-1 == execl("/bin/cp","cp" , path, new_path,NULL))rise_errno();
            }
            free(date_stamp);
            free(new_path);
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
            if(to_work_end < to_next_cycle)return 0;            
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
    if(argc < 4)rise_error("Not enough arguments\n");
    work_time = atoi(argv[2]);
    path = argv[1];
    observe(atoi(argv[3]));
    return changes;
}