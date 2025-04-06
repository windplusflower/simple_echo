
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "coheader.h"
#include "utils.h"
fdNode* head;

void* handle_client(void* arg) {
    client_info* info = (client_info*)arg;
    int fd = info->fd;
    struct sockaddr_in addr = info->addr;
    char buf[MAX_BUFFER];
    //添加到全局的链表
    fdNode* node = make_fdNode(fd);
    node->next = head->next;
    head->next = node;

    while (1) {
        ssize_t bytes_received = recv(fd, buf, MAX_BUFFER, 0);
        if (bytes_received <= 0) {
            // 客户端断开连接
            printf("Server disconnected!\n");
            close(fd);
            fdNode* p = head;
            //从链表中移除
            while (p->next) {
                if (p->next->fd == fd) {
                    fdNode* tmp = p->next;
                    p->next = tmp->next;
                    free(tmp);
                    break;
                }
                p = p->next;
            }
            return NULL;
        } else {
            char str[INET_ADDRSTRLEN + 16];
            snprintf(str, sizeof(str), "From %s:%d  ", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            buf[bytes_received] = '\0';
            if (buf[0] == '\n') continue;
            printf("Received %s%s", str, buf);
            // 广播消息给其他客户端
            fdNode* p = head;
            while (p->next) {
                p = p->next;
                if (p->fd == fd) continue;
                send(p->fd, str, strlen(str) - 1, 0);
                send(p->fd, buf, bytes_received, 0);
            }
        }
    }
}

int main() {
    enable_hook();
    int server_sock, client_sock;
    struct sockaddr_in server_addr;
    socklen_t addr_size;
    head = (fdNode*)calloc(1, sizeof(fdNode));

    server_sock = create_socket();
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, MAX_EVENTS) == -1) {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }
    printf("Server started, waiting for clients...\n");

    while (1) {
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(addr);
        int client_sock = accept(server_sock, (struct sockaddr*)&addr, &addr_size);
        if (client_sock == -1) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        // handle_client与主程序功能上相互独立，没有同步需求，并且主程序不会结束，所以不需要join。
        coroutine_t co = coroutine_create(handle_client, make_info(client_sock, addr), 0);
        coroutine_detach(co);
    }
    close(server_sock);
    return 0;
}
