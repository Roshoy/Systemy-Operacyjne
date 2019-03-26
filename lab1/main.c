#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#ifndef DLL
#include "libfind.h"
#endif

#ifdef DLL
#include <dlfcn.h>
#endif

long TICKS_PER_SEC = 0;

void to_file(FILE* buffer, clock_t r, clock_t s, clock_t u, char* command){
    if(buffer){
        fprintf(buffer, command);
        fprintf(buffer,":\n");
        fprintf(buffer,"Real time: %e s\nSystem time: %e s\nUser time: %e s\n",
            (double)r/TICKS_PER_SEC, (double)s/TICKS_PER_SEC, (double)u/TICKS_PER_SEC);
    }

}

int main(int argc, char** argv){
    #ifdef DLL
    void *handle = dlopen("./libfind.so.1", RTLD_LAZY);
    if(!handle){
        printf("Error\n");
        return 0;
    }
    void (*create)(unsigned int n);
    void (*search)();
    unsigned int (*add_mem_block)();
    void (*remove_mem_block)(unsigned int i);
    char* (*get_mem_block)(unsigned int i);
    void (*clear_mem)();

    create = (void(*)(unsigned int n))dlsym(handle, "create");
    search = (void(*)())dlsym(handle, "search");
    add_mem_block = (unsigned int(*)())dlsym(handle,"add_mem_block");
    remove_mem_block = (void(*)(unsigned int n))dlsym(handle, "remove_mem_block");
    get_mem_block = (char*(*)(unsigned int n))dlsym(handle, "get_mem_block");
    clear_mem = (void(*)())dlsym(handle,"clear_mem");
    #endif

    char* input = (char*)malloc(20 * sizeof(char));
    unsigned int* value = (unsigned int *)malloc(sizeof(unsigned int));
    char* name_file_temp = (char*)malloc(300 * sizeof(char));
    char* file_name = (char*)malloc(300 * sizeof(char));
    char* directory = (char*)malloc(300 * sizeof(char));
    unsigned int* reps = (unsigned int*)malloc(sizeof(unsigned int));
    TICKS_PER_SEC = sysconf(_SC_CLK_TCK);
    printf("Rep argument always tells how much times you should repeat given task,\n");
    printf("so the results may be more precise\nType help to see commands\n\n");
    FILE* buffer = NULL;
    if(argc > 1){
        buffer = fopen(argv[1],"a");
        if(!buffer)printf("blad plik\n");
    }
    struct tms time_clock;
    clock_t R_start;
    clock_t R_end;
    clock_t R_time;
    clock_t U_start;
    clock_t U_end;
    clock_t U_time;
    clock_t S_start;
    clock_t S_end;
    clock_t S_time;
    do{
        scanf("%s", input);
        if(strcmp(input,"create_table") == 0){   
                     
            scanf("%u", value);
            R_start = times(&time_clock);            
            S_start = time_clock.tms_stime;
            U_start = time_clock.tms_utime;                 
            for(int i=0; i< 10000; i++)create(*value);
            R_end = times(&time_clock);
            S_end = time_clock.tms_stime;
            U_end = time_clock.tms_utime;
            R_time = R_end - R_start;
            S_time = S_end - S_start;
            U_time = U_end - U_start;
            to_file(buffer, R_time, S_time, U_time, input);
        }else if(strcmp(input,"search_directory") == 0){
            scanf("%u", reps);
            scanf("%s", name_file_temp);
            scanf("%u", value);
            for(;*value>0;(*value)--){
                scanf("%s",directory);
                scanf("%s",file_name);
                R_start = times(&time_clock);          
                S_start = time_clock.tms_stime;
                U_start = time_clock.tms_utime;                 
                for(;*reps>0;(*reps)--)search(directory,file_name,name_file_temp);
                R_end = times(&time_clock);
                S_end = time_clock.tms_stime;
                U_end = time_clock.tms_utime;
                R_time = R_end - R_start;
                S_time = S_end - S_start;
                U_time = U_end - U_start;
                to_file(buffer, R_time, S_time, U_time, input);
                
            }
        }else if(strcmp(input, "search_directory_add")== 0){
            scanf("%u", reps);
            scanf("%s", name_file_temp);
            scanf("%u", value);
            for(;*value>0;(*value)--){
                scanf("%s",directory);
                scanf("%s",file_name);
                R_start = times(&time_clock);           
                S_start = time_clock.tms_stime;
                U_start = time_clock.tms_utime;                 
                for(;*reps>0;(*reps)--){
                    search(directory,file_name,name_file_temp);
                    add_mem_block();
                }
                R_end = times(&time_clock);
                S_end = time_clock.tms_stime;
                U_end = time_clock.tms_utime;
                R_time = R_end - R_start;
                S_time = S_end - S_start;
                U_time = U_end - U_start;
                to_file(buffer, R_time, S_time, U_time, input);
                
            }
            
        }else if(strcmp(input, "remove_block")== 0){
            scanf("%u", value);
            R_start = times(&time_clock);            
            S_start = time_clock.tms_stime;
            U_start = time_clock.tms_utime;                 
            remove_mem_block(*value); 
            R_end = times(&time_clock);
            S_end = time_clock.tms_stime;
            U_end = time_clock.tms_utime;
            R_time = R_end - R_start;
            S_time = S_end - S_start;
            U_time = U_end - U_start; 
            to_file(buffer, R_time, S_time, U_time, input);
            
        }else if(strcmp(input, "add_block")== 0){
            R_start = times(&time_clock);            
            S_start = time_clock.tms_stime;
            U_start = time_clock.tms_utime;                 
            add_mem_block();
            R_end = times(&time_clock);
            S_end = time_clock.tms_stime;
            U_end = time_clock.tms_utime;
            R_time = R_end - R_start;
            S_time = S_end - S_start;
            U_time = U_end - U_start;
            to_file(buffer, R_time, S_time, U_time, input);
        }else if(strcmp(input, "add_remove")== 0){
            scanf("%u", reps);
            R_start = times(&time_clock);            
            S_start = time_clock.tms_stime;
            U_start = time_clock.tms_utime;                 
            for(;*reps>0;(*reps)--){
                *value = add_mem_block();
                remove_mem_block(*value);
            }
            R_end = times(&time_clock);
            S_end = time_clock.tms_stime;
            U_end = time_clock.tms_utime;
            R_time = R_end - R_start;
            S_time = S_end - S_start;
            U_time = U_end - U_start;
            to_file(buffer, R_time, S_time, U_time, input);
        }else if(strcmp(input, "get_block")== 0){
            if(scanf("%u", value)>0){
                printf(get_mem_block(*value));
                printf("\n");
            }
            
        }else if(strcmp(input, "help")== 0){
            printf(" create_table size\n");
            printf(" search_directory rep name_file_temp search_count [dir file]\n");
            printf(" search_directory_add rep name_file_temp search_count [dir file] (calls add_block)\n");
            printf(" remove_block index\n");
            printf(" add_block\n");
            printf(" add_remove rep\n");
            printf(" get_block index\n");
            printf(" help \n");
            printf(" exit \n");
            
        }else if(strcmp(input, "exit")!= 0){
            printf("Unrecognized command, try help for command list\n");
        }
        
    }while(strcmp(input, "exit")!= 0);
    free(input);
    free(value);
    free(name_file_temp);
    free(directory);
    free(file_name);
    clear_mem();
    sysconf(_SC_CLK_TCK);
    fclose(buffer);
    
    #ifdef DLL
    dlclose(handle);
    #endif
    return 0;
}
