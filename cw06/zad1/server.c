#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/types.h> 
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
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
void friends();
void add_friend();
void delete_friend();
void to_all();
void to_friends();
void to_one();

void sigint_handler(int);

int send_msg(){
    return msgsnd(clients_friends[msg_buffer.other_id][0],
                  &msg_buffer, sizeof(Msg)-sizeof(long),0);
}

int main(int argc, char **argv){   
    atexit(exit_handler);
    sigset_t cl_sig_mask;
    sigfillset(&cl_sig_mask);
    sigdelset(&cl_sig_mask, SIGINT);
    signal(SIGINT, sigint_handler);
    key_t key = ftok(getenv("HOME"), SERVER_SEED);
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
                    stop_client();
                    break;
                case INIT:
                    add_client();
                    break;
                case FRIENDS:
                    friends();
                    break;
                case ADD:
                    add_friend();
                    break;
                case DEL:
                    delete_friend();
                    break;
                case LIST:
                    list();
                    break;
                case TOFRIENDS:
                    to_friends();
                    break;
                case TOALL:
                    to_all();
                    break;
                case TOONE:
                    to_one();
                    break;
                default:
                    printf("Not known type!");
                    break;
            }
            msg_buffer.type = -1;
        }
    }

    return 0;
}

void exit_handler(){
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients_friends[i][0] != -1){
            msg_buffer.rqs_type = STOP;
            msg_buffer.type = 3;
            msgsnd(clients_friends[i][0], &msg_buffer, sizeof(Msg)-sizeof(long),0);
        }
    }
    msgctl(queue_id, IPC_RMID, NULL);
}

void sigint_handler(int signum){
    printf("CTRL+C to kill me...\n");
    exit(0);
}

void add_sender_stamp(char *str){
    char buffer[26];
    sprintf(buffer, "\n--Sender ID: %d", clients_friends[msg_buffer.sender_id][0]);
    strcat(str, buffer);
}

void add_date_stamp(char* str){
    char text[23];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(text, sizeof(text)-1, "--%02d-%02m-%04Y %02H:%02M:%02S", t);
    strcat(str, "\n");
    strcat(str, text);
}

void add_client(){
    int client_key = msg_buffer.sender_id; // in init sender_id contains client queue key
    int add_id = 0;
    for(; add_id<MAX_CLIENTS && clients_friends[add_id][0]!=-1; add_id++); // find spot for client
    if(add_id >= MAX_CLIENTS){
        msg_buffer.other_id = -1;
        strcpy(msg_buffer.text,"Server too busy! Come back later");
        add_date_stamp(msg_buffer.text);
        msgsnd(client_key, &msg_buffer, sizeof(Msg)-sizeof(long),0);
        return;
    } //brak miejsca na serwerze
    clients_friends[add_id][0] = client_key; // first filed in i-th row in array contains key for i-th client
    for(int i=1; i<=MAX_CLIENTS; i++)clients_friends[add_id][i] = -1;//rest set at -1 as not friend
    
    msg_buffer.other_id = add_id;
    add_date_stamp(msg_buffer.text);
    //msgsnd(client_key, &msg_buffer, sizeof(Msg)-sizeof(long),0);
    send_msg();
    //send new id
}

void stop_client(){
    if(msg_buffer.sender_id < 0 || msg_buffer.sender_id >= MAX_CLIENTS || 
        clients_friends[msg_buffer.sender_id][0] == -1)return;
    for(int i=1; i<= MAX_CLIENTS; i++){
        clients_friends[i][msg_buffer.sender_id] = -1;
    }
    clients_friends[msg_buffer.sender_id][0] = -1;
}

void list(){
    char one_id[14]; //holding single id's of clients
    strcat(msg_buffer.text,"Active clients:");
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients_friends[i][0] == -1)continue; // is not active
        sprintf(one_id, "- %d", i); // int to string of active client
        strcat(msg_buffer.text, "\n");
        strcat(msg_buffer.text, one_id);
    }
    add_date_stamp(msg_buffer.text);
    msg_buffer.other_id = msg_buffer.sender_id;
    send_msg();
    //send result    
}

void friends(){
    int client_id = msg_buffer.sender_id;
    char one_id[14]; //holding single id's of clients
    strcat(msg_buffer.text,"Your friends:");
    for(int i=1; i<=MAX_CLIENTS; i++){ // MAX_CLIENTS + 1 is the length of row
        if(clients_friends[client_id][i] == -1)continue; // is not friend
        sprintf(one_id, "- %d", i-1); 
        strcat(msg_buffer.text, "\n");
        strcat(msg_buffer.text, one_id);
    }
    add_date_stamp(msg_buffer.text);
    msg_buffer.other_id = msg_buffer.sender_id;
    send_msg();
}

void add_friend(){
    if(msg_buffer.other_id >= 0 && msg_buffer.other_id < MAX_CLIENTS && 
        clients_friends[msg_buffer.other_id][0] != -1){
        clients_friends[msg_buffer.sender_id][msg_buffer.other_id+1] = 
            clients_friends[msg_buffer.other_id][0];
        strcat(msg_buffer.text, "Friend added!");
    }else{
        strcat(msg_buffer.text, "Client not active or wrong ID!");
    }
    msg_buffer.other_id = msg_buffer.sender_id;
    send_msg(); //send confirmation
}

void delete_friend(){
    if(msg_buffer.other_id >= 0 && msg_buffer.other_id < MAX_CLIENTS && 
        clients_friends[msg_buffer.other_id][0] != -1){
        clients_friends[msg_buffer.sender_id][msg_buffer.other_id+1] = -1;
        strcat(msg_buffer.text, "Friend deleted :(");
    }else{
        strcat(msg_buffer.text, "Client not active or wrong ID!");
    }
    msg_buffer.other_id = msg_buffer.sender_id;
    send_msg();
}

void to_all(){
    add_sender_stamp(msg_buffer.text);
    add_date_stamp(msg_buffer.text);
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients_friends[i][0] != -1 && i != msg_buffer.sender_id){
            msg_buffer.other_id = i;
            send_msg();
        }
    }
}

void to_friends(){
    add_sender_stamp(msg_buffer.text);
    add_date_stamp(msg_buffer.text);
    for(int i=1; i<=MAX_CLIENTS; i++){
        if(clients_friends[msg_buffer.sender_id][i] != -1){
            msg_buffer.other_id = i-1;
            send_msg();
        }
    }
}

void to_one(){
    add_sender_stamp(msg_buffer.text);
    add_date_stamp(msg_buffer.text);
    send_msg();
}

void rise_errno(){
    perror(NULL);
    exit(1);
}

void rise_error(char *errmsg){
    perror(errmsg);
    exit(1);
}