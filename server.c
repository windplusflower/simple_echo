#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "utils.h"

#define MAX_EVENTS 10

int main() {
    int server_sock, epoll_fd, nfds, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    struct epoll_event event, events[MAX_EVENTS];
    int client_fds[MAX_EVENTS];
    int num_clients = 0;

    server_sock = create_socket();
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, MAX_EVENTS) == -1) {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Epoll create failed");
        close(server_sock);
        exit(1);
    }

    set_non_blocking(server_sock);
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = server_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock, &event) == -1) {
        perror("Epoll ctl failed");
        close(server_sock);
        exit(1);
    }

    printf("Server started, waiting for clients...\n");

    while (1) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_sock) {
                addr_size = sizeof(client_addr);
                client_sock = accept(
                    server_sock, (struct sockaddr *)&client_addr, &addr_size);
                if (client_sock == -1) {
                    perror("Accept failed");
                    continue;
                }

                printf("Client connected: %s:%d\n",
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port));

                set_non_blocking(client_sock);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_sock;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event) ==
                    -1) {
                    perror("Epoll ctl failed");
                    close(client_sock);
                    continue;
                }

                client_fds[num_clients++] = client_sock;

            } else if (events[i].events & EPOLLIN) {
                char buf[MAX_BUFFER];
                ssize_t bytes_received =
                    recv_all(events[i].data.fd, buf, MAX_BUFFER);
                if (bytes_received <= 0) {
                    // 客户端断开连接
                    close(events[i].data.fd);
                    for (int j = 0; j < num_clients; j++) {
                        if (client_fds[j] == events[i].data.fd) {
                            client_fds[j] = client_fds[--num_clients];
                            break;
                        }
                    }
                } else {
                    buf[bytes_received] = '\0';
                    printf("Received: %s", buf);
                    // 广播消息给其他客户端
                    for (int j = 0; j < num_clients; j++) {
                        if (client_fds[j] != events[i].data.fd) {
                            send_all(client_fds[j], buf, bytes_received);
                        }
                    }
                }
            }
        }
    }

    close(server_sock);
    return 0;
}
