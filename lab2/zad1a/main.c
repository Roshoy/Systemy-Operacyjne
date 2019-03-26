#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>

int generate(char* file_name,int blocks, int block_size){
    int file = open(file_name, O_WRONLY | O_CREAT | O_TRUNC,
     S_IRUSR | S_IWUSR | S_IXUSR);
    if(file < 0)return -1;
    char* buff = (char*)calloc(block_size,sizeof(char));
    for(int i=0; i<blocks; i++){
        for(int j=0; j<block_size; j++){
            buff[j] = rand()%256 - 128;
        }
        if(0 > write(file, buff, block_size))return -1;
    }
    if(0 > lseek(file, 0, SEEK_SET))return -1;
    if(0 > write(file,buff, block_size))return -1;
    if(0 > close(file))return -1;
    free(buff);
    return 0;
}

int sort_sys(char* file_name, int blocks, int block_size){
    int file = open(file_name, O_RDWR);
    char *min;
    char *buff;
    min = (char*)calloc(block_size,sizeof(char));
    buff = (char*)calloc(block_size,sizeof(char));
    int min_index;
    for(int start=0; start<blocks-1; start++){
        min_index = start;
        if(0 > lseek(file, start * block_size, SEEK_SET))return -1;
        if(0 > read(file,min,block_size))return -1;
        for(int j=start; j<blocks; j++){
            if(0 > lseek(file, j * block_size, SEEK_SET))return -1;
            if(0 > read(file, buff, block_size))return -1;
            if(min[0] > buff[0]){
                strcpy(min,buff);
                min_index = j;
            }
        }
        if(0 > lseek(file, start * block_size,SEEK_SET))return -1;
        if(0 > read(file, buff, block_size))return -1;
        if(0 > lseek(file, -block_size,SEEK_CUR))return -1;
        if(0 > write(file, min, block_size))return -1;
        if(0 > lseek(file, min_index*block_size, SEEK_SET))return -1;
        if(0 > write(file, buff, block_size))return -1;
    }
    
    free(min);   
    free(buff);
    if(0 > close(file))return -1;
    return 0;
}

int copy_sys(char* file_name1, char* file_name2, int blocks, int block_size){
    int file1 = open(file_name1, O_RDONLY);
    int file2 = open(file_name2, O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IXUSR);
    if(file1 < 0 || file2 < 0)return -1;
    char* buff = (char*)calloc(block_size, sizeof(char));
    for(int i=0; i < blocks; i++){
        if(0 > read(file1, buff, block_size))return -1;
        if(0 > write(file2, buff, block_size))return -1;
    }
    if(0 > close(file1))return -1;
    if(0 > close(file2))return -1;
    free(buff);
    return 0;
}

int sort_lib(char* file_name, int blocks, int block_size){
    FILE* file = fopen(file_name, "r+");
    if(file == NULL)return -1;
    char *min;
    char *buff;
    min = (char*)calloc(block_size,sizeof(char));
    buff = (char*)calloc(block_size,sizeof(char));
    int min_index;
    for(int start=0; start<blocks-1; start++){
        min_index = start;
        if(0 != fseek(file, start*block_size, SEEK_SET))return -1;
        if(block_size != fread(min, 1, block_size, file))return -1;
        for(int j=start; j<blocks; j++){
            if(0 != fseek(file, j * block_size, SEEK_SET))return -1;
            if(block_size != fread(buff,1,block_size,file))return -1;
            if(min[0] > buff[0]){
                strcpy(min,buff);
                min_index = j;
            }
        }
        if(0 != fseek(file, start * block_size,SEEK_SET))return -1;
        if(block_size != fread(buff,1,block_size,file))return -1;
        if(0 != fseek(file, -block_size,SEEK_CUR))return -1;
        if(block_size != fwrite(min,1,block_size,file))return -1;
        if(0 != fseek(file, min_index*block_size, SEEK_SET))return -1;
        if(block_size != fwrite(buff,1,block_size,file))return -1;
    }
    
    free(min);   
    free(buff);
    if(0 != fclose(file))return -1;
    return 0;
}

int copy_lib(char* file_name1, char* file_name2, int blocks, int block_size){
    FILE *file1 = fopen(file_name1, "r");
    FILE *file2 = fopen(file_name2, "w");
    if(file1 == NULL || file2 == NULL)return -1;
    char* buff = (char*)calloc(block_size, sizeof(char));
    for(int i=0; i < blocks; i++){
        if(block_size != fread(buff,1,block_size,file1))return -1;
        if(block_size != fwrite(buff,1,block_size,file2))return -1;
    }
    if(0 != fclose(file1))return -1;
    if(0 != fclose(file2))return -1;
    free(buff);
    return 0;
}

