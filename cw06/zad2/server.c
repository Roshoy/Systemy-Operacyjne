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
#include <mqueue.h>
#include "chat.h"

mqd_t **clients_friends;
struct Msg msg_buffer;
mqd_t queue_id;
char key[13];
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
void echo();

void sigint_handler(int);

int send_msg();

int main(int argc, char **argv){   
    atexit(exit_handler);
    sigset_t cl_sig_mask;
    sigfillset(&cl_sig_mask);
    sigdelset(&cl_sig_mask, SIGINT);
    signal(SIGINT, sigint_handler);
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_QUEUE_SIZE;
    attr.mq_msgsize = sizeof(Msg);
    attr.mq_curmsgs = 0;
    sprintf(key, "/%d", SERVER_SEED);
    queue_id = mq_open(key, O_RDWR | O_CREAT ,PERMISSIONS, &attr);
    if(-1 == queue_id){
        printf("%s 4\n", strerror(errno));
        exit(1);
    }
    printf("Server qid: %d\n", queue_id);
    mq_getattr(queue_id, &attr);
    clients_friends = calloc(MAX_CLIENTS, sizeof(int*));
    for(int i=0; i<MAX_CLIENTS; i++){
        clients_friends[i] = calloc(MAX_CLIENTS+1, sizeof(int));
        clients_friends[i][0] = -1;
    }
    msg_buffer.type = -1;
    while(1){
        sleep(1);
        mq_receive(queue_id, (char*)&msg_buffer, sizeof(Msg), NULL);
        if(msg_buffer.type != -1){
            if(msg_buffer.rqs_type == INIT || clients_friends[msg_buffer.sender_id-1][0] != -1){
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
                    case ECHO:
                        echo();
                        break;
                    default:
                        break;
                }
            }
            msg_buffer.type = -1;
        }
    }

    return 0;
}

void exit_handler(){
    strcpy(msg_buffer.text, "Server is closed");
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients_friends[i][0] != -1){
            msg_buffer.rqs_type = STOP;
            msg_buffer.type = 1;
            mq_send(clients_friends[i][0], (char*)&msg_buffer, sizeof(Msg),msg_buffer.type);
        }
    }
    mq_close(queue_id);
    sleep(2);
    mq_unlink(key);
    for(int i=0; i<MAX_CLIENTS; i++)free(clients_friends[i]);
    free(clients_friends);
}

void sigint_handler(int signum){
    printf("CTRL+C to kill me...\n");
    exit(0);
}

int send_msg(){
    //printf(">%s\n",msg_buffer.text);
    return mq_send(clients_friends[msg_buffer.other_id-1][0],
                  (char*)&msg_buffer, sizeof(Msg), msg_buffer.type);
}

void add_sender_stamp(char *str){
    char buffer[26];
    sprintf(buffer, "\n--Sender ID: %d", msg_buffer.sender_id);
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
    
    int add_id = 1;
    for(; add_id-1<MAX_CLIENTS && clients_friends[add_id-1][0]!=-1; add_id++); // find spot for client
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_QUEUE_SIZE;
    attr.mq_msgsize = sizeof(Msg);
    attr.mq_curmsgs = 0;
    if(add_id > MAX_CLIENTS){
        msg_buffer.other_id = -1;
        strcpy(msg_buffer.text,"Server too busy! Come back later");
        add_date_stamp(msg_buffer.text);
        msg_buffer.type = 1;
        msg_buffer.rqs_type = STOP;
        
        mqd_t temp = mq_open(msg_buffer.text, O_WRONLY, PERMISSIONS, &attr);
        mq_send(temp, (char*)&msg_buffer, sizeof(Msg),msg_buffer.type);
        mq_close(temp);
        return;
    } //brak miejsca na serwerze
    while(clients_friends[add_id-1][0] == -1){
        clients_friends[add_id-1][0] = mq_open(msg_buffer.text, O_RDWR, PERMISSIONS, &attr); // first filed in i-th row in array contains key for i-th client
        if(clients_friends[add_id-1][0] == -1)printf("Not added client: %s\n", strerror(errno));
    }
    for(int i=1; i<=MAX_CLIENTS; i++)clients_friends[add_id-1][i] = -1;//rest set at -1 as not friend
    msg_buffer.other_id = add_id;
    strcpy(msg_buffer.text, "Added successfully");
    msg_buffer.sender_id = add_id;
    add_date_stamp(msg_buffer.text);
    send_msg();
    //send new id
}

