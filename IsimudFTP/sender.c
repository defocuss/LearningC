#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include "protocol.h"

void conf_sockaddr(struct sockaddr_in *to_addr, const char *ip);
static FILE * send_header(const char *f_path, int sockfd, struct header *h);
static int send_all(int client, const void *buf, size_t len);
static char *discover_receiver(void);

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in to_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    char *ip = discover_receiver();
    if (ip == NULL) {
        fprintf(stderr, "No se encontró ningún receiver\n");
        exit(1);
    }
    conf_sockaddr(&to_addr, ip);
    free(ip);

    if (connect(sockfd, (struct sockaddr *)&to_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    struct header h = {0};
    FILE *fptr = send_header("video.mp4", sockfd, &h);

    unsigned char chunk[CHUNK];
    ssize_t n;

    while ((n = fread(chunk, 1, CHUNK, fptr)) > 0 ) {
        send_all(sockfd, chunk, n);
    }
    close(sockfd);
    printf("All bytes sended correctly");

    fclose(fptr);

    return 0;

}

// Function to discover the receiver in the network using UDP broadcast
static char *discover_receiver(void) {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1) { perror("socket"); return NULL; }

    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));
    struct timeval tv = { .tv_sec = 2 };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in bc = {0};
    bc.sin_family = AF_INET;
    bc.sin_port = htons(DISCOVERY_PORT);
    bc.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    sendto(s, DISCOVERY_MSG, strlen(DISCOVERY_MSG), 0,
           (struct sockaddr *)&bc, sizeof(bc));

    char buf[64];
    struct sockaddr_in from;
    socklen_t len = sizeof(from);
    ssize_t n = recvfrom(s, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr *)&from, &len);
    close(s);

    if (n <= 0) return NULL;
    buf[n] = '\0';
    if (strcmp(buf, DISCOVERY_REPLY) != 0) return NULL;

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &from.sin_addr, ip, sizeof(ip));
    return strdup(ip);
}

// Function to open the connection and send the header
static FILE * send_header(const char *f_path, int sockfd, struct header *h) {
    FILE *fptr = fopen(f_path, "rb");

    if (fptr == NULL) {
        perror("fopen");
        exit(1);
    }

    strncpy(h -> name, f_path, MAX_NAME - 1);
    h -> name_size = strlen(h -> name);
    fseek(fptr, 0, SEEK_END);
    h -> file_size = ftell(fptr);
    rewind(fptr);

    send_all(sockfd, h, sizeof(*h));

    return fptr;
}

// To ensure all packets are send
static int send_all(int client, const void *buf, size_t len) {
    const unsigned char *p = buf;
    while (len > 0) {
        ssize_t n = send(client, p, len, 0);
        if (n <= 0) return -1;
        p += n;
        len -= n;
    }
    return 0;
}

void conf_sockaddr(struct sockaddr_in *to_addr, const char *ip) {
    to_addr -> sin_family = AF_INET;
    to_addr -> sin_port = htons(PORT);
    memset(&(to_addr -> sin_zero), '\0', 8);

    if (inet_pton(AF_INET, ip, &to_addr->sin_addr) != 1) {
        fprintf(stderr, "IP inválida: %s\n", ip);
        exit(1);
    }
}

