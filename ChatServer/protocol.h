/*
 ** protocol.h -- file that saves the protocol structs
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_NAME 32
#define MAX_MESSAGE 1024

struct chat_msg {
    char from[MAX_NAME];
    char to[MAX_NAME];
    char msg[MAX_MESSAGE];
};

struct conn_client {
    int  sockfd;
    char name[MAX_NAME];
    int  status;
};

#endif
