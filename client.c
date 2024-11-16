#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utils.h"

static const MAX_EVENTS = 2;

// 处理标准输入
void handle_stdin_input(int sockfd) {
    char buffer[MAX_BUFFER];
    ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        ssize_t bytes_sent = send_all(sockfd, buffer, bytes_read);
        if (bytes_sent == -1) {
            perror("send failed");
        }
    } else if (bytes_read == 0) {
        printf("Standard input closed\n");
    } else {
        perror("read from stdin");
    }
}

// 处理网络输入
void handle_network_input(int fd) {
    char buffer[MAX_BUFFER];
    ssize_t bytes_received = recv_all(fd, buffer, sizeof(buffer) - 1);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    } else if (bytes_received == 0) {
        printf("Connection closed by server\n");
        close(fd);
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("recv failed");
    }
}

int main() {
    int sockfd = create_socket();

    set_non_blocking(sockfd);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 连接到本地服务器

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        if (errno != EINPROGRESS) {
            perror("connect failed");
            exit(EXIT_FAILURE);
        }
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1 failed");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = STDIN_FILENO;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) == -1) {
        perror("epoll_ctl failed for stdin");
        exit(EXIT_FAILURE);
    }

    event.data.fd = sockfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
        perror("epoll_ctl failed for sockfd");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("epoll_wait failed");
            break;
        }

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == 0) {
                handle_stdin_input(sockfd);
            } else if (events[i].data.fd == sockfd) {
                handle_network_input(sockfd);
            }
        }
    }

    close(epoll_fd);
    close(sockfd);
    return 0;
}