void backup_copy(char *file_name){
    int length = 4 + strlen(file_name) * 2 + 7;
    char* command = (char*)calloc(length, sizeof(char));
    strcat(command, "cp ");
    strcat(command, file_name);
    strcat(command, " ");
    strcat(command, file_name);
    strcat(command, "_backup");
    system(command);
}

void bring_backup(char *file_name){
    int length = 4 + strlen(file_name) * 2 + 7;
    char* command = (char*)calloc(length, sizeof(char));
    strcat(command, "cp ");
    strcat(command, file_name);
    strcat(command, "_backup");
    strcat(command, " ");
    strcat(command, file_name);    
    system(command);
}

void print_test(FILE* results,char* tested_func, clock_t u_time, clock_t s_time){
    int TICKS_PER_SEC = sysconf(_SC_CLK_TCK);
    if(results){
        fprintf(results, tested_func);
        fprintf(results, ":  UserTime: %6f s   SystemTime: %6f s\n",
        (double)u_time/TICKS_PER_SEC, (double)s_time/TICKS_PER_SEC);
    }
    printf(tested_func);
    printf("\n");
}

int main(int argc, char** argv){
    srand(time(NULL));
    if(argc>1){

        if(argc>4 && strcmp(argv[1],"generate")==0){
            if(-1 == generate(argv[2],atoi(argv[3]),atoi(argv[4])))return -1;
            backup_copy(argv[2]);
        }else if(argc>5 && strcmp(argv[1],"sort")==0){
            if(strcmp(argv[5],"sys")){
                bring_backup(argv[2]);
                if(-1 == sort_sys(argv[2],atoi(argv[3]),atoi(argv[4])))return -1;
            }else{
                bring_backup(argv[2]);
                if(-1 == sort_lib(argv[2],atoi(argv[3]),atoi(argv[4])))return -1;
            }
        }else if(argc>6 && strcmp(argv[1],"copy")==0){
            if(strcmp(argv[6],"sys")){
                bring_backup(argv[2]);
                if(-1 == copy_sys(argv[2],argv[3],atoi(argv[4]),atoi(argv[5])))return -1;
            }else{
                bring_backup(argv[2]);
                if(-1 == copy_lib(argv[2],argv[3],atoi(argv[4]),atoi(argv[5])))return -1;
            }
        }else if(argc>3 && strcmp(argv[1], "test")==0){
            printf("test\n");
            char* file_name = "data";
            char* file_name2 = "data2";
            FILE* results = fopen("wyniki.txt", "a");
            int blocks = atoi(argv[2]);
            int block_size = atoi(argv[3]);
            fprintf(results,"Blocks: %d   Of size: %d B\n",blocks,block_size);
            clock_t U_start;
            clock_t U_end;
            clock_t S_start;
            clock_t S_end;
            struct tms time_clock;
            
            if(-1 == generate(file_name, blocks,block_size))return -1;
            backup_copy(file_name);
            times(&time_clock);
            S_start = time_clock.tms_stime;
            U_start = time_clock.tms_utime;
            if(-1 == sort_sys(file_name, blocks,block_size))return -1;
            times(&time_clock);
            S_end = time_clock.tms_stime;
            U_end = time_clock.tms_utime;
            print_test(results, "sort_sys", U_end - U_start, S_end-S_start);
            bring_backup(file_name);
            times(&time_clock);
            S_start = time_clock.tms_stime;
            U_start = time_clock.tms_utime;
            if(-1 == sort_lib(file_name, blocks,block_size))return -1;
            times(&time_clock);
            S_end = time_clock.tms_stime;
            U_end = time_clock.tms_utime;
            print_test(results, "sort_lib", U_end - U_start, S_end-S_start);
            bring_backup(file_name);
            times(&time_clock);
            S_start = time_clock.tms_stime;
            U_start = time_clock.tms_utime;
            if(-1 == copy_sys(file_name, file_name2, blocks,block_size))return -1;
            times(&time_clock);
            S_end = time_clock.tms_stime;
            U_end = time_clock.tms_utime;
            print_test(results, "copy_sys", U_end - U_start, S_end-S_start);
            bring_backup(file_name);
            times(&time_clock);
            S_start = time_clock.tms_stime;
            U_start = time_clock.tms_utime;
            if(-1 == copy_lib(file_name, file_name2, blocks,block_size))return -1;
            times(&time_clock);
            S_end = time_clock.tms_stime;
            U_end = time_clock.tms_utime;
            print_test(results, "copy_lib", U_end - U_start, S_end-S_start);
            fprintf(results,"\n");
            if(0 != fclose(results))return -1;
        }else{
            printf("Cos z args\n");
        }
        
    }else{
        printf("Not enough arguments!\n");
    }
    //write(file, "ab", sizeof(char)*2);
    return 0;
}