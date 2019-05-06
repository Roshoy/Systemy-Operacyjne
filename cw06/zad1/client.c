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

FILE *stream;
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
void echo();

void bye_function();
void sigint_handler(int);

void parse_input();
int send_msg(enum RqsType);
int rcv_msg();
void input_to_upper(char *input);

int main(int argc, char **argv){    
    atexit(bye_function);
    stream = stdin;
    if(argc == 2)stream = fopen(argv[1], "r");
    sigset_t cl_sig_mask;
    sigfillset(&cl_sig_mask);
    sigdelset(&cl_sig_mask, SIGINT);
    signal(SIGINT, sigint_handler);
    key_t server_key = ftok(getenv("HOME"), SERVER_SEED);
    if(server_key == -1){
        printf("%s ftok server\n", strerror(errno));
        exit(1);
    }
    server_id = msgget(server_key, PERMISSIONS);
    if(-1 == server_id){
        printf("%s get server qid\n", strerror(errno));
        exit(1);
    }
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
    if(EOF == fscanf(stream, "%s", input))stream = stdin;
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
    }else if(strcmp(input, "ECHO") == 0){
        echo();
    }else{
        printf("Unknown command %s\n",input);
        sleep(1);
    }
}

void bye_function(){
    if(child == 0)msgctl(queue_id, IPC_RMID, NULL);
    kill(child, SIGINT);
    if(msg_buff.rqs_type != STOP){
        msg_buff.sender_id = client_id;
        send_msg(STOP);
    }
    if(stream != stdin)fclose(stream);
    if(child != 0)printf("Shutting down client\n");
}

void sigint_handler(int signum){
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
    printf(">%s\n", msg_buff.text); 
    if(msg_buff.rqs_type == STOP){
        if(msg_buff.other_id != -1)printf("Server died!\n");
        exit(0);
    }   
    return 0;
}

void init(){
    key_t key = ftok(getenv("HOME"), getpid());
    if(key == -1)printf("%s ftok client\n", strerror(errno));
    queue_id = msgget(key, IPC_CREAT | IPC_EXCL | PERMISSIONS);
    if(-1 == queue_id){
        printf("%s\nCould not open queue\n", strerror(errno));
        exit(1);
    }
    msg_buff.sender_id = queue_id;
    if(-1 == send_msg(INIT))printf("%s snd init fail\n", strerror(errno));
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
    fgets(msg_buff.text, MAX_MSG_LENGTH, stream);

    if ((strlen(msg_buff.text) > 0) && (msg_buff.text[strlen (msg_buff.text) - 1] == '\n'))
        msg_buff.text[strlen (msg_buff.text) - 1] = '\0';
    if(-1 == send_msg(FRIENDS))printf("%s 1\n", strerror(errno));
}

void add_friend(){
    // if( 0 == scanf("%d", &msg_buff.other_id)){
    //     printf("Too few arguments / not good argument\n");
    //     return;
    // }
    fgets(msg_buff.text, MAX_MSG_LENGTH, stream);

    if ((strlen(msg_buff.text) > 0) && (msg_buff.text[strlen (msg_buff.text) - 1] == '\n'))
        msg_buff.text[strlen (msg_buff.text) - 1] = '\0';
    if(-1 == send_msg(ADD))printf("%s 1\n", strerror(errno));
}

void delete_friend(){

    fgets(msg_buff.text, MAX_MSG_LENGTH, stream);

    if ((strlen(msg_buff.text) > 0) && (msg_buff.text[strlen (msg_buff.text) - 1] == '\n'))
        msg_buff.text[strlen (msg_buff.text) - 1] = '\0';
    if(-1 == send_msg(DEL))printf("%s 1\n", strerror(errno));
}

void to_all(){
    fgets(msg_buff.text, MAX_MSG_LENGTH, stream);

    if ((strlen(msg_buff.text) > 0) && (msg_buff.text[strlen (msg_buff.text) - 1] == '\n'))
        msg_buff.text[strlen (msg_buff.text) - 1] = '\0';
    if(-1 == send_msg(TOALL))printf("%s 1\n", strerror(errno));
}

void to_friends(){
    fgets(msg_buff.text, MAX_MSG_LENGTH, stream);

    if ((strlen(msg_buff.text) > 0) && (msg_buff.text[strlen (msg_buff.text) - 1] == '\n'))
        msg_buff.text[strlen (msg_buff.text) - 1] = '\0';
    if(-1 == send_msg(TOFRIENDS))printf("%s 1\n", strerror(errno));
}

void to_one(){
    if(0 == scanf("%d", &msg_buff.other_id)){
        printf("Wrong ID argument\n");
        return;
    }
    fgets(msg_buff.text, MAX_MSG_LENGTH, stream);

    if ((strlen(msg_buff.text) > 0) && (msg_buff.text[strlen (msg_buff.text) - 1] == '\n'))
        msg_buff.text[strlen (msg_buff.text) - 1] = '\0';
    
    if(-1 == send_msg(TOONE))printf("%s 1\n", strerror(errno));
}

void echo(){    
    fgets(msg_buff.text, MAX_MSG_LENGTH, stream);
    if ((strlen(msg_buff.text) > 0) && (msg_buff.text[strlen (msg_buff.text) - 1] == '\n'))
        msg_buff.text[strlen (msg_buff.text) - 1] = '\0';
    if(-1 == send_msg(ECHO))printf("%s 1\n", strerror(errno));
}