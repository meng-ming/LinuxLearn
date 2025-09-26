#ifndef HEAD_H_
#define HEAD_H_

#include <errno.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void test();
void fork_test(); // 测试 fork 创建子进程
void fork_fd_test(); // 通过 fork 创建子进程之后，父子进程均写入文件
void execve_test(); // 父进程产生子进程之后，子进程执行可执行程序
void execve_fork_test(); // 父进程产生子进程之后，子进程执行可执行程序，父进程继续往下执行
void waitpid_test(); // 父进程等待某进程完成之后再进行，防止孤儿进程，父进程结束太快，子进程找不到原本的父进程
void pstree_test();  // 通过一直挂起父进程，使用 pstree -p
                     // 命令查看进程树
// 进程间使用管道进行通讯  只能单方面进行通讯，父给子或者子给父
void unnamed_pipe_test(
    int argc,
    char const* argv[]); // 通过匿名管道，使得父子进程之间能够通信
// 两个程序之间通过有名管道进行通讯
void fifo_write_test(); // 写入有名管道操作
void fifo_read_test();  // 从有名管道读取信息操作
// 进程间使用共享内存进行通讯
void shared_memory_test(); //
void message_queue_test(); // 父子进程间使用消息队列进行通讯（先根据优先级进行消息排序，然后再先进先出）
// 两个程序之间通过消息队列进行通讯
void message_queue_write();   // 对消息队列进行写入信息
void message_queue_receive(); // 读取消息队列中的信息
void signal_callback(int signal_num); // 信号回调函数
void signal_test();                   // 测试信号的作用
#endif                                // HEAD_H_