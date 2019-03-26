#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "libfind.h"

struct mem_block_arr *mem_block = NULL;
char* dir = NULL;
char* file = NULL;
char* command = NULL;
char* tmp_file_name = NULL;

void create(unsigned int size){
    mem_block = (struct mem_block_arr*)calloc(1,sizeof(struct mem_block_arr));
    mem_block->arr = (char**)calloc(size, sizeof(char*));
    mem_block->max_size = size;
    mem_block->first_null = 0;
}

int add_mem_block(){
    FILE* tmp_file = fopen(tmp_file_name, "r");
    long length;
    char* file_content = NULL;
    //file length and declaring space for its content
    if(tmp_file){
        fseek(tmp_file, 0L, SEEK_END);
        length = ftell(tmp_file);
        fseek(tmp_file, 0L, SEEK_SET);
        file_content = (char*)calloc(length,sizeof(char));
        if(fread(file_content, length, 1, tmp_file) != 1){
            exit(1);
        }
        fclose(tmp_file);    
    }
    //alocating next memory block in first null in array
    int allocated_index = mem_block->first_null;
    if(allocated_index != mem_block->max_size){ 
        mem_block->arr[allocated_index] = file_content;
        set_next_null();
        return allocated_index;
    }
    return -1;
}

void set_next_null(){
    if(mem_block == NULL)return; // throw?
    unsigned int size = mem_block->max_size;
    for(unsigned int i=mem_block->first_null; i<size; i++){
        if(mem_block->arr[i] == NULL){
            mem_block->first_null = i;
            return;
        }
    }
    mem_block->first_null = size;
}

void remove_mem_block(unsigned int i){
    if(mem_block == NULL || i >= mem_block->max_size){
        printf("Out of range!\n");
        return; 
    }
    free(mem_block->arr[i]);
    mem_block->arr[i] = NULL;
    if(mem_block->first_null > i)mem_block->first_null = i;
}

void set_curr_dir(char* new_dir){
    dir = new_dir;
}

void set_curr_file(char* new_file){
    file = new_file;
}

void set_tmp_file(char* new_tmp_file){
    tmp_file_name = new_tmp_file;
}

void set_command(){
    unsigned int length = strlen("find")+ 1 + strlen (dir) + 1 + strlen("-name") +
        2 + strlen(file) + 4 + strlen(tmp_file_name);
    command = (char*)calloc(length,  sizeof(char));
    strcat(command, "find ");
    strcat(command, dir);
    strcat(command, " -name \"");
    strcat(command, file);
    strcat(command, "\" > ");
    strcat(command, tmp_file_name);
}

void search(char* arg_dir, char* arg_file, char* new_tmp_file){
    set_curr_dir(arg_dir);
    set_curr_file(arg_file);
    set_tmp_file(new_tmp_file);
    set_command();
    system(command);    
}

char* get_mem_block(unsigned int i){
    if(mem_block == NULL || i >= mem_block->max_size){
        printf("Out of range!\n");
        return NULL;
    }
    return mem_block->arr[i];
}

void clear_mem(){
    if(mem_block != NULL){
        unsigned int length = mem_block->max_size;
        for(unsigned int i = 0; i<length ; i++){
            free(mem_block->arr[i]);
        }
        free(mem_block->arr);
        free(mem_block);
    }        
}