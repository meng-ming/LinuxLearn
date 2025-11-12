#include "socket.h"

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void num_endianess_convert()
{
    // 声明两个端口号
    unsigned short local_num = 0x1f, network_num = 0;
    network_num = htons(local_num);
    // 同字节内不会改变，将主机整数前面的字节与当前字节顺序调换（两个字节顺序调换）
    printf("主机无符号整数0x%hX转换为网络字节序0x%hX\n", local_num,
           network_num);
    local_num = ntohs(network_num);
    printf("网络字节序0x%hX转换为主机无符号整数0x%hX\n", network_num,
           local_num);
}

void inet_endianess_convert()
{
    printf("192.168.6.101: 0x%x 0x%x 0x%x 0x%x\n", 192, 168, 6, 101);
    // 声明结构体接受数据
    struct sockaddr_in server_addr;
    struct in_addr server_in_addr;
    // 初始化
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&server_in_addr, 0, sizeof(server_in_addr));
    // 将 IP 地址转换为网络字节序
    inet_aton("192.168.6.101", &server_in_addr);
    printf("inet_aton: 0x%X\n", server_in_addr.s_addr);
    // 万能方法
    inet_pton(AF_INET, "192.168.6.101", &server_addr.sin_addr);
    printf("inet_pton: 0x%X\n",
           server_addr.sin_addr.s_addr); // 修正：使用正确的结构体成员
    // 将结构体转换为字符串
    printf("转换为字符串：%s\n", inet_ntoa(server_in_addr));
}

void socket_test_server()
{
    int* client_socket_fd_ptr = malloc(sizeof(int));
    if (client_socket_fd_ptr == NULL) {
        perror("malloc client fd ptr");
        exit(EXIT_FAILURE);
    }
    int server_socket_fd, temp_result;
    pthread_t pid_read, pid_write;

    // 创建服务端和客户端的结构体
    struct sockaddr_in server_addr, client_addr;
    // 初始化
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // 填写服务端协议
    server_addr.sin_family = AF_INET;
    // 填写服务端IP以及端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 初始化 0.0.0.0
    server_addr.sin_port = htons(8888);

    // 网络编程正式流程
    // 1.创建 socket
    server_socket_fd =
        handle_error("server_socket", socket(AF_INET, SOCK_STREAM, 0));
    if (server_socket_fd < 0) {
        exit(EXIT_FAILURE);
    }
    // 允许端口复用
    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt(socket_test_server)");
        close(server_socket_fd);
        free(client_socket_fd_ptr);
        exit(EXIT_FAILURE);
    }

    // 2.绑定地址
    temp_result = handle_error(
        "bind", bind(server_socket_fd, (const struct sockaddr*)&server_addr,
                     sizeof(server_addr)));
    if (temp_result < 0) {
        free(client_socket_fd_ptr);
        exit(EXIT_FAILURE);
    }
    // 3.开始监听
    temp_result = handle_error("listen", listen(server_socket_fd, 128));
    if (temp_result < 0) {
        free(client_socket_fd_ptr);
        exit(EXIT_FAILURE);
    }
    // 4.获取客户端的连接
    socklen_t client_addr_len = sizeof(client_addr);
    *client_socket_fd_ptr = handle_error(
        "accept", accept(server_socket_fd, (struct sockaddr*)&client_addr,
                         &client_addr_len));
    if (*client_socket_fd_ptr < 0) {
        free(client_socket_fd_ptr);
        exit(EXIT_FAILURE);
    }
    printf("与客户端%s 端口%d建立连接 文件描述符：%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
           *client_socket_fd_ptr);

    // 创建读写子线程
    if (pthread_create(&pid_read, NULL, read_from_client,
                       client_socket_fd_ptr) != 0) {
        perror("pthread_create read(socket_test_server)");
        free(client_socket_fd_ptr);
        close(*client_socket_fd_ptr);
        close(server_socket_fd);
        return;
    }
    if (pthread_create(&pid_write, NULL, write_to_client,
                       client_socket_fd_ptr) != 0) {
        perror("pthread_create write(socket_test_server)");
        free(client_socket_fd_ptr);
        close(*client_socket_fd_ptr);
        close(server_socket_fd);
        return;
    }
    // 等待子线程完成，阻塞主线程
    pthread_join(pid_read, NULL);
    pthread_join(pid_write, NULL);

    // 释放资源
    free(client_socket_fd_ptr);
    close(server_socket_fd);
    close(*client_socket_fd_ptr);
}

