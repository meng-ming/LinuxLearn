#include "head.h"

int main(int argc, char const* argv[])
{
    // system("clear");
    // test();
    // fork_test();
    // fork_fd_test();
    // if (argc < 2)
    // {
    //     printf("当前参数过少，重新输入！\n");
    //     return 1;
    // }
    // printf("成功跳转到目标程序：我是%s 当前进程号：%d
    // 父进程号：%d\n", argv[1], getpid(), getppid()); execve_test();
    // execve_fork_test();
    // waitpid_test();
    // pstree_test();
    // unnamed_pipe_test(argc, argv);
    // fifo_write_test();
    // fifo_read_test();
    // shared_memory_test();
    // message_queue_test();
    // message_queue_write();
    // message_queue_receive();
    signal_test();
    return 0;
}