void stop_client(){
    
    if(msg_buffer.sender_id < 1 || msg_buffer.sender_id > MAX_CLIENTS || 
        clients_friends[msg_buffer.sender_id-1][0] == -1)return;
    for(int i=0; i< MAX_CLIENTS; i++){
        clients_friends[i][msg_buffer.sender_id] = -1;
    }
    strcpy(msg_buffer.text, "");
    msg_buffer.other_id = msg_buffer.sender_id;

    send_msg();
    mq_close(clients_friends[msg_buffer.sender_id-1][0]);
    clients_friends[msg_buffer.sender_id-1][0] = -1;
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
    for(int i=1; i<MAX_CLIENTS; i++)clients_friends[msg_buffer.sender_id-1][i] = -1;
    long int x;
    char *pEnd;
    x = strtol(msg_buffer.text, &pEnd, 10);
    if(x >= 0 && x < MAX_CLIENTS){
        clients_friends[msg_buffer.sender_id-1][x] = 
            clients_friends[x][0];
    }
    while((x = strtol(pEnd, &pEnd, 10)) != 0L){
        if(x >= 0 && x < MAX_CLIENTS){
        clients_friends[msg_buffer.sender_id-1][x] = 
            clients_friends[x][0];
        }
    }
    strcpy(msg_buffer.text, "Friends list set");
    add_date_stamp(msg_buffer.text);
    msg_buffer.other_id = msg_buffer.sender_id;
    send_msg();
}

void add_friend(){
    long int x;
    char *pEnd;
    x = strtol(msg_buffer.text, &pEnd, 10);
    if(x >= 0 && x < MAX_CLIENTS){
        clients_friends[msg_buffer.sender_id-1][x] = 
            clients_friends[x][0];
    }
    while((x = strtol(pEnd, &pEnd, 10)) != 0L){
        if(x >= 0 && x < MAX_CLIENTS){
        clients_friends[msg_buffer.sender_id-1][x] = 
            clients_friends[x][0];
        }
    }
    strcpy(msg_buffer.text, "Friends list edited");
    add_date_stamp(msg_buffer.text);
    msg_buffer.other_id = msg_buffer.sender_id;
    send_msg(); //send confirmation
}

void delete_friend(){
    long int x;
    char *pEnd;
    x = strtol(msg_buffer.text, &pEnd, 10);
    if(x >= 0 && x < MAX_CLIENTS && 
        clients_friends[x][0] != -1){
        clients_friends[msg_buffer.sender_id-1][x] = -1;
    }
    while((x = strtol(pEnd, &pEnd, 10)) != 0L){
        if(x >= 0 && x < MAX_CLIENTS && 
            clients_friends[x][0] != -1){
            clients_friends[msg_buffer.sender_id-1][x] = -1;
        }
    }
    strcpy(msg_buffer.text, "Friends list edited");
    add_date_stamp(msg_buffer.text);
    msg_buffer.other_id = msg_buffer.sender_id;
    send_msg();
}

void to_all(){
    add_sender_stamp(msg_buffer.text);
    add_date_stamp(msg_buffer.text);
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients_friends[i][0] != -1 && i != msg_buffer.sender_id-1){
            msg_buffer.other_id = i+1;
            send_msg();
        }
    }
}

void to_friends(){
    add_sender_stamp(msg_buffer.text);
    add_date_stamp(msg_buffer.text);
    for(int i=1; i<=MAX_CLIENTS; i++){
        if(clients_friends[msg_buffer.sender_id-1][i] != -1){
            msg_buffer.other_id = i;
            send_msg();
        }
    }
}

void to_one(){
    add_sender_stamp(msg_buffer.text);
    add_date_stamp(msg_buffer.text);
    send_msg();
}

void echo(){
    add_date_stamp(msg_buffer.text);
    msg_buffer.other_id = msg_buffer.sender_id;
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