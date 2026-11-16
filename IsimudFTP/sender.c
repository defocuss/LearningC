#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocol.h"

void conf_sockaddr(struct sockaddr_in *to_addr, const char *ip);
static FILE * send_header(const char *f_path, int sockfd, struct header *h);
static int send_all(int client, const void *buf, size_t len);

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in to_addr;
    if (argc < 2) {
        fprintf(stderr, "uso: %s <ip_destino>\n", argv[0]);
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    conf_sockaddr(&to_addr, argv[1]);

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
    printf("All bytes sended correctly");

    fclose(fptr);

    return 0;

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

