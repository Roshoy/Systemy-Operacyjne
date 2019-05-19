#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <errno.h>

#define BLOCK 0
#define INTERLEAVED 1

double **filter;
int **org_img;
int **result_img;
int img_size[2];
int max_value;
int filter_size;
int threads_count;

long *thread_args;

void read_org_img(const char* path);
void read_filter(const char* path);
void save_to_file(const char* path);

void filtering_function(int x);
void* thread_block(void *);
void* thread_interleaved(void *);
int pixel_update(int x, int y);


void free_mem();

int main(int argc, char **argv){
    if(argc < 6){
        printf("Not enough arguments\n");
        exit(1);
    }
    threads_count = atoi(argv[1]);
    int division_type;
    if(strcmp(argv[2], "block") == 0){
        division_type = BLOCK;
    }else{
        division_type = INTERLEAVED;
    }
    char *org_img_path = argv[3];
    char *filter_path = argv[4];
    char *result_path = argv[5];
    read_org_img(org_img_path);
    read_filter(filter_path);

    pthread_t *threads = calloc(threads_count, sizeof(pthread_t));
    thread_args = calloc(threads_count, sizeof(long));
    struct timeval  tv_start, tv_end;
    long **thread_times = calloc(threads_count, sizeof(long*));
    gettimeofday(&tv_start, NULL);
    for(int i=0; i<threads_count; i++){
        thread_args[i] = i;
        if(division_type == BLOCK)pthread_create(threads+i, NULL, thread_block, thread_args+i);
        else pthread_create(threads+i, NULL, thread_interleaved, thread_args+i);
    }

    for(int i=0; i<threads_count; i++){
        pthread_join(threads[i], (void **)&thread_times[i]);
    }
    gettimeofday(&tv_end, NULL);
    long usec = tv_end.tv_usec - tv_start.tv_usec;
    long sec = tv_end.tv_sec - tv_start.tv_sec;
    if(usec < 0){
        sec--;
        usec += 1000000;
    } 
    printf("Threads: %2d | SUM TIME : %3ld.%06ld s\n", threads_count, sec, usec);
    for(int i=0;i<threads_count; i++){
        printf("Thread %2d: %3ld.%06ld s\n", i, *thread_times[i]/1000000L, *thread_times[i]%1000000L);
    }

    save_to_file(result_path);
    for(int i=0; i<threads_count; i++){
        free(thread_times[i]);
    }
    free(thread_times);
    free(threads);
    free(thread_args);
    free_mem();
    return 0;
}

void read_org_img(const char *path){
    FILE *file = fopen(path, "r");
    if(!file){
        printf("Not opened %s :\n%s\n", path, strerror(errno));
    }
    char *buff = NULL;
    size_t len = 0;
    getline(&buff, &len, file); // read P2
    free(buff);
    getline(&buff, &len, file); // read # things
    free(buff);
    //fscanf(file, "P2\n");
    fscanf(file, "%d %d\n", img_size, img_size+1);
    fscanf(file, "%d\n", &max_value);
    org_img = calloc(img_size[0], sizeof(int*));
    result_img = calloc(img_size[0], sizeof(int*));
    for(int x=0; x<img_size[0]; x++){
        org_img[x] = calloc(img_size[1], sizeof(int));
        result_img[x] = calloc(img_size[1], sizeof(int));
    }
    for(int y=0; y<img_size[1] ; y++){
        for(int x=0; x<img_size[0] ; x++){
            fscanf(file, "%d", &org_img[x][y]);
            //printf("%3d ", org_img[x][y]);
        }
        //printf("\n");
    }   
    fclose(file);
}

void read_filter(const char *path){
    /********************
    size
    num1 num2 ...
    ...
    ********************/
    FILE *file = fopen(path, "r");
    if(!file){
        printf("Not opened %s :\n%s\n", path, strerror(errno));
    }
    fscanf(file, "%d\n", &filter_size);

    filter = calloc(filter_size, sizeof(double*));
    for(int x=0; x<filter_size; x++){
        filter[x] = calloc(filter_size, sizeof(double));
    }
    for(int y=0; y<filter_size; y++){
        for(int x=0; x<filter_size; x++){
            fscanf(file, "%lf", &filter[x][y]);
            //printf("%lf ", filter[x][y]);
        }
        //printf("\n");
    }   
    fclose(file);

}

void filtering_function(int x){
    for(int y=0; y<img_size[1]; y++){
        double s = 0;
        for(int i=0; i<filter_size; i++){
            for(int j=0; j<filter_size; j++){
                int a = fmax(0, fmin(x - ceil(filter_size/2) + i, img_size[0]-1));
                int b = fmax(0, fmin(y - ceil(filter_size/2) + j, img_size[1]-1));
                s += org_img[a][b] * filter[i][j];
            }
        }
        //printf("s[%d][%d] = %f\n", x, y, s);
        
        result_img[x][y] = round(s);
    }
}

void *thread_block(void *p){
    //printf("%d", img_size[0]);
    struct timeval  tv_start, tv_end;
    gettimeofday(&tv_start, NULL);
    int k = (*(int*)p);
    for(int i=(k)*ceil(img_size[0]/threads_count); i<(k+1)*ceil(img_size[0]/threads_count); i++){
        filtering_function(i);
    }
    gettimeofday(&tv_end, NULL);
    long *t = malloc(sizeof(long));
    *t = (tv_end.tv_sec - tv_start.tv_sec)*1000000L + tv_end.tv_usec - tv_start.tv_usec;
    //printf("%d\n",t);
    pthread_exit(t);
}

void *thread_interleaved(void *p){
    struct timeval  tv_start, tv_end;
    gettimeofday(&tv_start, NULL);
    int k = (*(int*)p);
    //printf("%d\n", k);
    for(int i=k; i<img_size[0]; i+=threads_count){
        filtering_function(i);
    }
    gettimeofday(&tv_end, NULL);
    long *t = malloc(sizeof(long));
    *t = (tv_end.tv_sec - tv_start.tv_sec)*1000000L + tv_end.tv_usec - tv_start.tv_usec;
    //printf("%d\n",*t);
    pthread_exit(t);
    //printf("%d\n", result_img[0][0] = 200);
}

void save_to_file(const char *path){
    FILE *file = fopen(path, "w+");
    if(!file){
        printf("Not opened %s :\n%s\n", path, strerror(errno));
    }
    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", img_size[0], img_size[1]);
    fprintf(file, "%d\n", max_value);

    for(int y = 0; y<img_size[1]; y++){
        for(int x = 0; x<img_size[0]; x++){
            fprintf(file, "%d ", result_img[x][y]);
            //printf("%d ", result_img[x][y]);
        }
        fprintf(file, "\n");
        //printf("\n");
    }
    fclose(file);
}

void free_mem(){
    for(int x=0; x<img_size[0]; x++){
        free(org_img[x]);
        free(result_img[x]);
    }
    free(org_img);
    free(result_img);

    for(int x=0; x<filter_size; x++){
        free(filter[x]);
    }
    free(filter);
}

int thread_operator(int k){
    return 0;
}
