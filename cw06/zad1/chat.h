#ifndef CHAT_H
#define CHAT_H

#define MAX_MSG_LENGTH 1000
#define SERVER_SEED 2
#define PERMISSIONS 0666

enum RqsType{
    STOP = 1,
    LIST = 2,
    FRIENDS = 6,
    INIT = 3,
    ADD = 7,
    DEL = 11,
    TOALL = 15,
    TOFRIENDS = 19,
    TOONE = 23
}RqsType;

struct Msg{
    long type;
    enum RqsType rqs_type;
    char text[MAX_MSG_LENGTH];
    int sender_id;
    int other_id;
}Msg;


#endif  