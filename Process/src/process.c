#include "head.h"

#include <bits/time.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void test()
{
    int sys_result = system("ping -c 10 www.baidu.com");
    if (sys_result != 0) {
        perror("system");
    }
}

void fork_test()
{
    // 创建子进程之前都在父进程中进行
    printf("父进程的进程号：%d\n", getpid());
    // 使用fork创建子进程后
    /*
        __pid_t fork (void):
        不需要传参
        返回值：
            (1) ：-1 出错
            (2) ：父进程中子进程的PID
            (3) ：子进程中显示 0
    */
    pid_t pid = fork();
    // fork 之后所有的代码都会在父子进程中各自运行一遍
    printf("进程号：%d\n", pid);
    // 根据 pid 的值判断当前是父还是子进程
    if (pid < 0) {
        perror("创建子进程失败！\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 当前为子进程
        // 子进程单独执行代码
        printf("父进程号：%d,当前进程号:%d\n", getppid(), getpid());
    } else {
        // 当前为父进程
        // 父进程单独执行代码
        printf("当前进程号:%d,子进程号:%d\n", getpid(), pid);
    }
}

void fork_fd_test()
{
    // 父进程创建一个文件并给予写入权限
    char* filename = "fork_fd.txt";
    char* buff;
    int fd = open(
        filename, O_CREAT | O_WRONLY | O_APPEND,
        0644); // 文件不存在，需要创建并基于写入权限，由于两个进程写入，使用追加方式，同时文件权限设置为
               // 644

    // 异常处理
    if (-1 == fd) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    // 创建子进程
    // 使用fork之后，文件引用计数从 1 变成
    // 2，只有当引用计数变为0时，才不能对文件进行操作 close(fd)
    // 只减少一次引用计数
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (0 == pid) {
        // 子进程单独处理
        buff = "当前为子进程，写入数据！\n";
    } else {
        sleep(1); // 延迟一秒，防止两个进程同时写入
        // 父进程单独处理
        buff = "当前为父进程，写入数据！\n";
    }
    int bytes_write = write(fd, buff, strlen(buff));

    // 写入异常处理
    if (-1 == bytes_write) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("写入数据成功！\n");
    // 正常写入之后
    close(fd);
    if (pid == 0) {
        printf("子进程写入完毕！\n");
    } else {
        printf("父进程写入完毕！\n");
    }
}

void execve_test()
{
    char* name = "憨憨宝！";
    // 此处一定需要 '\n'
    //（1）printf
    //并不是直接把内容马上输出到屏幕，而是先写到一个“缓冲区”里，等到遇到换行符
    // \n，缓冲区满了，或程序结束/flush 时才输出到屏幕。
    //（2）当调用 execve
    //后，如果跳转成功，当前进程的代码段被新程序覆盖，之前的缓冲还没输出就直接消失了。
    printf("我是%s 当前进程号：%d 父进程号：%d\n", name, getpid(),
           getppid());
    /*
        int execve (const char *__path, char *const __argv[],char
       *const __envp[]) const char *__path:
       执行程序跳转的路径（可执行程序保存的路径） char *const
       __argv[]: 给可执行程序提供的参数，对应 main(int argc,char const
       *argv[])中的 argv[] 第一个参数：可执行程序保存的路径
            第二个参数：跳转的可执行程序需要传入的参数
            第三个参数：固定为NULL
        char *const __envp[]: 传递的环境变量
            第一个参数: key = value
            第二个参数：NULL
        return int：
            成功时：无返回值，进入可执行程序，execve()后的语句无意义
            失败时：返回 -1
*/
    char* argv[] = {"/home/zjm/LinuxLearn/Process/bin/execve", name,
                    NULL};
    char* envps[] = {NULL};
    // 执行跳转
    int result = execve(argv[0], argv, envps);
    if (result == -1) {
        printf("跳转可执行程序失败！\n");
        exit(EXIT_FAILURE);
    }
    printf("测试跳转之后是否执行语句！\n");
}

void execve_fork_test()
{
    // 父进程产生子进程之后，子进程执行可执行程序，父进程继续往下执行
    char* name = "父进程";
    char* child_name = "可执行程序";
    printf("你好 %s,当前进程号：%d\n", name, getpid());

    // 创建子进程
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 当前为子进程，跳转到目标执行程序
        char* argv[] = {"/home/zjm/LinuxLearn/Process/bin/execve",
                        child_name, NULL};
        char* envps[] = {NULL};
        int execve_result = execve(argv[0], argv, envps);
        if (execve_result == -1) {
            // 跳转可执行程序失败
            printf("跳转失败！");
            exit(EXIT_FAILURE);
        }
    } else {
        // 父进程仍然继续往下执行
        sleep(1); // 防止父进程关闭太快，导致子进程找不到该父进程
        printf("当前为：%s,进程号为：%d,子进程号为：%d\n", name,
               getpid(), pid);
    }
}

