#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
//#include <signal.h>
#include <errno.h>
#include <unistd.h>

typedef struct Cart{
    int *passengers;
    int current_pass_count;
    int door_closed;
    int riding;
}Cart;

int carts_count;
int passenger_count;
int current_cart_id;
int current_pass_id;
int cart_capacity;
int rides_count;

int end_of_riding;
Cart *carts;

pthread_mutex_t mutex_cart_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cart_queue = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_pass_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_pass_queue = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_start = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_start = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_getting_to_cart = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_getting_to_cart = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_getting_off = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_getting_off = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_cart_start = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cart_full = PTHREAD_COND_INITIALIZER;


void *cart_thread(void *p);
void *passenger_thread(void *p);
void exit_fun(){
    pthread_cond_destroy(&cond_cart_queue);
    pthread_cond_destroy(&cond_cart_full);
    pthread_cond_destroy(&cond_getting_off);
    pthread_cond_destroy(&cond_getting_to_cart);
    pthread_cond_destroy(&cond_pass_queue);
    pthread_cond_destroy(&cond_start);

    pthread_mutex_destroy(&mutex_cart_queue);
    pthread_mutex_destroy(&mutex_cart_start);
    pthread_mutex_destroy(&mutex_getting_off);
    pthread_mutex_destroy(&mutex_getting_to_cart);
    pthread_mutex_destroy(&mutex_pass_queue);
    pthread_mutex_destroy(&mutex_start);
}

int main(int argc, char **argv){
    srand(time(NULL));
    atexit(exit_fun);
    if(argc < 5){
        printf("Za mało argumentów\n");
        exit(1);
    }
    
    passenger_count = atoi(argv[1]);
    carts_count = atoi(argv[2]);   
    cart_capacity = atoi(argv[3]);
    rides_count = atoi(argv[4]);

    if(carts_count * cart_capacity > passenger_count){
        printf("Za malo pasazerow\n");
        exit(2);
    }
    

    end_of_riding = 0;
    current_cart_id = 0;
    carts = calloc(carts_count, sizeof(Cart));
    for(int i=0; i<carts_count; i++){
        carts[i].passengers = calloc(cart_capacity, sizeof(int));
        carts[i].current_pass_count = 0;
        carts[i].door_closed = 0;
        carts[i].riding = 1;
    }

    pthread_t *cart_threads = calloc(carts_count, sizeof(pthread_t));
    pthread_t *passenger_threads = calloc(passenger_count, sizeof(pthread_t));
    int *ids = calloc(carts_count, sizeof(int));
    int *p_ids = calloc(passenger_count, sizeof(int));
    for(int i=0; i<carts_count; i++){
        ids[i] = i;
        pthread_create(cart_threads+i, NULL, cart_thread, ids+i);
    }
    for(int i=0; i<passenger_count; i++){
        p_ids[i] = i;
        pthread_create(passenger_threads+i, NULL, passenger_thread, p_ids+i);
        pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    }
    
    for(int i=0; i<carts_count; i++){
        pthread_join(cart_threads[i], NULL);
    }
    end_of_riding = 1;
    for(int i=0; i<passenger_count; i++)pthread_cond_broadcast(&cond_getting_to_cart);
    for(int i=0; i<passenger_count; i++){
        p_ids[i] = i;
        pthread_cond_broadcast(&cond_getting_to_cart);
        pthread_join(passenger_threads[i], NULL);
    }
    free(cart_threads);
    free(passenger_threads);
    free(p_ids);
    free(ids);
    for(int i=0; i<carts_count; i++)free(carts[i].passengers);
    free(carts);
    return 0;
}

void *cart_thread(void *p){
    struct timeval tv_start, tv_end;
    int id = (*(int*)p);
    for(int j=0; j<rides_count; j++){        
        //wchodzenie na przód kolejki
        pthread_mutex_lock(&mutex_cart_queue);
        while(current_cart_id != id){
            pthread_cond_wait(&cond_cart_queue, &mutex_cart_queue);
        }
        printf("Wagonik %d otwiera drzwi\n", id);
        //wysiadanie
        carts[id].door_closed = 0;//otwieram drzwi
        carts[id].riding = 2; //wysiadanie
        pthread_cond_broadcast(&cond_getting_off);
        pthread_mutex_lock(&mutex_getting_off);
        while(carts[id].current_pass_count != 0){
            pthread_cond_wait(&cond_getting_off, &mutex_getting_off);
        }
        carts[id].riding = 0; //ustawienie wsiadania
        pthread_cond_broadcast(&cond_getting_to_cart);
        pthread_mutex_unlock(&mutex_getting_off);

        /////////////////////////////

        pthread_mutex_lock(&mutex_getting_to_cart);
        while(carts[id].current_pass_count < cart_capacity)
        {
            pthread_cond_wait(&cond_cart_full, &mutex_getting_to_cart);
        }
        carts[id].riding = 1;
        pthread_cond_broadcast(&cond_start);
        pthread_mutex_unlock(&mutex_getting_to_cart);
            //////////////////////////////////////////////////
        pthread_mutex_lock(&mutex_cart_start);
        while(carts[id].door_closed == 0){
            pthread_cond_wait(&cond_start, &mutex_cart_start);
        }
        pthread_mutex_unlock(&mutex_cart_start);
        printf("Wagonik %d zamniety\n", id);
        current_cart_id = (current_cart_id + 1) % carts_count; //nastepny wagon na stacji
        
        pthread_cond_broadcast(&cond_cart_queue);
        pthread_mutex_unlock(&mutex_cart_queue);
        ////////////////////////////////////

        //jazda        
        printf("Wagonik %d jedzie po raz %d\n", id, j+1);        
        for(long i = 0; i<3000000; i++);        
        printf("Wagonik dojechał i czeka na podjechanie do stacji\n");
        ///////
    }

    pthread_mutex_lock(&mutex_cart_queue);
    while(current_cart_id != id){
        pthread_cond_wait(&cond_cart_queue, &mutex_cart_queue);
    }        
    //wysiadanie
    carts[id].door_closed = 0;//otwieram drzwi
    carts[id].riding = 2; //wysiadanie
    printf("Wagonik %d chce wypuszczać\n", id);
    pthread_cond_broadcast(&cond_getting_off);
    pthread_mutex_lock(&mutex_getting_off);
    while(carts[id].current_pass_count != 0){
        pthread_cond_wait(&cond_getting_off, &mutex_getting_off);
    }
    printf("Wysiedli z %d\n", id);
    carts[id].riding = 0; //ustawienie wsiadania
    pthread_cond_broadcast(&cond_getting_off);
    pthread_mutex_unlock(&mutex_getting_off);
    current_cart_id = (current_cart_id + 1) % carts_count;
    pthread_cond_broadcast(&cond_cart_queue);
    pthread_mutex_unlock(&mutex_cart_queue);
    printf("Wagonik %d się zakończył\n", id);

    
    pthread_exit(&id);
}

