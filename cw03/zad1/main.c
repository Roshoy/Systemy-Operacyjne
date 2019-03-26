#define _XOPEN_SOURCE 500
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include <dirent.h>
#include <ftw.h>
#include <sys/wait.h>

time_t comp_time;
char comp_type;
char* run_path;

void rise_error(char* text){
    perror(text);
    exit(1);
}

void rise_errno(){
    perror(NULL);
    exit(1);
}

int is_date_good(const time_t mod_time){
    double diff = difftime( mod_time, comp_time);
    if((diff < 0 && comp_type == '<')||(diff == 0 && comp_type == '=')||
    (diff > 0 && comp_type == '>'))return 0;
    return -1;
}

char* relative_path(const char* abs_path){
    char *res;
    int length = strlen(abs_path);
    int run_len = strlen(run_path);
    if(length - run_len <= 1){
        //function return pointer to allocated memory and it will freed
        res = calloc(2, sizeof(char));
        if(!res)rise_error("Can't allocate memory\n");
        res[0] = '.';
        return res;
    }

    run_len++;

    res = calloc((length - run_len)+1, sizeof(char));
    if(!res)rise_error("Can't allocate memory\n");
    for(int i=run_len; i<length; i++){
        res[i - run_len] = abs_path[i];
    }
    return res;
}

int new_process(const char* abs_path){
    int child = fork();
    if(child == -1)rise_errno();
    if(child != 0){
        wait(NULL);
    }else{
        
        printf("PID: %u\n", (int)getpid());
        char* path = relative_path(abs_path);
        printf("Relative path: %s\n", path);
        int child2 = fork();
        if(child2 != 0){
            wait(NULL);
        }else{
            if(0 != execl("/bin/ls", "ls", "-l", abs_path, NULL))rise_errno();
        }
        free(path);
    }
    
    return child;
}

int fn(const char *path,const struct stat *cur_stat, int typeflag, struct FTW *ftwbuf){
    if(S_ISDIR(cur_stat->st_mode)==1 && is_date_good(cur_stat->st_mtime) == 0){
        char *abs_path;
        abs_path = realpath(path,NULL);
        if(!abs_path)rise_errno();
        new_process(abs_path);
        free(abs_path);
    }
    return 0;
}

int search_dir_stat(const char* dir_path){
    DIR *dir = opendir(dir_path);
    if(NULL == dir)return 0;
    struct dirent *current;
    struct stat cur_stat;
    char* abs_path;
    while((current = readdir(dir))!=0){     
        if( 0 == strcmp(current->d_name,"..")){
            continue;            
        }      
        abs_path = calloc(strlen(dir_path) + 2 + strlen(current->d_name), sizeof(char));  
        sprintf(abs_path,"%s/%s", dir_path, current->d_name);        
        if(0 != lstat(abs_path, &cur_stat))rise_errno();
        if(S_ISDIR(cur_stat.st_mode)==1 && 
            (strcmp(current->d_name,".") != 0 || strlen(abs_path) - strlen(run_path) == 2)
            && is_date_good(cur_stat.st_mtime) == 0){
            if(0 == new_process(abs_path) && strcmp(current->d_name,".") != 0){
                search_dir_stat(abs_path);
                break;
            }
        }
        free(abs_path);
    }
    if(0 != closedir(dir))rise_errno();
    return 0;
}

int search_dir_nftw(const char* dir_path){
    return nftw(dir_path, fn, 100, FTW_PHYS);
}

time_t string_to_time(char* time_arg){
    struct tm tms = {0};
    char* time_format = "%d-%d-%d %d:%d:%d";
    if(EOF == sscanf(time_arg, time_format, &tms.tm_year, &tms.tm_mon, &tms.tm_mday,
        &tms.tm_hour, &tms.tm_min, &tms.tm_sec))
        rise_error("Error while parsing time to string");    
    tms.tm_mon--;
    tms.tm_year -= 1900;
    time_t res = mktime(&tms);
    
    return res;
}

int main(int argc, char** argv){
    if(argc <= 3)rise_error("Not enough arguments!");
    
    comp_time = string_to_time(argv[3]);
    comp_type = argv[2][0];
    run_path = realpath(argv[1], NULL);
    search_dir_stat(run_path);
    
    ///search_dir_nftw(run_path);
    free(run_path);
    return 0;
}