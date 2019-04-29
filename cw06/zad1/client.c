#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <errno.h>
#include "chat.h"

int server_id;
int queue_id;
int client_id;
struct Msg msg_buff;
void init();
void bye_function();

int main(int argc, char **argv){    
    atexit(bye_function);
    key_t server_key = ftok(getenv("HOME"), SERVER_SEED);

    printf("Key: ");
    printf("%s 1\n", strerror(errno));
    server_id = msgget(server_key, PERMISSIONS);
    if(-1 == server_id)printf("%s 3\n", strerror(errno));
    init();
    //while(1);
    exit(0);
}

void bye_function(){
    msgctl(queue_id, IPC_RMID, NULL);
    msg_buff.sender_id = client_id;
    msg_buff.type = STOP%4;
    msg_buff.rqs_type = STOP;
    msgsnd(server_id, &msg_buff, sizeof(Msg) - sizeof(long),0);
}

void init(){
    key_t key = ftok(getenv("HOME"), getpid());
    queue_id = msgget(key, IPC_CREAT | IPC_EXCL | PERMISSIONS);
    if(-1 == queue_id)printf("%s 1\n", strerror(errno));
    msg_buff.sender_id = queue_id;
    printf("%d\n",msg_buff.type = INIT%4);
    msg_buff.rqs_type = INIT;
    printf("server_id: %d\n", server_id);
    if(-1 == msgsnd(server_id, &msg_buff, sizeof(Msg)-sizeof(long), 0))printf("%s 1\n", strerror(errno));
    printf("client stopped probably\n");
    msgrcv(queue_id, &msg_buff, sizeof(Msg)-sizeof(long), 0, 0);
    printf("%s\n", msg_buff.text);    
}