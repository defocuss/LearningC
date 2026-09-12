/*
** client.c -- client that will send data to the server
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "protocol.h" // header file with my own protocol

#define SERVERPORT 3490
#define BACKLOG 10

void conf_sockaddr(struct sockaddr_in *server_addr);
void send_to(char *to);
void aut_registry(int sockfd, struct chat_msg *registry_msg);
void get_msg(char *msg);
void send_msg(int sockfd, char *to, char *msg, struct chat_msg *chatmsg_sent);
void recv_msg(int sockfd, struct chat_msg *chatmsg_recieved);

int main(int argc, char *argv[])
{
    int sockfd;
    int len;
    ssize_t numbytes_sent, numbytes_recieved;
    char msg[MAX_MESSAGE],msg_rec[MAX_MESSAGE], to[MAX_NAME]; // MAX_MESSAGE comes from header file
    struct sockaddr_in server_addr; // info of the server_addr
    struct chat_msg chatmsg_sent, chatmsg_recieved;

    if (argc > 1) {
        strncpy(chatmsg_sent.from, argv[1], MAX_NAME);
    } else {
        fprintf(stderr, "error al ingresar argumentos");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // error check
        perror("socket");
        exit(1);
    }

    conf_sockaddr(&server_addr);

    // stablish the conection with the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    aut_registry(sockfd, &chatmsg_sent);

    while(1) {
        send_msg(sockfd, to, msg, &chatmsg_sent);
        recv_msg(sockfd, &chatmsg_recieved);
    }

    return 1;
}

void conf_sockaddr(struct sockaddr_in *server_addr) {
    server_addr -> sin_family          = AF_INET;
    server_addr -> sin_port            = htons(SERVERPORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr->sin_addr);
    memset(&(server_addr -> sin_zero),'\0',8);
}

void aut_registry(int sockfd, struct chat_msg *registry_msg){
    ssize_t nbytes_sent;

    strncpy(registry_msg -> to,  "server"       , MAX_NAME);
    strncpy(registry_msg -> msg, "registry_conf", MAX_MESSAGE);
    if ((nbytes_sent = send(sockfd, registry_msg, sizeof(struct chat_msg), 0)) == -1){
        perror("registry");
        exit(1);
    }
    printf("Registry completed succesfully, user name \"%s\"\n", registry_msg -> from);
}

void send_msg(int sockfd, char *to, char *msg, struct chat_msg *chatmsg_sent) {
    ssize_t numbytes_sent;

    send_to(to);
    get_msg(msg);
    strncpy(chatmsg_sent -> to,  to,  MAX_NAME);
    strncpy(chatmsg_sent -> msg, msg, MAX_MESSAGE);

    if ((numbytes_sent = send(sockfd, chatmsg_sent, sizeof(struct chat_msg), 0)) == -1) {
        perror("send");
        exit(1);
    }
    printf("Message sent succesfully - %zd bytes sended\n", numbytes_sent);
}

void recv_msg(int sockfd, struct chat_msg *chatmsg_recieved) {
    if (recv(sockfd, chatmsg_recieved, sizeof(struct chat_msg), 0) == -1) {
        perror("recv");
        exit(1);
    }
    printf("From %s: %s\n", chatmsg_recieved -> from, chatmsg_recieved -> msg);
}

void send_to(char *to) {
    printf("Send to: ");
    fgets(to, MAX_NAME, stdin);
    to[strcspn(to,"\n")] = '\0';
}

void get_msg(char *msg) {
    printf("Message: ");
    fgets(msg, MAX_MESSAGE, stdin);
}
