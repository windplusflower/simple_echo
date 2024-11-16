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

int create_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(1);
    }
    return sockfd;
}

void set_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get flags failed");
        exit(1);
    }
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

ssize_t send_all(int sockfd, const char *buffer, size_t len) {
    size_t total_sent = 0;
    while (total_sent < len) {
        ssize_t bytes_sent =
            send(sockfd, buffer + total_sent, len - total_sent, 0);
        if (bytes_sent == -1) {
            perror("send failed");
            return -1;
        }
        total_sent += bytes_sent;
    }
    return total_sent;
}

ssize_t recv_all(int sockfd, char *buffer, size_t buffer_size) {
    ssize_t total_received = 0;  // 累计接收到的数据字节数
    ssize_t bytes_received = 0;

    // 循环接收数据，直到没有数据可读
    while (total_received < buffer_size) {
        bytes_received = recv(sockfd, buffer + total_received,
                              buffer_size - total_received, 0);

        if (bytes_received > 0) {
            total_received += bytes_received;
        } else if (bytes_received == 0) {
            break;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        } else {
            perror("recv failed");
            break;
        }
    }

    return total_received;
}

#endif
