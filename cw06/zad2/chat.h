#ifndef CHAT_H
#define CHAT_H

#define MAX_MSG_LENGTH 1000
#define SERVER_SEED 3
#define PERMISSIONS 0666
#define MAX_CLIENTS 10
#define MAX_QUEUE_SIZE 10

enum RqsType{
    STOP = 3,
    LIST = 2,
    FRIENDS = 6,
    INIT = 1,
    ADD = 5,
    DEL = 9,
    TOALL = 13,
    TOFRIENDS = 17,
    TOONE = 21,
    ECHO = 25
}RqsType;

struct Msg{
    int type;
    enum RqsType rqs_type;
    char text[MAX_MSG_LENGTH];
    int sender_id;
    int other_id;
}Msg;

#endif  