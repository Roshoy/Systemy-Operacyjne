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
    double **filter;
    unsigned long goal_sum = 1;
    while(goal_sum*2 < 256 * c * c){
        goal_sum *= 2;
    }
    unsigned long current_sum = 0;
    int rand_rad = 256;
    filter = calloc(c, sizeof(double*));
    for(int i=0; i<c; i++){
        filter[i] = calloc(c, sizeof(double));
    }
    FILE *file = fopen(path, "w+");
    fprintf(file, "%d\n", c);

    for(int y = 0; y<c; y++){
        for(int x = 0; x<c; x++){
            if(y == c-1 && x == c-1)filter[x][y] = goal_sum - current_sum;
            else filter[x][y] = rand()%(rand_rad < goal_sum - current_sum ? rand_rad : goal_sum - current_sum);
            current_sum += filter[x][y];
            fprintf(file, "%lf ", filter[x][y]/goal_sum);
            //printf("%f ", filter[x][y]/goal_sum);
        }
        fprintf(file, "\n");
        //printf("\n");
    }
    fclose(file);

    for(int i=0; i<c; i++){
        free(filter[i]);
    }
    free(filter);
    return 0;
}