void* read_from_client(void* arg)
{
    int client_socket_fd = *(int*)arg;
    char read_buf[1024];
    memset(read_buf, 0, sizeof(read_buf));

    // 接收数据
    for (;;) {
        ssize_t recv_count =
            recv(client_socket_fd, read_buf, sizeof(read_buf) - 1, 0);
        if (recv_count < 0) {
            if (errno == EINTR) continue;
            perror("recv(read_from_client)");
            break;
        }
        if (recv_count == 0) {
            printf("客户端停止发送消息！\n");
            shutdown(client_socket_fd, SHUT_RD);
            return NULL;
        }
        // 将读取到的数据打印到终端
        read_buf[recv_count] = '\0';
        write(STDOUT_FILENO, read_buf, recv_count);
        fflush(stdout);
    };
    // 读取结束
    printf("客户端关闭连接\n");
    shutdown(client_socket_fd, SHUT_RD);
    return NULL;
}

void* write_to_client(void* arg)
{
    int client_socket_fd = *(int*)arg;

    char write_buf[1024];
    memset(write_buf, 0, sizeof(write_buf));
    ssize_t write_count;

    while (fgets(write_buf, sizeof(write_buf), stdin) != NULL) {
        // 发送数据
        ssize_t sented_len = 0;
        size_t len = strlen(write_buf);
        while (sented_len < len) {
            ssize_t n = send(client_socket_fd, write_buf + sented_len,
                             len - sented_len, 0);
            if (n < 0) {
                if (errno == EINTR) continue;
                perror("send(write_to_client)");
                shutdown(client_socket_fd, SHUT_WR); // 半关闭写方向
                return NULL;
            }
            sented_len += n;
        }
    }
    printf("服务端停止写入到客户端！\n");
    shutdown(client_socket_fd, SHUT_WR);
    return NULL;
}

void socket_test_client()
{
    int* client_socket_fd_ptr = malloc(sizeof(int));
    if (client_socket_fd_ptr == NULL) {
        perror("malloc client fd ptr");
        exit(EXIT_FAILURE);
    }
    int temp_result;
    pthread_t pid_read, pid_write;
    // 创建 sockaddr_in 结构体（只适用于 IPV4）
    struct sockaddr_in server_addr;
    // 清空
    memset(&server_addr, 0, sizeof(server_addr));

    // 初始化 服务端IP地址和端口号（得与服务端的IP及端口号相同）
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    server_addr.sin_port = htons(8888);

    // 网络编程
    // 1.创建socket
    *client_socket_fd_ptr =
        handle_error("client_socket_fd", socket(AF_INET, SOCK_STREAM, 0));
    if (*client_socket_fd_ptr < 0) {
        free(client_socket_fd_ptr);
        exit(EXIT_FAILURE);
    }
    // 2.连接服务端
    temp_result =
        handle_error("connect", connect(*client_socket_fd_ptr,
                                        (const struct sockaddr*)&server_addr,
                                        sizeof(server_addr)));
    if (temp_result < 0) {
        free(client_socket_fd_ptr);
        exit(EXIT_FAILURE);
    }
    printf("已经连接上服务端 %s %d \n", inet_ntoa(server_addr.sin_addr),
           ntohs(server_addr.sin_port));

    // 创建读写子线程
    if (pthread_create(&pid_read, NULL, read_from_server,
                       client_socket_fd_ptr) != 0) {
        perror("pthread_create read(socket_test_client)");
        close(*client_socket_fd_ptr);
        free(client_socket_fd_ptr);
        return;
    }
    if (pthread_create(&pid_write, NULL, write_to_server,
                       client_socket_fd_ptr) != 0) {
        perror("pthread_create write(socket_test_client)");
        close(*client_socket_fd_ptr);
        free(client_socket_fd_ptr);
        return;
    }
    // 阻塞主线程
    pthread_join(pid_read, NULL);
    pthread_join(pid_write, NULL);

    // 释放资源
    close(*client_socket_fd_ptr);
    free(client_socket_fd_ptr);
}

void* read_from_server(void* arg)
{
    int client_socket_fd = *(int*)arg;

    char read_buf[1024];
    memset(read_buf, 0, sizeof(read_buf));

    // 接收数据

    for (;;) {
        ssize_t recv_count =
            recv(client_socket_fd, read_buf, sizeof(read_buf) - 1, 0);
        if (recv_count < 0) {
            if (errno == EINTR) continue;
            perror("recv(read_from_server)");
            break;
        }
        if (recv_count == 0) {
            printf("服务端停止发送消息！\n");
            shutdown(client_socket_fd, SHUT_RD);
            return NULL;
        }
        // 将读取到的数据打印到终端
        read_buf[recv_count] = '\0';
        write(STDOUT_FILENO, read_buf, recv_count);
        fflush(stdout);
    };
    // 读取结束
    printf("服务端关闭连接！\n");
    shutdown(client_socket_fd, SHUT_RD);
    return NULL;
}

