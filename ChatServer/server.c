/* 
** server.c -- in charge of passing messages
*/

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <stdlib.h>
#include "protocol.h" // my own protocol

#define MYPORT      3490
#define MAXBYTES    100
#define BACKLOG     10
#define MAXCLIENTS  100
#define TIMEOUTSECS 500

void conf_sockaddr(struct sockaddr_in *server_addr);

int main()
{
    int sockfd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    struct conn_client clients[MAXCLIENTS]; // 100 clients
    struct pollfd fds[MAXCLIENTS];
    int sin_size;
    int len, poll_structures, i, j, encontrado;
    int listen_status;
    ssize_t bytes_recieved;
    int client_count = 0;
    int optval = 1;
    struct chat_msg incoming_msg;


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    conf_sockaddr(&server_addr);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // initialize the fds
    for (i=0; i<MAXCLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }

    fds[0].fd = sockfd;
    client_count = 1;

    printf("Servidor de chat iniciado en el puerto %d...\n", MYPORT);

    while (1) {
        poll_structures = poll(fds, client_count, TIMEOUTSECS);
        if (poll_structures == -1) {perror("poll"); continue;}

        if (poll_structures > 0) {
            for (i=0; i<client_count; i++) {
                
                // nothing to do with the descriptor
                if (!(fds[i].revents & POLLIN)) {
                    continue;
                }

                // there is data to read
                if (i==0) {
                    // recive the data
                    sin_size = sizeof(struct sockaddr_in);
                    new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &sin_size);
                    if (new_fd == -1) { perror("accept"); continue;} // error cheking

                    // check that server is not full
                    if (client_count  < MAXCLIENTS) {
                        fds[client_count].fd = new_fd;  // save the new sock descriptor
                        fds[client_count].events = POLLIN;
                        clients[client_count].sockfd = new_fd;
                        strcpy(clients[client_count].name, "");
                        client_count++;
                        printf("A new client has connected, number of clients: %d\n", client_count - 1);
                    } else {
                        printf("server full\n");
                        close(new_fd);
                    }
                } else {
                    bytes_recieved = recv(fds[i].fd, &incoming_msg, sizeof(struct chat_msg), 0);

                    if (bytes_recieved <= 0) {
                        close(fds[i].fd);
                        client_count--;
                        fds[i] = fds[client_count];
                        clients[i] = clients[client_count];
                        fds[client_count].fd = -1; // to clean the free space
                        i--;
                        printf("failed to recieve message\n");
                    } else {
                        // automatic name configuration
                        if (strcmp(incoming_msg.to, "server") == 0) {
                            strncpy(clients[i].name, incoming_msg.from, MAX_NAME);
                            continue;
                        }

                        // search for the reciever
                        encontrado = 0;
                        printf("Trying to find: (%s)\n", incoming_msg.to);
                        for (j=0; j < client_count; j++) {
                            if (strcmp(clients[j].name, incoming_msg.to) == 0) {
                                // send the message to the new client
                                send(clients[j].sockfd, &incoming_msg, sizeof(struct chat_msg), 0);
                                encontrado = 1;
                                break;
                            }
                        }
                        if (encontrado == 0){
                            printf("Reciever not found\n");
                        }
                    }
                }
            }
        }
    }
    return 1;
}

// set initial parameters for server struct
void conf_sockaddr(struct sockaddr_in *server_addr)
{
    server_addr -> sin_family      = AF_INET; // for ipv4
    server_addr -> sin_port        = htons(MYPORT);
    server_addr -> sin_addr.s_addr = htonl(INADDR_ANY); // with this i select my own ip
    memset(&(server_addr -> sin_zero),'\0', 8);
}
