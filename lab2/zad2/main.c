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

// ścieżka bezwzględna pliku,
// rodzaj pliku (zwykły plik - file, katalog - dir, urządzenie znakowe - char dev, urządzenie blokowe - block dev, potok nazwany - fifo, link symboliczny - slink, soket - sock) 
// rozmiar w bajtach,
// datę ostatniego dostępu,
// datę ostatniej modyfikacji.

time_t comp_time;
char comp_type;

enum When{
    BEFORE = -1,
    NOW = 0,
    AFTER = 1
};

void rise_error(char* text){
    perror(text);
    exit(1);
}

void rise_errno(){
    perror(NULL);
    exit(1);
}

char* absolute_path(char* dir_path, char* d_name){
    int length = strlen(dir_path) + strlen(d_name) + 1;
    char* buff = calloc(length, 1);
    if(buff == NULL)rise_errno();
    strcat(buff, dir_path);
    strcat(buff, "/");
    strcat(buff, d_name);
    return realpath(buff,NULL);
}

char *file_type(mode_t st_mode){
    char *type = "unknown";
    if (S_ISREG(st_mode)) {
        type = "file";
    } else if (S_ISDIR(st_mode)) {
        type = "dir";
    } else if (S_ISCHR(st_mode)) {
        type = "char dev";
    } else if (S_ISBLK(st_mode)) {
        type = "block dev";
    } else if (S_ISFIFO(st_mode)) {
        type = "fifo";
    } else if (S_ISLNK(st_mode)) {
        type = "slink";
    } else if (S_ISSOCK(st_mode)) {
        type = "sock";
    }
    return type;
}

int is_date_good(const time_t mod_time){
    double diff = difftime( mod_time, comp_time);
    if((diff < 0 && comp_type == '<')||(diff == 0 && comp_type == '=')||
    (diff > 0 && comp_type == '>'))return 0;
    return -1;
}

char* time_to_str(const time_t* time){
    char* buff = calloc(20, 1);
    if(buff == NULL)rise_errno();
    if(0 == strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(time))){
        rise_error("Error while converting time to string!");
    };
    return buff;
}

int print_file_details(const struct stat *cur_stat){
    char* f_type = file_type(cur_stat->st_mode);
    if(!f_type)rise_errno();
    printf("File type: %s\n", f_type);
    printf("File size: %ld B\n", cur_stat->st_size);
    char* a_time = time_to_str(&cur_stat->st_atime);
    if(!a_time)rise_errno();
    printf("Access time: %s\n", a_time);
    char* m_time = time_to_str(&cur_stat->st_mtime);
    if(!m_time)rise_errno();
    printf("Modification time: %s\n\n", m_time);
    free(m_time);
    free(a_time);
    return 0;
}

int fn(const char *path,const struct stat *cur_stat, int typeflag, struct FTW *ftwbuf){
    if(is_date_good(cur_stat->st_mtime) == 0){
        char *abs_path;
        abs_path = realpath(path,NULL);
        if(!abs_path)rise_errno();
        printf("Absolute path: %s\n", abs_path);
        print_file_details((const struct stat *)cur_stat);
        free(abs_path);
    }
    return 0;
}

int search_dir_stat(char* dir_path){
    DIR *dir = opendir(dir_path);
    if(NULL == dir)return 0;
    struct dirent *current;
    struct stat cur_stat;
    char* abs_path;
    while((current = readdir(dir))!=0){     
        if( 0 == strcmp(current->d_name,"..")){
            continue;            
        }      
        abs_path = malloc(strlen(dir_path) + 1 + strlen(current->d_name));  
        
        sprintf(abs_path,"%s/%s", dir_path, current->d_name);
        
        if(0 != lstat(abs_path, &cur_stat))rise_errno();
      //  
        if((!S_ISDIR(cur_stat.st_mode) || strcmp(current->d_name,".") == 0)
            && is_date_good(cur_stat.st_mtime) == 0){
            printf("Absolute path: %s\n", abs_path);           
            print_file_details(&cur_stat);
        }
        if(S_ISDIR(cur_stat.st_mode)==1 && strcmp(current->d_name,".") != 0)
            search_dir_stat(abs_path);
        
        free(abs_path);
    }
    
    if(0 != closedir(dir))rise_errno();
    return 0;
}

int search_dir_nftw(char* dir_path){
    return nftw(dir_path, fn, 100, FTW_PHYS);
}

time_t string_to_time(char* time_arg){
    struct tm tms = {0};
    char* time_format = "%d-%d-%d %d:%d:%d";
    if(EOF == sscanf(time_arg, time_format, &tms.tm_mday, &tms.tm_mon, &tms.tm_year,
        &tms.tm_hour, &tms.tm_min, &tms.tm_sec)) rise_error("Error while parsing time to string");
    
    tms.tm_mon--;
    tms.tm_year -= 1900;
    time_t res = mktime(&tms);
    
    return res;
}

int main(int argc, char** argv){
    if(argc <= 3)rise_error("Not enough arguments!");
    comp_time = string_to_time(argv[3]);
    comp_type = argv[2][0];
    char* path_abs = realpath(argv[1], NULL);
    printf("------------------------------------------------");
    search_dir_stat(path_abs);
    printf("------------------------------------------------");
    search_dir_nftw(path_abs);
    free(path_abs);
    return 0;
}