#ifndef SOCKET_H
#define SOCKET_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef handle_error
static int handle_error(const char* prompt, int result)
{
    if (result < 0) {
        perror(prompt);
        return -1;
    }
    return result;
}

#endif

// socket_ipc
#define SOCKET_PATH "unix_domain.socket"
#define SERVER_MODE 1
#define CLIENT_MODE 2
#define BUF_LEN 1024

#ifndef handle_error_ipc
static void handle_error_ipc(const char* prompt, int result)
{
    if (result < 0) {
        perror(prompt);
        exit(EXIT_FAILURE);
    }
}

#endif

// epoll
#define MAX_EVENTS 128 //定义 epoll_wait 每次循环最多能处理多少个“就绪”事件

void num_endianess_convert(); // 网络字节序和主机字节序相互转换

void inet_endianess_convert();

void socket_test_server(); // 进行 socket 测试中的服务端

void* read_from_client(void* arg); // 读取客户端发送的消息

void* write_to_client(void* arg); // 向客户端发送消息

void socket_test_client(); // 进行 socket 测试中的客户端

void* read_from_server(void* arg); // 读取服务端发送的消息

void* write_to_server(void* arg); // 向服务端发送消息

void multi_connection_threads(); // 多线程多个客户端连接服务端

void* read_from_client_then_write(
    void* arg); // 服务端连接多个客户端只通过当前一个函数进行读取和回复

void multi_connection_processes(); //多进程多个客户端连接服务端

void zombie_dealer(int sig); // 关闭回收所有的子进程，防止僵尸进程

void UDP_test_server(); // UDP 协议测试的服务端(当客户端发送消息与 "EOF"
                        // 相同时，结束连接)

void UDP_test_client(); // UDP 协议测试的客户端

void socket_ipc_test(int argc,
                     char const* argv[]); // 通过 main 传参选择不同的模式

void ipc_server_mode(int socket_fd, char* buf_ipc,
                     struct sockaddr_un socket_ipc_addr);

void ipc_client_mode(int socket_fd, char* buf_ipc,
                     struct sockaddr_un socket_ipc_addr);

static int set_non_blocking(int fd); // 设置文件描述符为非阻塞模式

void multi_connection_epoll(); // I/O
                               // 多路复用，一个线程即可管理多个客户端连接，不阻塞

#endif // SOCKET_H