void* write_to_server(void* arg)
{
    int client_socket_fd = *(int*)arg;

    char write_buf[1024];
    memset(write_buf, 0, sizeof(write_buf));
    ssize_t write_count;

    while (fgets(write_buf, sizeof(write_buf), stdin) != NULL) {
        // 发送数据
        ssize_t sented_len = 0;
        size_t len = strlen(write_buf);
        while (sented_len < len) {
            ssize_t n = send(client_socket_fd, write_buf + sented_len,
                             len - sented_len, 0);
            if (n < 0) {
                if (errno == EINTR) continue;
                perror("send(write_to_server)");
                shutdown(client_socket_fd, SHUT_WR); // 半关闭写方向
                return NULL;
            }
            sented_len += n;
        }
    }
    printf("客户端停止写入到服务端！\n");
    shutdown(client_socket_fd, SHUT_WR); // 半关闭写方向
    return NULL;
}

void multi_connection_threads()
{
    int server_socket_fd, temp_result;

    // 创建服务端和客户端的结构体
    struct sockaddr_in server_addr, client_addr;
    // 初始化
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // 填写服务端协议
    server_addr.sin_family = AF_INET;
    // 填写服务端IP以及端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 初始化 0.0.0.0
    server_addr.sin_port = htons(8888);

    // 网络编程正式流程
    // 1.创建 socket
    server_socket_fd =
        handle_error("server_socket", socket(AF_INET, SOCK_STREAM, 0));
    if (server_socket_fd < 0) {
        exit(EXIT_FAILURE);
    }
    // 允许端口复用
    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt(multi_connection_threads)");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // 2.绑定地址
    temp_result = handle_error(
        "bind", bind(server_socket_fd, (const struct sockaddr*)&server_addr,
                     sizeof(server_addr)));
    if (temp_result < 0) {
        exit(EXIT_FAILURE);
    }
    // 3.开始监听
    temp_result = handle_error("listen", listen(server_socket_fd, 128));
    if (temp_result < 0) {
        exit(EXIT_FAILURE);
    }
    // 4.获取客户端的连接
    socklen_t client_addr_len = sizeof(client_addr);
    // 死循环连接多个客户端
    while (1) {
        pthread_t pid_read_from_client_then_write;

        int* client_socket_fd_ptr = malloc(sizeof(int));
        if (client_socket_fd_ptr == NULL) {
            perror("malloc client_socket_fd_ptr");
            continue;
        }

        // 保证每个连接的客户端的 文件描述符 物理位置不同
        *client_socket_fd_ptr = handle_error(
            "accept", accept(server_socket_fd, (struct sockaddr*)&client_addr,
                             &client_addr_len));
        if (*client_socket_fd_ptr < 0) {
            free(client_socket_fd_ptr); // 接受失败，释放内存
            exit(EXIT_FAILURE);
        }
        printf("与客户端%s 端口%d建立连接 文件描述符：%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
               *client_socket_fd_ptr);

        // 创建读写子线程
        if (pthread_create(&pid_read_from_client_then_write, NULL,
                           read_from_client_then_write,
                           client_socket_fd_ptr) != 0) {
            perror("pthread_create read(multi_connection_threads)");
            close(*client_socket_fd_ptr);
            free(client_socket_fd_ptr);
            continue; // 继续接受下一个客户端
        }
        // 等待子线程完成，不阻塞主线程
        pthread_detach(pid_read_from_client_then_write);
    }
    close(server_socket_fd);
}

void* read_from_client_then_write(void* arg)
{
    int* client_socket_fd_ptr = (int*)arg;
    int client_socket_fd = *client_socket_fd_ptr;
    char read_buf[1024];
    memset(read_buf, 0, sizeof(read_buf));

    char write_buf[1024];
    memset(write_buf, 0, sizeof(write_buf));

    ssize_t recv_count, write_count;

    for (;;) {
        // 读取收到的消息并输出到控制台
        recv_count = recv(client_socket_fd, read_buf, sizeof(read_buf) - 1, 0);

        if (recv_count < 0) {
            if (errno == EINTR) continue;
            perror("recv(read_from_client_then_write)");
            close(client_socket_fd);
            free(client_socket_fd_ptr);
            return NULL;
        }
        // 客户端输入 Ctrl + D
        else if (recv_count == 0) {
            printf("客户端 %d 已断开连接\n", client_socket_fd);
            close(client_socket_fd);
            free(client_socket_fd_ptr);
            return NULL;
        }

        // 打印客户端发送的消息
        if (read_buf[recv_count - 1] == '\n') {
            read_buf[recv_count - 1] = '\0'; // 去除回车
        } else {
            read_buf[recv_count - 1] = '\0';
        }

        char msg[1100];
        snprintf(msg, sizeof(msg), "客户端%d发送的消息:%s\n", client_socket_fd,
                 read_buf);
        write(STDOUT_FILENO, msg, strlen(msg));

        // 回复客户端 收到！
        strcpy(write_buf, "收到！\n");
        // 发送数据
        ssize_t sented_len = 0;
        size_t len = strlen(write_buf);
        while (sented_len < len) {
            ssize_t n = send(client_socket_fd, write_buf + sented_len,
                             len - sented_len, 0);
            if (n < 0) {
                if (errno == EINTR) continue;
                perror("send(read_from_client_then_write)");
                shutdown(client_socket_fd, SHUT_WR); // 半关闭写方向
                free(client_socket_fd_ptr);
                return NULL;
            }
            sented_len += n;
        }
    }
}