void waitpid_test()
{
    int sub_process_status;
    // 父进程等待某进程完成之后再进行，防止孤儿进程，父进程结束太快，子进程找不到原本的父进程，会不断找父进程的父进程，直到一个稳定的父进程
    printf("主进程开始！\n");
    pid_t pid = fork(); // 创建子进程
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 子进程跳转到 ping 程序
        printf("当前为子进程！\n");
        printf("跳转到可执行程序，当前进程号：%d,父进程号：%d\n",
               getpid(), getppid());
        char* argv[] = {"/usr/bin/ping", "-c", "15", "www.baidu.com",
                        NULL};
        char* envps[] = {NULL};
        int execve_result = execve(argv[0], argv, envps);
        if (execve_result == -1) {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    } else {
        // 父进程
        printf("当前为父进程！\n");
        printf("当前进程号：%d,子进程号：%d\n", getpid(), pid);
        // 通过 waitpid()
        // 等待子进程完成，保证先执行父进程号，回收子进程
        waitpid(pid, &sub_process_status, 0);
    }
    printf("子进程执行完毕！当前为父进程，进程号：%d\n", getpid());
}

void pstree_test()
{
    char* name = "父进程";
    char* child_name = "可执行程序";
    printf("你好 %s,当前进程号：%d\n", name, getpid());

    // 创建子进程
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 当前为子进程，跳转到目标执行程序
        char* argv[] = {"/home/zjm/LinuxLearn/Process/bin/execve",
                        child_name, NULL};
        char* envps[] = {NULL};
        int execve_result = execve(argv[0], argv, envps);
        if (execve_result == -1) {
            // 跳转可执行程序失败
            printf("跳转失败！");
            exit(EXIT_FAILURE);
        }
    } else {
        // 父进程仍然继续往下执行
        printf("当前为：%s,进程号为：%d,子进程号为：%d\n", name,
               getpid(), pid);
        fgetc(
            stdin); // 只有输入一个字符并回车才会结束此命令，否则一直挂起
        exit(EXIT_SUCCESS);
    }
}

void unnamed_pipe_test(int argc, char const* argv[])
{
    int pipefd[2]; // pipe 参数，大小为 2 的 int 数组，储存的是
                   // 文件描述符
    pid_t child_pid;
    // 先进行参数个数判断
    if (argc < 2) {
        printf("需要输入参数！\n");
        exit(EXIT_FAILURE);
    }
    // 创建一个公共的管道，父子进程公用此管道进行通讯
    // 判断创建管道是否成功
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    // 创建子进程
    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        // 子进程通过匿名管道与父进程进行通讯
        printf("当前为子进程！进程号：%d,父进程号：%d\n", getpid(),
               getppid());
        // 关闭写管道操作
        close(pipefd[1]);
        // 读取管道内部信息，打印到控制台
        char buff;
        while (read(pipefd[0], &buff, 1) > 0) {
            write(STDOUT_FILENO, &buff, 1);
        }
        // 打印换行操作
        write(STDOUT_FILENO, "\n", 1);
        // 关闭读管道操作
        close(pipefd[0]);
        // 退出子进程，直接清除缓存
        _exit(EXIT_SUCCESS);
    } else {
        // 父进程对匿名管道进行写入操作，提供给子进程读
        printf("当前为父进程！进程号：%d,子进程号：%d\n", getpid(),
               child_pid);
        // 当前需对管道进行写操作，关闭读管道操作
        close(pipefd[0]);
        // 将传进来的第一个参数进行写入
        write(pipefd[1], argv[1], strlen(argv[1]) + 1);
        // 关闭写入操作
        close(pipefd[1]);
        // 等待子进程结束,无需查看返回状态，直接设为 NULL
        waitpid(child_pid, NULL, 0);
        // 子进程运行结束之后退出程序
        exit(EXIT_SUCCESS);
    }
}

