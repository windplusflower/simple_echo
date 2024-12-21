#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "coheader.h"
#include "coroutine.h"
#include "hook.h"
#include "utils.h"

// 处理标准输入
void* handle_stdin_input(const void* arg) {
    int sockfd = *(int*)arg;
    char buffer[MAX_BUFFER];
    while (1) {
        ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) send(sockfd, buffer, bytes_read, 0);
    }
}

// 处理网络输入
void* handle_network_input(const void* arg) {
    int sockfd = *(int*)arg;
    char buffer[MAX_BUFFER];
    while (1) {
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
        }
    }
}

int main() {
    int sockfd = create_socket();
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 连接到本地服务器

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        if (errno != EINPROGRESS) {
            perror("connect failed");
            exit(EXIT_FAILURE);
        }
    }
    enable_hook();
    coroutine_t input, network;
    input = coroutine_create(handle_stdin_input, &sockfd, 0);
    network = coroutine_create(handle_network_input, &sockfd, 0);
    coroutine_join(input);
    coroutine_join(network);

    close(sockfd);
    return 0;
}
