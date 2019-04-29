#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/types.h> 
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "chat.h"

#define MAX_CLIENTS 100

int **clients_friends;
struct Msg msg_buffer;
int queue_id;
void rise_errno();
void rise_error(char *);
void exit_handler();
void add_sender_stamp(char *);
void add_date_stamp(char *);
void add_client();
void stop_client();
void list();

int main(int argc, char **argv){   
    printf("Key2: \n"); 
    atexit(exit_handler);
    key_t key = ftok(getenv("HOME"), SERVER_SEED);
    printf("Key3: \n");
    printf("%s 1\n", strerror(errno));
    queue_id = msgget(key, IPC_CREAT |PERMISSIONS);
    if(-1 == queue_id)printf("%s 4\n", strerror(errno));;
    clients_friends = calloc(MAX_CLIENTS, sizeof(int*));
    for(int i=0; i<MAX_CLIENTS; i++){
        clients_friends[i] = calloc(MAX_CLIENTS+1, sizeof(int));
        clients_friends[i][0] = -1;
    }
    while(1){
        msgrcv(queue_id, &msg_buffer, sizeof(Msg) - sizeof(long), -100, IPC_NOWAIT);
        if(msg_buffer.type != -1){
            switch(msg_buffer.rqs_type){
                case STOP:
                    printf("client sssstopped probably\n");
                    stop_client();
                    printf("client stopped probably\n");
                    break;
                case INIT:
                    printf("client stopped probably\n");
                    add_client();
                    printf("client initiated probably\n");
                    break;
                default:
                    //printf("Not known type!");
                    break;
            }
            msg_buffer.type = -1;
        }
    }

    return 0;
}

void exit_handler(){
    msgctl(queue_id, IPC_RMID, NULL);
    perror(NULL);
}

void add_client(){
    int client_key = msg_buffer.sender_id; // in init sender_id contains client queue key
    int add_id = 0;
    printf("client stopped probably1\n");
    for(; add_id<MAX_CLIENTS && clients_friends[add_id][0]!=-1; add_id++); // find spot for client
    printf("client stopped probably2\n");
    if(add_id >= MAX_CLIENTS)return; //brak miejsca na serwerze
    clients_friends[add_id][0] = client_key; // first filed in i-th row in array contains key for i-th client
    printf("client stopped probably3\n");
    for(int i=1; i<=MAX_CLIENTS; i++)clients_friends[add_id][i] = -1;//rest set at -1 as not friend
    char *result = calloc(MAX_MSG_LENGTH,1);
    strcat(result, "Client added");
    printf("client stopped probably4\n");
    add_date_stamp(result);
    printf("client stopped probably5\n");
    msg_buffer.other_id = add_id;
    strcpy(msg_buffer.text, result);
    printf("client stopped probably6\n");
    msgsnd(client_key, &msg_buffer, sizeof(Msg)-sizeof(long),0);
    msg_buffer.type = -1;
    //send new id
}

void add_date_stamp(char* str){
    char text[21];
    printf("5\n");
    time_t now = time(NULL);
    printf("4\n");
    struct tm *t = localtime(&now);
    printf("3\n");
    strftime(text, sizeof(text)-1, "%02d-%02m-%04Y %02H:%02M:%02S", t);
    printf("2\n");
    strcat(str, "\n");
    printf("1\n");
    strcat(str, text);
}

void stop_client(){
    int delete_id = msg_buffer.sender_id;
    clients_friends[delete_id][0] = -1;
}

void list(){
    char one_id[12]; //holding single id's of clients
    char *result = calloc(MAX_MSG_LENGTH, 1);
    strcat(result,"Active clients:");
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients_friends[i][0] == -1)continue; // is not active
        sprintf(one_id, "%d", clients_friends[i][0]); // int to string of active client
        strcat(result, "\n");
        strcat(result, one_id);
    }
    add_date_stamp(result);
    //send result    
}

void friends(){
    int client_id = msg_buffer.sender_id;
    char one_id[12]; //holding single id's of clients
    char *result = calloc(MAX_MSG_LENGTH, 1);
    strcat(result,"Your friends:");
    for(int i=1; i<=MAX_CLIENTS; i++){ // MAX_CLIENTS + 1 is the length of row
        if(clients_friends[client_id][i] == -1)continue; // is not friend
        sprintf(one_id, "%d", clients_friends[client_id][i]); 
        strcat(result, "\n");
        strcat(result, one_id);
    }
    add_date_stamp(result);
}

void add_friend(){
    clients_friends[msg_buffer.sender_id][msg_buffer.other_id+1] = 
        clients_friends[msg_buffer.other_id][0];
    //send
    //send confirmation
}

void delete_friend(){
    clients_friends[msg_buffer.sender_id][msg_buffer.other_id+1] = -1;
}

void rise_errno(){
    perror(NULL);
    exit(1);
}

void rise_error(char *errmsg){
    perror(errmsg);
    exit(1);
}