void multi_connection_processes()
{
    int server_socket_fd, temp_result;

    // 创建服务端和客户端的结构体
    struct sockaddr_in server_addr, client_addr;
    // 初始化
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // 注册信号处理函数 SIGCHLD
    signal(SIGCHLD, zombie_dealer);

    // 填写服务端协议
    server_addr.sin_family = AF_INET;
    // 填写服务端IP以及端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 初始化 0.0.0.0
    server_addr.sin_port = htons(8888);

    // 网络编程正式流程
    // 1.创建 socket
    server_socket_fd =
        handle_error("server_socket", socket(AF_INET, SOCK_STREAM, 0));
    if (server_socket_fd < 0) {
        exit(EXIT_FAILURE);
    }
    // 允许端口复用
    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt(multi_connection_processes)");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // 2.绑定地址
    temp_result = handle_error(
        "bind", bind(server_socket_fd, (const struct sockaddr*)&server_addr,
                     sizeof(server_addr)));
    if (temp_result < 0) {
        exit(EXIT_FAILURE);
    }
    // 3.开始监听
    temp_result = handle_error("listen", listen(server_socket_fd, 128));
    if (temp_result < 0) {
        exit(EXIT_FAILURE);
    }
    // 4.获取客户端的连接
    socklen_t client_addr_len = sizeof(client_addr);
    // 死循环连接多个客户端
    while (1) {
        pthread_t pid_read_from_client_then_write;

        // 保证每个连接的客户端的 文件描述符 物理位置不同
        int client_socket_fd = handle_error(
            "accept", accept(server_socket_fd, (struct sockaddr*)&client_addr,
                             &client_addr_len));
        if (client_socket_fd < 0) {
            exit(EXIT_FAILURE);
        }

        // 每次连接上新的客户端创建子进程
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_socket_fd);
            continue;
        } else if (pid == 0) {
            // 子进程
            // 关闭不使用的 server_socket_fd
            close(server_socket_fd);
            // 文件描述符 4 在子进程
            // 1和在子进程2中，是完全不同的两个东西，它们指向两个不同的网络连接。
            // 它们只是恰好共享了同一个数字"4"，因为在它们各自的进程空间里，这个数字"4"都是可用的。
            printf("与客户端%s 端口%d建立连接 文件描述符：%d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                   client_socket_fd);

            // 必须为子进程提供它自己的 I/O循环
            // （原有的read_then_write函数必须要free内存，当前进程不能实现）
            // 从客户端读取信息并回复 收到！
            char read_buf[1024];
            char write_buf[1024];
            ssize_t recv_count;

            for (;;) {
                memset(read_buf, 0, sizeof(read_buf));
                recv_count =
                    recv(client_socket_fd, read_buf, sizeof(read_buf) - 1, 0);

                if (recv_count <= 0) { // 合并 0 和 < 0
                    if (recv_count == 0) {
                        printf("客户端 %d 已断开连接\n", client_socket_fd);
                    } else if (errno != EINTR) {
                        perror("recv(multi_connection_processes)");
                    }
                    break; // 退出循环
                }

                read_buf[recv_count] = '\0';
                printf("客户端%d发送的消息:%s\n", client_socket_fd, read_buf);

                // 回复 "收到！"
                strcpy(write_buf, "收到！\n");
                ssize_t sented_len = 0;
                size_t len = strlen(write_buf);
                while (sented_len < len) {
                    ssize_t n = send(client_socket_fd, write_buf + sented_len,
                                     len - sented_len, 0);
                    if (n < 0) {
                        if (errno == EINTR) continue;
                        perror("send(multi_connection_processes)");
                        break; // 跳出 send 循环
                    }
                    sented_len += n;
                }
                if (sented_len != len) { // 如果 send 失败，也退出主循环
                    break;
                }
            }

            // 子进程的 I/O 循环结束后，关闭 fd 并退出
            close(client_socket_fd);
            exit(EXIT_SUCCESS);
        } else {
            // 父进程
            close(client_socket_fd);
        }
    }
    close(server_socket_fd);
}