void fifo_write_test()
{
    char buff[100];
    int bytes_read;
    // 头文件 <sys/stat.h>
    // 创建 fifo
    // 文件路径，从有名管道读和写均使用此路径，若路径相同，则可通信，像
    // ROS 中的消息机制
    char* fifo_file_path = "fifo_test";
    // 判断是否创建成功
    if (mkfifo(fifo_file_path, 0664) != 0) {
        perror("mkfifo");
        if (errno != 17) {
            exit(EXIT_FAILURE);
        }
    }
    // 对有名管道创建的特殊文件进行写操作
    int fd = open(fifo_file_path, O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    // 使用控制台进行输入，并且将输入的数据存储到有名管道中
    while ((bytes_read = read(STDIN_FILENO, buff, 100)) > 0) {
        write(fd, buff, bytes_read);
    }
    // 判断异常
    if (bytes_read < 0) {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    // 写入完成
    printf("有名管道写入完成！\n");
    close(fd);
    // 释放管道，清楚对应的特殊文件，否则下次执行会出现异常
    if (unlink(fifo_file_path) == -1) {
        perror("unlink");
    }
}

void fifo_read_test()
{
    char buff[100];
    int bytes_read;
    // fifo
    // 文件路径，从有名管道读和写均使用此路径，若路径相同，则可通信，像
    // ROS 中的消息机制
    char* fifo_file_path = "fifo_test";

    // 对有名管道创建的特殊文件进行读操作
    int fd = open(fifo_file_path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    // 读取有名管道中的数据
    while ((bytes_read = read(fd, buff, 100)) > 0) {
        write(STDOUT_FILENO, buff, bytes_read);
    }
    // 判断异常
    if (bytes_read < 0) {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    // 读取完成
    printf("有名管道读取完成！\n");
    close(fd);
}

void shared_memory_test()
{
    // 1.创建共享内存对象(类似于open函数)(头文件<sys/mman.h>)
    /*int shm_open(const char* __name, int __oflag, mode_t __mode)
    const char* __name:必须以"/"开头，以"\0"结尾的字符串，且独一无二
    int __oflag：只读 O_RDONLY 或者 只写 O_WRONLY...
    mode_t __mode: 不同用户读写权限
*/
    char shm_name[100];
    sprintf(shm_name, "/shm_name%d", getpid());
    int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0644);
    // 如果创建失败
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    // 2.设置共享内存大小
    ftruncate(shm_fd, 1024);

    // 3.内存映射
    char* shared_memory = mmap(NULL, 1024, PROT_READ | PROT_WRITE,
                               MAP_SHARED, shm_fd, 0);
    // 判断内存映射是否失败
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    // 内存映射完成 关闭fd连接，非释放
    close(shm_fd);

    // 4.使用内存映射实现进程间通讯
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 子进程输入信息到共享内存中
        strcpy(shared_memory, "你好呀！憨憨宝!\n");
        printf("当前是子进程:%d,父进程为:%d,发送消息！\n", getpid(),
               getppid());
    } else {
        // 父进程读取共享内存中的信息
        printf("当前是父进程:%d,子进程为:%d,接收消息！\n", getpid(),
               pid);
        // 等待子进程完成
        waitpid(pid, NULL, 0);
        // 打印子进程存储的消息
        printf("父进程中打印子进程存储的消息:%s", shared_memory);
        // 5.操作完之后释放映射区
        int munmap_result = munmap(shared_memory, 1024);
        if (munmap_result == -1) {
            perror("munmap");
            exit(EXIT_FAILURE);
        }
    }
    // 6.释放共享内存对象
    shm_unlink(shm_name);
}

void message_queue_test()
{
    int message_size = 1000;
    // 消息队列结构体
    struct mq_attr attr;
    // 初始化消息队列结构体，防止奇怪信息
    memset(&attr, 0, sizeof(attr));
    // 在创建消息队列时有用参数
    attr.mq_maxmsg = 10; // 队列可容纳的“消息条数”的上限
    attr.mq_msgsize = message_size; // 单条消息的最大字节数
    // 被忽略的消息(此处可以不赋值)
    attr.mq_curmsgs =
        0; //当前队列中“已排队”的消息条数（只读，由内核维护）
    attr.mq_flags = 0; // 队列/描述符的标志位（目前只用到 O_NONBLOCK）

    char* mq_name = "/message_queue_test";
    // 1.创建消息队列(头文件 <mqueue.h>)
    mqd_t mqdes = mq_open(mq_name, O_RDWR | O_CREAT, 0664, &attr);
    if (mqdes == -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // 创建父子进程
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        mq_close(mqdes);
        mq_unlink(mq_name);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 子进程，接收消息队列中的信息
        // 接收区必须足够大
        char receive_buf[message_size];
        while (1) {
            // 初始化接收消息数组
            memset(receive_buf, 0, sizeof(receive_buf));
            // 创建接收消息时间结构体对象
            struct timespec time_receive;
            clock_gettime(CLOCK_REALTIME, &time_receive);
            time_receive.tv_sec += 5; // 与 time_send 功能类似
            // 接收消息队列中的消息
            // mq_receive/mq_timedreceive时，传入的缓冲区大小（第三个参数）必须大于等于消息队列的
            // mq_msgsize，否则就会报Message too
            // long，errno=EMSGSIZE。
            if (mq_timedreceive(mqdes, receive_buf, message_size, 0,
                                &time_receive) == -1) {
                perror("mq_timedreceive");
                break;
            }
            // 打印消息
            printf("子进程%d收到消息%s", getpid(), receive_buf);
            // 如果接收到特殊消息 "Quit\n" 则退出
            if (!strcmp(receive_buf, "Quit\n")) break;
        }
        // 父子进程均需要释放消息队列描述符
        mq_close(mqdes);
    } else {
        // 父进程，发送信息到消息队列中
        char send_buf[100];
        struct timespec time_send;

        // 不断发送消息
        for (int i = 1; i <= 15; i++) {
            // 初始化消息
            memset(send_buf, 0, sizeof(send_buf));
            if (i == 15) {
                strcpy(send_buf, "Quit\n");
            } else {
                sprintf(send_buf, "父进程%d发送第%d条消息！\n",
                        getpid(), i);
            }
            clock_gettime(CLOCK_REALTIME, &time_send);
            // time_send 指定“超时时间点”
            time_send.tv_sec += 5;
            // 发送消息
            // 如果队列满了，mq_timedsend 会阻塞直到有空位或者到达
            // time_send 指定的时刻，超过就返回失败（-1）,errno 设为
            // ETIMEDOUT。
            if (mq_timedsend(mqdes, send_buf, strlen(send_buf) + 1, 0,
                             &time_send) == -1) {
                perror("mq_timedsend");
            } else {
                printf("父进程成功发送消息！\n");
            }
            // 防止发送消息过快，每发送一条消息休眠1s
            sleep(1);
        }

        // 父子进程均需要释放消息队列描述符
        mq_close(mqdes);
        // 通常只让父进程清理
        mq_unlink(mq_name);
    }
}

void message_queue_write()
{
    // 创建消息队列
    int message_size = 100;
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_msgsize = message_size;
    attr.mq_maxmsg = 10;
    char* mq_name = "/message_queue";
    mqd_t mq_des = mq_open(mq_name, O_RDWR | O_CREAT, 0664, &attr);

    if (mq_des == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    char write_buf[message_size];
    struct timespec time_send;
    // 对消息队列进行写入
    while (1) {
        // 清空消息缓存区
        memset(write_buf, 0, sizeof(write_buf));
        // 从控制台读取信息到 write_buf 中
        int read_count = read(STDIN_FILENO, write_buf, 100);
        // 获取当前时间
        clock_gettime(CLOCK_REALTIME, &time_send);
        time_send.tv_sec += 5;
        // 读取错误
        if (read_count == -1) {
            perror("read");
            continue;
        } else if (read_count == 0) {
            // 从控制台接收到停止发送消息 Ctrl + D
            // 发送一条结束的消息
            printf("停止发送！");
            strcpy(write_buf, "Quit\n");
            if (mq_timedsend(mq_des, write_buf, strlen(write_buf) + 1,
                             0, &time_send) == -1) {
                perror("mq_timedsend");
            }
            // 结束发送
            break;
        }
        // 正常从控制台接收信息
        if (mq_timedsend(mq_des, write_buf, strlen(write_buf) + 1, 0,
                         &time_send) == -1) {
            perror("mq_timedsend");
        }
    }

    // 发送完消息释放消息队列描述符
    mq_close(mq_des);
}

void message_queue_receive()
{
    // 创建消息队列
    int message_size = 100;
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_msgsize = message_size;
    attr.mq_maxmsg = 10;
    char* mq_name = "/message_queue";
    mqd_t mq_des = mq_open(mq_name, O_RDWR | O_CREAT, 0664, &attr);

    if (mq_des == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    char read_buf[message_size];
    struct timespec time_send;
    // 对消息队列进行写入
    while (1) {
        clock_gettime(CLOCK_REALTIME, &time_send);
        time_send.tv_sec += 5;
        if (mq_timedreceive(mq_des, read_buf, message_size, 0,
                            &time_send) == -1) {
            perror("mq_timedreceive");
            //遇到致命错误（比如ETIMEDOUT、EAGAIN之外的错误）直接break退出
            if (errno != ETIMEDOUT && errno != EAGAIN) break;
        }

        // 正常从消息队列中接收信息
        // 匹配到退出消息
        if (!strcmp(read_buf, "Quit\n")) {
            printf("当前程序准备退出！\n");
            break;
        } else {
            printf("接收到的消息：%s", read_buf);
        }
    }

    // 接收完消息释放消息队列描述符
    mq_close(mq_des);
    // 释放消息队列对象
    mq_unlink(mq_name);
}

void signal_callback(int signal_num)
{
    printf("\n收到%d信号，停止程序！\n", signal_num);
    exit(signal_num);
}

void signal_test()
{
    // 主程序不断输出 “你好” ，当按下 Ctrl+C
    // 时，会跳入信号处理回调函数中进行相应的处理
    if (signal(SIGINT, signal_callback) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    while (1) {
        sleep(1);
        printf("你好！\n");
    }
}