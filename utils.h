#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

ssize_t recv_message(int sockfd, char *buffer) {
    memset(buffer, 0, MAX_BUFFER_SIZE);
    return recv(sockfd, buffer, MAX_BUFFER_SIZE - 1, 0);
}

ssize_t send_message(int sockfd, const char *message) {
    return send(sockfd, message, strlen(message), 0);
}

#endif