void *passenger_thread(void *p){
    int id = (*(int*)p);
    int im_in_current_cart = -1; // nie jest w żadnym
    while(1){
 
        pthread_mutex_lock(&mutex_pass_queue);
        while(current_pass_id != id){
            pthread_cond_wait(&cond_pass_queue, &mutex_pass_queue);
        }
        
        pthread_mutex_lock(&mutex_getting_to_cart);
        while(carts[current_cart_id].current_pass_count >= cart_capacity || carts[current_cart_id].riding != 0){
            pthread_cond_wait(&cond_getting_to_cart, &mutex_getting_to_cart);            
        }
        if(end_of_riding){
            current_pass_id = (current_pass_id + 1) % passenger_count;
            pthread_mutex_unlock(&mutex_getting_to_cart);
            pthread_cond_broadcast(&cond_pass_queue);
            pthread_mutex_unlock(&mutex_pass_queue);
            printf("Pasazer %d się zakończył\n", id);
            return NULL;
        }
        carts[current_cart_id].passengers[carts[current_cart_id].current_pass_count++] = id;
        im_in_current_cart = current_cart_id;   
        printf("Pasazer %d wsiadl do %d, jest tam %d pasazerow\n", id, im_in_current_cart, carts[current_cart_id].current_pass_count);

        if(carts[im_in_current_cart].current_pass_count != cart_capacity)
            pthread_cond_broadcast(&cond_getting_to_cart);
        else
            pthread_cond_broadcast(&cond_cart_full);
        pthread_mutex_unlock(&mutex_getting_to_cart);

            //////////////////////////////////////////////////
        
        current_pass_id = (current_pass_id + 1) % passenger_count; //nastepny ustawia sie w kolejce

        pthread_cond_broadcast(&cond_pass_queue);
        pthread_mutex_unlock(&mutex_pass_queue);
        ////////////////////////////////////

        pthread_mutex_lock(&mutex_start);
        while(carts[im_in_current_cart].current_pass_count != cart_capacity){
            pthread_cond_wait(&cond_start, &mutex_start);
        }
        if(carts[im_in_current_cart].door_closed == 0){
            carts[im_in_current_cart].door_closed = 1;
            pthread_cond_broadcast(&cond_start);
            printf("Pasazer %d wcisną start w wagoniku %d\n", id, im_in_current_cart);
        }
        pthread_mutex_unlock(&mutex_start);

        //////////////////////////////////////////
        ///wysiadanie
        pthread_mutex_lock(&mutex_getting_off);
        while(carts[im_in_current_cart].riding != 2){
            pthread_cond_wait(&cond_getting_off, &mutex_getting_off);
        }
        printf("Pasazer %d wysiada z %d\n", id, im_in_current_cart);
        carts[im_in_current_cart].current_pass_count--;
        im_in_current_cart = -1;        
        pthread_cond_broadcast(&cond_getting_off);
        pthread_mutex_unlock(&mutex_getting_off);      
        ////
    }
    printf("Pasazer %d się zakończył\n", id);
    pthread_exit(&id);
}


/*  
           /\      
          /  \
     -----    -----
       -        -
         -    -         
        /  --  \
       / -    - \
▄ ▀ █ ░
              ▄             
             █░█             
            █░░░█              
           █░░░░░█            
   ▀█▀▀▀▀▀▀░░░░░░░▀▀▀▀▀▀█▀     
     ▀▀▄▄░░░░░░░░░░░▄▄▀▀      
         ▀█░░░░░░░█▀           
         █░░░▄▀▄░░░█           
        █░░▄▀   ▀▄░░█           
       █░▄▀       ▀▄░█          
       █▀           ▀█          
          

*/