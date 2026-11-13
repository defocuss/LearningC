#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "protocol.h"

#define PORT 3490

void conf_sockaddr(struct sockaddr_in *to_addr);
static int send_all(int client, const void *buf, size_t len);

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in to_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    conf_sockaddr(&to_addr);

    if (connect(sockfd, (struct sockaddr *)&to_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    FILE *fptr = fopen("video.mp4", "rb");

    if (fptr == NULL) {
        perror("fopen");
        return -1;
    }

    struct header h = {0};
    strncpy(h.name, "video.mp4", MAX_NAME - 1);
    h.name_size = strlen(h.name);
    fseek(fptr, 0, SEEK_END);
    h.file_size = ftell(fptr);
    rewind(fptr);

    send_all(sockfd, &h, sizeof(h));

    unsigned char chunk[CHUNK];
    ssize_t n;

    while ((n = fread(chunk, 1, CHUNK, fptr)) > 0 ) {
        send_all(sockfd, chunk, n);
    }
    printf("All bytes sended correctly");

    return 0;

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

void conf_sockaddr(struct sockaddr_in *to_addr) {
    to_addr -> sin_family = AF_INET;
    to_addr -> sin_port = htons(PORT);
    to_addr -> sin_addr.s_addr = INADDR_ANY;
    memset(&(to_addr -> sin_zero), '\0', 8);
}