void zombie_dealer(int sig)
{
    pid_t pid;
    int status;
    // 一个 SIGCHLD 可能对应多个子进程的退出
    // 使用 while 循环回收所有退出的子进程，避免僵尸进程的出现
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("子进程：%d 以 %d 状态正常退出，已被回收！\n", pid,
                   WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("子进程：%d 被 %d 信号杀死，已被回收！\n", pid,
                   WTERMSIG(status));
        } else {
            printf("子进程：%d 因其他原因退出，已被回收！\n", pid);
        }
    }
}

void UDP_test_server()
{
    int server_socket_fd, temp_result;
    char read_buf[1024];

    // 创建服务端和客户端的结构体
    struct sockaddr_in server_addr, client_addr;
    // 初始化
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // 填写服务端协议
    server_addr.sin_family = AF_INET;
    // 填写服务端IP以及端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 初始化 0.0.0.0
    server_addr.sin_port = htons(8888);

    // 网络编程正式流程
    // 1.创建 socket
    server_socket_fd =
        handle_error("server_socket", socket(AF_INET, SOCK_DGRAM, 0));
    if (server_socket_fd < 0) {
        exit(EXIT_FAILURE);
    }
    // 允许端口复用
    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt(UPD_test_server)");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }
    // 2.绑定地址
    temp_result = handle_error(
        "bind", bind(server_socket_fd, (const struct sockaddr*)&server_addr,
                     sizeof(server_addr)));
    if (temp_result < 0) {
        exit(EXIT_FAILURE);
    }

    socklen_t client_addr_len = sizeof(client_addr);

    // 直接可以收发数据
    while (1) {
        // 每次接受数据清空缓存区
        memset(read_buf, 0, sizeof(read_buf));

        // 接收数据
        ssize_t n = recvfrom(server_socket_fd, read_buf, sizeof(read_buf), 0,
                             (struct sockaddr*)&client_addr, &client_addr_len);
        if (n < 0) {
            exit(EXIT_FAILURE);
        } else {
            read_buf[n] = '\0';
        }

        if (strcmp(read_buf, "EOF") != 0) {
            printf("接收到客户端%s %d信息 %s\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                   read_buf);
        } else {
            // 断开连接
            printf("客户端断开连接！\n");
            break;
        }

        // 回复收到
        const char* server_reply = "收到！\n";
        temp_result = handle_error(
            "sendto",
            sendto(server_socket_fd, server_reply, strlen(server_reply), 0,
                   (struct sockaddr*)&client_addr, client_addr_len));
        if (temp_result < 0) {
            exit(EXIT_FAILURE);
        }
    }
    close(server_socket_fd);
}

void UDP_test_client()
{
    int client_socket_fd, temp_result;
    char buf[1024];

    // 创建服务端和客户端的结构体
    struct sockaddr_in server_addr;
    // 初始化
    memset(&server_addr, 0, sizeof(server_addr));

    // 填写服务端协议
    server_addr.sin_family = AF_INET;
    // 填写服务端IP以及端口
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    server_addr.sin_port = htons(8888);

    // 网络编程正式流程
    // 1.创建 socket
    client_socket_fd =
        handle_error("server_socket", socket(AF_INET, SOCK_DGRAM, 0));
    if (client_socket_fd < 0) {
        exit(EXIT_FAILURE);
    }

    // 直接可以收发数据
    while (1) {
        // 先输入发送消息，若为退出标志，则退出循环，否则发送到 server

        // 每次接受数据清空缓存区
        memset(buf, 0, sizeof(buf));

        // 发送消息
        printf("请输入你想要发送的消息：");
        fflush(stdout); // 关键！
        int write_mes_len = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (write_mes_len <= 0) {
            printf("输入错误或EOF,退出\n");
            break;
        } else {
            if (buf[write_mes_len - 1] == '\n')
                buf[write_mes_len - 1] = '\0'; // 去换行
            else
                buf[write_mes_len] = '\0';
        }

        temp_result =
            handle_error("sendto", sendto(client_socket_fd, buf, strlen(buf), 0,
                                          (struct sockaddr*)&server_addr,
                                          sizeof(server_addr)));
        if (temp_result < 0) {
            exit(EXIT_FAILURE);
        }

        // 如果当前为退出逻辑
        if (strcmp(buf, "EOF") == 0) {
            break;
        }

        // 接收数据
        ssize_t n =
            recvfrom(client_socket_fd, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (n < 0) {
            exit(EXIT_FAILURE);
        } else {
            buf[n] = '\0';
        }

        if (strcmp(buf, "EOF") != 0) {
            printf("接收到服务端%s %d信息 %s", inet_ntoa(server_addr.sin_addr),
                   ntohs(server_addr.sin_port), buf);
        } else {
            // 断开连接
            printf("服务端断开连接！\n");
            break;
        }
    }
    close(client_socket_fd);
}

void socket_ipc_test(int argc, char const* argv[])
{
    // 如果不填参数或者填写"server"，启动服务端
    int mode = 0, socket_fd;
    char* buf_ipc = malloc(BUF_LEN);
    if (buf_ipc == NULL) {
        perror("malloc buf_ipc");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un socket_ipc_addr;

    if (argc == 1 || strcmp(argv[1], "server") == 0) {
        mode = SERVER_MODE;
        // 主动删除上一次运行（可能是异常退出的）所残留的旧 socket文件。
        // 防止 "Address already in use"
        unlink(SOCKET_PATH);
    } else if (strcmp(argv[1], "client") == 0) {
        mode = CLIENT_MODE;
    } else {
        perror("参数错误！");
        exit(EXIT_FAILURE);
    }

    // 网络连接
    // 初始化 ipc 结构体
    memset(&socket_ipc_addr, 0, sizeof(socket_ipc_addr));
    socket_ipc_addr.sun_family = AF_UNIX;
    strcpy(socket_ipc_addr.sun_path, SOCKET_PATH);

    // 1.创建 socket
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    handle_error_ipc("socket", socket_fd);

    // 选择不同的模式
    switch (mode) {
    case SERVER_MODE:
        ipc_server_mode(socket_fd, buf_ipc, socket_ipc_addr);
        break;
    case CLIENT_MODE:
        ipc_client_mode(socket_fd, buf_ipc, socket_ipc_addr);
    default:
        break;
    }

    // 清空变量
    if (mode == SERVER_MODE) {
        unlink(SOCKET_PATH); // 服务端模式，在最后负责清理
    }
    free(buf_ipc);
    close(socket_fd);
}

void ipc_server_mode(int socket_fd, char* buf_ipc,
                     struct sockaddr_un socket_ipc_addr)
{
    printf("启动服务端！\n");
    // 服务端收到消息并打印，收到 EOF 表示结束
    int temp_result, client_socket_fd, recv_count;

    // bind
    handle_error_ipc("ipc_bind",
                     bind(socket_fd, (const struct sockaddr*)&socket_ipc_addr,
                          sizeof(socket_ipc_addr)));

    // listen
    handle_error_ipc("listen_ipc", listen(socket_fd, 128));

    // accept
    client_socket_fd = accept(socket_fd, NULL, NULL);
    handle_error_ipc("accept_ipc", client_socket_fd);

    // 打印连接成功
    printf("成功连接上客户端！\n");

    // 收到消息判断并打印
    while (1) {
        memset(buf_ipc, 0, BUF_LEN);

        // 接收消息
        recv_count = recv(client_socket_fd, buf_ipc, BUF_LEN, 0);
        handle_error_ipc("recv_ipc", recv_count);
        // 客户端按下 Ctrl + D
        if (recv_count == 0) {
            shutdown(client_socket_fd, SHUT_RD);
            printf("客户端停止发送消息！\n");
            fflush(stdout);
            close(client_socket_fd);
            break;
        }

        // 在比较 EOF 之前移除换行符
        if (recv_count > 0 && buf_ipc[recv_count - 1] == '\n') {
            buf_ipc[recv_count - 1] = '\0';
        } else {
            buf_ipc[recv_count] = '\0'; // 正常添加 null 终止符
        }

        // 判断是否终止信息
        if (strcmp(buf_ipc, "EOF") == 0) {
            printf("客户端断开连接！\n");
            close(client_socket_fd);
            break;
        }

        printf("客户端发送消息:%s\n", buf_ipc);

        // 服务端回复 收到
        strcpy(buf_ipc, "收到！\n");
        handle_error_ipc("send_ipc",
                         send(client_socket_fd, buf_ipc, strlen(buf_ipc), 0));
    }
}

void ipc_client_mode(int socket_fd, char* buf_ipc,
                     struct sockaddr_un socket_ipc_addr)
{
    printf("启动客户端！\n");

    // 连接服务端
    handle_error_ipc("connect_ipc",
                     connect(socket_fd,
                             (const struct sockaddr*)&socket_ipc_addr,
                             sizeof(socket_ipc_addr)));
    printf("成功连接上服务端!\n");

    while (1) {
        memset(buf_ipc, 0, BUF_LEN);

        printf("请输入要发送的消息:");
        fflush(stdout);

        // 读取控制台输入
        ssize_t read_count = read(STDIN_FILENO, buf_ipc, BUF_LEN - 1);
        handle_error_ipc("read_ipc_client", read_count);
        if (read_count > 0) {
            // 移除可能的换行符,否则 buf_ipc 内读取的是 "EOF\n"
            if (buf_ipc[read_count - 1] == '\n') {
                buf_ipc[read_count - 1] = '\0';
            } else {
                buf_ipc[read_count] = '\0';
            }
        } else {
            // 客户端按下 Ctrl + D
            strcpy(buf_ipc, "EOF");
        }

        // 发送消息
        handle_error_ipc("send_ipc",
                         send(socket_fd, buf_ipc, strlen(buf_ipc), 0));

        if (strcmp(buf_ipc, "EOF") == 0) {
            printf("客户端主动断开连接！\n");
            shutdown(socket_fd, SHUT_WR);
            break;
        }

        // 打印客户端回复的消息
        ssize_t recv_count = recv(socket_fd, buf_ipc, BUF_LEN, 0);
        handle_error_ipc("recv_ipc_client", recv_count);

        // 防止服务端主动断开连接
        if (recv_count == 0) {
            printf("服务端已断开连接！\n");
            break; // 退出循环
        }

        if (buf_ipc[recv_count - 1] == '\n') {
            buf_ipc[recv_count - 1] = '\0';
        }
        printf("服务端：%s\n", buf_ipc);
    }
}

static int set_non_blocking(int fd)
{
    // 1.获取 fd 当前的标志
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        return -1;
    }
    // 2.在现有标志基础上添加非阻塞标识（O_NONBLOCK)
    flags |= O_NONBLOCK;

    // 3.将修改完之后的标志重新设置回去
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl(F_SETFL)");
        return -1;
    }
    return 0;
}

void multi_connection_epoll()
{
    int server_socket_fd, temp_result;
    struct sockaddr_in server_addr, client_addr;

    // --- 步骤 1：初始化服务器 Socket  ---

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));



    // 初始化 server_addr
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网卡
    server_addr.sin_port = htons(8888);              // 监听8888端口

    // 1.创建 socket
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("socket(multi_connection_epoll)");
        exit(EXIT_FAILURE);
    }

    // 设置端口复用，允许服务器快速重启
    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt(multi_connection_epoll)");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // 2.绑定地址
    temp_result = bind(server_socket_fd, (const struct sockaddr*)&server_addr,
                       sizeof(server_addr));
    if (temp_result < 0) {
        perror("bind(multi_connection_epoll)");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // 3.开始监听
    temp_result = listen(server_socket_fd, 128);
    if (temp_result < 0) {
        perror("listen(multi_connection_epoll)");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // --- 步骤 2：初始化 Epoll (新内容) ---
    // 4.创建一个 epoll 实例（可看作在内核中创建了一个“事件中心”）
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1(multi_connection_epoll)");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // 5.将“监听套接字”设置为非阻塞模式，防止 accept() 卡住主循环
    if (set_non_blocking(server_socket_fd) == -1) {
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // 6.将监听套接字注册到 epoll 事件中心
    struct epoll_event ep_event;
    ep_event.events = EPOLLIN; // 关心 "可读" 事件 (EPOLLIN)
                               // 对于监听 FD 来说, "可读" = "有新的连接请求"
    ep_event.data.fd =
        server_socket_fd; // 告诉 epoll 这个事件是属于 server_socket_fd 的

    // EPOLL_CTL_ADD: 添加新事件； EPOLL_CTL_MOD: 修改； EPOLL_CTL_DEL: 删除
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &ep_event) == -1) {
        perror("epoll_ctl(multi_connection_epoll):server_socket_fd");
        exit(EXIT_FAILURE);
    }

    // 用来接收 epoll_wait 返回的“就绪事件”的数组
    struct epoll_event ep_events[MAX_EVENTS];
    socklen_t client_addr_len = sizeof(client_addr);

    printf("Epoll 服务器启动，等待连接...\n");

    // --- 步骤 3：主事件循环 (服务器的核心) ---
    while (1) {
        // 7. 等待事件发生
        // epoll_wait 会阻塞当前线程，直到有 FD 就绪，或者超时
        // `events` 数组会被内核填充
        // `MAX_EVENTS` 是最多返回多少个事件
        // `-1` 表示无限期阻塞 (永不超时)
        // 返回值 n_ready 是 "就绪" 的 FD 数量
        int n_ready = epoll_wait(epoll_fd, ep_events, MAX_EVENTS, -1);
        if (n_ready == -1) {
            if (errno == EINTR) continue; // 如果是被信号中断，就继续循环
            perror("epoll_wait(multi_connection_epoll)");
            break;
        }

        // 8. 遍历所有 n_ready 个就绪的事件
        for (int i = 0; i < n_ready; i++) {
            // --- 9. 事件类型 1：是 "监听FD" 就绪吗？ ---
            // 这表示有 "新的客户端连接" 到来了
            if (ep_events[i].data.fd == server_socket_fd) {
                // (关键) 我们必须使用循环来 accept
                // 因为 server_socket_fd 是非阻塞的，可能有多个连接同时到达
                while (1) {
                    int client_socket_fd =
                        accept(server_socket_fd,
                               (struct sockaddr* restrict)&client_addr,
                               &client_addr_len);
                    if (client_socket_fd == -1) {
                        // 因为是非阻塞模式，当 accept 返回 -1 时：
                        // 检查 errno 是否是 EAGAIN 或 EWOULDBLOCK
                        // 这表示 "所有的新连接都已经被处理完毕了"
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; // 跳出 accept
                                   // 循环，处理下一个事件

                        } else {
                            perror(
                                "accept(multi_connection_epoll)"); // 发生了真正的
                                                                   // accept
                                                                   // 错误
                            break;
                        }
                    }
                    // 成功接受一个客户端
                    printf("与客户端%s 端口%d建立连接，文件描述符%d\n",
                           inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port), client_socket_fd);

                    // 将新客户端的 文件描述符 也设置为非阻塞模式
                    set_non_blocking(client_socket_fd);

                    // (关键) 将新客户端的 FD 添加到 epoll 的监视列表
                    ep_event.events =
                        EPOLLIN; // 我们关心这个客户端的 "可读" 事件
                    ep_event.data.fd = client_socket_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket_fd,
                                  &ep_event) == -1) {
                        perror("epoll_ctl(multi_connection_epoll): "
                               "client_socket_fd");
                        close(client_socket_fd);
                    }
                }
            } else {
                // --- 10. 事件类型 2：是 "客户端FD" 就绪吗？ ---
                // 这表示 "老客户端发来了数据"
                int client_socket_fd = ep_events[i].data.fd;

                // 检查是否是 "可读" 事件 (EPOLLIN)
                // (epoll 还会报告 EPOLLERR, EPOLLHUP 等错误事件)
                if (ep_events[i].events & EPOLLIN) {
                    char read_buf[1024];
                    memset(read_buf, 0, sizeof(read_buf));

                    // 因为 client_fd 是非阻塞的，我们开始读取
                    // (在 ET 模式下，这里必须用 while 循环读，直到 EAGAIN。
                    //  在 LT 模式下，读一次是安全的，但效率稍低)
                    // 为简单起见，我们这里只读一次
                    ssize_t recv_count =
                        recv(client_socket_fd, read_buf, sizeof(read_buf), 0);
                    if (recv_count < 0) {
                        // 和 accept 一样，EAGAIN/EWOULDBLOCK 不是错误，
                        // 只表示“缓冲区数据已读完”
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            perror("recv(multi_connection_epoll)");
                            // 发生真错误，关闭客户端
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket_fd,
                                      NULL);
                            close(client_socket_fd);
                        }
                    } else if (recv_count == 0) {
                        // 客户端主动断开 (recv 返回 0)
                        printf("客户端 %d 已断开连接\n", client_socket_fd);
                        // (关键)从 epoll 中移除并关闭
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket_fd,
                                  NULL);
                        close(client_socket_fd);
                    } else {
                        // 成功收到了数据
                        if (read_buf[recv_count - 1] == '\n') {
                            read_buf[recv_count - 1] = '\0';
                        } else
                            read_buf[recv_count] = '\0';
                        printf("客户端%d发送消息:%s\n", client_socket_fd,
                               read_buf);
                        fflush(stdout);

                        // 回复 "收到！"
                        // (注意：send 也应该是非阻塞的，并处理
                        // EAGAIN/EWOULDBLOCK
                        char write_buf[] = "收到！\n";

                        // 我们简单地尝试一次性发送
                        if (send(client_socket_fd, write_buf, strlen(write_buf),
                                 0) == -1) {
                            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                                // 发生真正的 send 错误
                                perror("send");
                            }
                        }
                    }
                }
            }
        }
    }
    // --- 步骤 4：清理 ---
    printf("服务器关闭。\n");
    close(epoll_fd);
    // 关闭 epoll 实例
    close(server_socket_fd); // 关闭监听套接字
}