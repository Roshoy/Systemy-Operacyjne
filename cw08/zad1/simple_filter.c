#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

int main(int argc, char **argv){
    const char *path = "filter";
    srand(time(NULL));
    int c = 10;
    if(argc >= 2)c = atoi(argv[1]);
    long **filter;
    unsigned long goal_sum = 1;
    unsigned long current_sum = 0;
    int rand_rad = 16;
    filter = calloc(c, sizeof(long*));
    for(int i=0; i<c; i++){
        filter[i] = calloc(c, sizeof(long));
    }
    FILE *file = fopen(path, "w+");
    fprintf(file, "%d\n", c);

    for(int y = 0; y<c; y++){
        for(int x = 0; x<c; x++){
            if(y == c-1 && x == c-1)filter[x][y] = goal_sum - current_sum;
            else filter[x][y] = rand()%(rand_rad-current_sum) - (rand_rad-current_sum)/2;
            current_sum += filter[x][y];
            fprintf(file, "%d ", filter[x][y]);
            printf("%d ", filter[x][y]);
        }
        fprintf(file, "\n");
        printf("\n");
    }
    fclose(file);

    for(int i=0; i<c; i++){
        free(filter[i]);
    }
    free(filter);
    return 0;
}