#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUFFER 1024
#define PORT 8080
#define MAX_EVENTS 128

typedef struct fdNode {
    int fd;
    struct fdNode* next;
} fdNode;

typedef struct client_info {
    int fd;
    struct sockaddr_in addr;
} client_info;

client_info* make_info(int fd, struct sockaddr_in addr) {
    client_info* res = (client_info*)malloc(sizeof(client_info));
    res->fd = fd;
    res->addr = addr;
    return res;
}

fdNode* make_fdNode(int fd) {
    fdNode* res = (fdNode*)malloc(sizeof(fdNode));
    res->fd = fd;
    res->next = NULL;
    return res;
}

int create_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(1);
    }
    return sockfd;
}

#endif
