#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "protocol.h"
#include "reciever.h"

#define BACKLOG 10

static void conf_sockaddr(struct sockaddr_in *from_addr);
static int recv_all(int client, void *buf, size_t len);
static void *discovery_responder(void *arg);

int reciever(void) {
    int i = 0, sockfd, client, optval = 1;
    struct sockaddr_in from_addr;
    socklen_t sin_size;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    conf_sockaddr(&from_addr);

    if (bind(sockfd, (struct sockaddr *)&from_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Start the discovery responder in a separate thread
    pthread_t tid;
    pthread_create(&tid, NULL, discovery_responder, NULL);
    pthread_detach(tid);

    sin_size = sizeof(struct sockaddr_in);
    client = accept(sockfd, (struct sockaddr*)&from_addr, &sin_size);
    if (client == -1) {
        perror("accept");
        exit(1);
    }

    printf("Connection established with sender, waiting for data...\n");
    struct header h;
    recv_all(client, &h, sizeof(h));
    h.name[MAX_NAME - 1] = '\0';

    if (strchr(h.name, '/') != NULL) {
        fprintf(stderr, "nombre invalido\n");
        exit(1);
    }

    unsigned char chunk[CHUNK];
    uint64_t off = 0;
    int count = 0;

    FILE *out = fopen(h.name, "wb");
    if (out == NULL) {
        perror("fopen");
        exit(1);
    }

    while (off < h.file_size) {
        size_t want;
        if (h.file_size - off < CHUNK) {
            want = h.file_size - off;
        } else want = CHUNK;

        if (recv_all(client, chunk, want) == -1) break;
        fwrite(chunk, 1, want, out);
        off += want;
    }
    printf("All bytes received correctly\n");

    fclose(out);

    return 0;
}

static void *discovery_responder(void *arg) {
    (void)arg;
    int s = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind discovery");
        return NULL;
    }

    char buf[64];
    struct sockaddr_in from;
    for (;;) {
        socklen_t len = sizeof(from);
        ssize_t n = recvfrom(s, buf, sizeof(buf) - 1, 0,
                             (struct sockaddr *)&from, &len);
        if (n <= 0) continue;
        buf[n] = '\0';
        if (strcmp(buf, DISCOVERY_MSG) == 0)
            sendto(s, DISCOVERY_REPLY, strlen(DISCOVERY_REPLY), 0,
                   (struct sockaddr *)&from, len);
    }
}

static int recv_all(int client, void *buf, size_t len) {
    unsigned char *p = buf;
    while (len > 0) {
        ssize_t n = recv(client, p, len, 0);
        if (n <= 0) return -1;
        p += n;
        len -= n;
    }
    return 0;
}

static void conf_sockaddr(struct sockaddr_in *from_addr) {
    from_addr -> sin_family = AF_INET;
    from_addr -> sin_port = htons(PORT);
    from_addr -> sin_addr.s_addr = INADDR_ANY;
    memset(&(from_addr -> sin_zero), '\0', 8);
}

