#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include "chat.h"

int child;
int server_id;
int queue_id;
int client_id;
struct Msg msg_buff;
void init();
void stop();
void list();
void friends();
void add_friend();
void delete_friend();
void to_all();
void to_friends();
void to_one();

void bye_function();
void sigint_handler(int);

void parse_input();
int send_msg(enum RqsType);
int rcv_msg();
void input_to_upper(char *input);

int main(int argc, char **argv){    
    atexit(bye_function);
    sigset_t cl_sig_mask;
    sigfillset(&cl_sig_mask);
    sigdelset(&cl_sig_mask, SIGINT);
    signal(SIGINT, sigint_handler);
    key_t server_key = ftok(getenv("HOME"), SERVER_SEED);
    printf("%s 1\n", strerror(errno));
    server_id = msgget(server_key, PERMISSIONS);
    if(-1 == server_id)printf("%s 3\n", strerror(errno));
    init();
    child = fork();
    if(child != 0){
        while(1){
            parse_input();
        }
    }else{
        while(1){
            rcv_msg();
        }
    }
    exit(0);
}

void input_to_upper(char *input){
    int len = strlen(input);
    for(int i=0; i<len; i++){
        input[i] = toupper(input[i]);
    }
}

void parse_input(){
    char input[20];
    scanf("%s", input);
    input_to_upper(input);
    if(strcmp(input, "STOP") == 0){
        exit(0);
    }else if(strcmp(input, "LIST") == 0){
        list();
    }else if(strcmp(input, "FRIENDS") == 0){
        friends();
    }else if(strcmp(input, "2ALL") == 0){
        to_all();
    }else if(strcmp(input, "2FRIENDS") == 0){
        to_friends();
    }else if(strcmp(input, "2ONE") == 0){
        to_one();
    }else if(strcmp(input, "ADD") == 0){
        add_friend();
    }else if(strcmp(input, "DELETE") == 0){
        delete_friend();
    }else{
        printf("Unknown command\n");
    }
}

void bye_function(){
    msgctl(queue_id, IPC_RMID, NULL);
    kill(child, SIGINT);
    if(msg_buff.rqs_type != STOP){
        msg_buff.sender_id = client_id;
        send_msg(STOP);
    }
}

void sigint_handler(int signum){
    printf("CTRL+C to kill me...\n");
    if(child == 0)kill(getppid(), SIGINT);
    else {
        kill(child, SIGINT);
    }
    
    exit(0);
}

int send_msg(enum RqsType type){
    msg_buff.type = type % 4;
    msg_buff.rqs_type = type;
    return msgsnd(server_id, &msg_buff, sizeof(Msg)-sizeof(long),0);
}

int rcv_msg(){
    if(-1 == msgrcv(queue_id, &msg_buff, sizeof(Msg)-sizeof(long), 0, 0))return -1;
    printf("%s\n", msg_buff.text); 
    if(msg_buff.rqs_type == STOP){
        printf("Server died!\nShutting down client\n");
        exit(0);
    }   
    return 0;
}

void init(){
    key_t key = ftok(getenv("HOME"), getpid());
    queue_id = msgget(key, IPC_CREAT | IPC_EXCL | PERMISSIONS);
    if(-1 == queue_id)printf("%s 1\n", strerror(errno));
    msg_buff.sender_id = queue_id;
    printf("server_id: %d\n", server_id);
    if(-1 == send_msg(INIT))printf("%s 1\n", strerror(errno));
    rcv_msg();
    client_id = msg_buff.other_id;
    msg_buff.sender_id = client_id;
       
}

void stop(){
    strcpy(msg_buff.text, "");
    if(-1 == send_msg(STOP))printf("%s 1\n", strerror(errno));
}

void list(){
    strcpy(msg_buff.text, "");
    if(-1 == send_msg(LIST))printf("%s 1\n", strerror(errno));
}

void friends(){
    strcpy(msg_buff.text, "");
    if(-1 == send_msg(FRIENDS))printf("%s 1\n", strerror(errno));
}

void add_friend(){
    strcpy(msg_buff.text, "");
    if( 0 == scanf("%d", &msg_buff.other_id)){
        printf("Too few arguments / not good argument\n");
        return;
    }
    if(-1 == send_msg(ADD))printf("%s 1\n", strerror(errno));
}

void delete_friend(){
    strcpy(msg_buff.text, "");
    if( 0 == scanf("%d", &msg_buff.other_id)){
        printf("Too few arguments / not good argument\n");
        return;
    }
    if(-1 == send_msg(DEL))printf("%s 1\n", strerror(errno));
}

void to_all(){
    while(0==scanf("\"%s\"", msg_buff.text) && 0==scanf("%s", msg_buff.text))printf("No message given");
    
    if(-1 == send_msg(TOALL))printf("%s 1\n", strerror(errno));
}

void to_friends(){
    scanf("\"%s\"", msg_buff.text);
    if(-1 == send_msg(TOFRIENDS))printf("%s 1\n", strerror(errno));
}

void to_one(){
    if(2 > scanf("%d \"%s\"", &msg_buff.other_id, msg_buff.text)){
        printf("Wrong arguments for 2ONE command\n");
        return;
    };
    if(-1 == send_msg(TOONE))printf("%s 1\n", strerror(errno));
}