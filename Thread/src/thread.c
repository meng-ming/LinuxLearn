#include "thread.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char buf[BUF_MAX_LEN];
int num = 0;
int count = 0;

// 通常对全局或者静态锁变量使用宏定义 PTHREAD_MUTEX_INITIALIZER
// 初始化，不能改变其配置属性；使用完全不用显示销毁该锁
pthread_mutex_t num_accumulate = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t num_add;
// 与上同理
// pthread_rwlock_t num_add = PTHREAD_RWLOCK_INITIALIZER;

// 测试条件信号全局变量
pthread_cond_t not_full;
pthread_cond_t not_empty;
pthread_mutex_t cond_mutex;
int count;

// 信号量测试
sem_t unname_sem;
sem_t full;
sem_t empty;
int sem_buf;

int shm_fd;
shared_data* data;
sem_t *sem_empty, *sem_full;
char *sem_empty_fd = "/sem_empty", *sem_full_fd = "sem_full";

void* thread_input(void* argv)
{
    // 不断从控制台读取信息存储到缓存中，一个字符一个字符读取
    int i = 0;
    while (1) {
        // 获取控制台输入的每个字符
        char c = fgetc(stdin);
        // 当遇到空白字符或者 '\n' 时，不读取
        if (c && c != '\n') {
            buf[i++] = c;
        }
        // 读满缓存区之后，循环覆盖缓存区
        if (i >= BUF_MAX_LEN) {
            i = 0;
        }
    }
}

void* thread_output(void* argv)
{
    // 将缓存中存储的字符一个一个读取输出到控制台上
    int i = 0;
    while (1) {
        if (buf[i]) {
            // 将字节输出到控制台
            fputc(buf[i], stdout);
            fputc('\n', stdout);
            buf[i++] = 0;
            // 如果读取到最大下标
            if (i >= BUF_MAX_LEN) {
                i = 0;
            }
        } else {
            // 防止控制台未输入时消耗过多资源
            sleep(1);
        }
    }
}
void thread_create_test()
{
    // 将缓存初始化
    memset(buf, 0, BUF_MAX_LEN);
    pthread_t pid_input;
    pthread_t pid_output;
    // 创建从控制台输入信息到缓存区线程
    pthread_create(&pid_input, NULL, thread_input, NULL);
    // 创建从缓存中读取信息输出到控制台上线程
    pthread_create(&pid_output, NULL, thread_output, NULL);

    // 此处不会退出
    // 主线程等待读写线程结束
    pthread_join(pid_input, NULL);
    pthread_join(pid_output, NULL);
}

void* thread_zhangsan(void* argv)
{
    // 接收参数
    char arg = *((char*)argv);
    // 判断第一个字符是否与参数相等
    if (buf[0] == arg) {
        // 如果相等，返回固定前缀以及对他说的话
        const char* prefix = "你正在与张三对话：";
        int total_len = strlen(prefix) + BUF_MAX_LEN + 1;
        char* answer = malloc(total_len);
        if (!answer) {
            perror("malloc");
            pthread_exit(NULL);
        }
        strcpy(answer, prefix);
        strcat(answer, buf);
        pthread_exit((void*)answer);
    } else {
        printf("你怎么不问张三？\n");
        pthread_exit(NULL);
    }
}

void* thread_lisi(void* argv)
{
    // 接收参数
    char arg = *((char*)argv);
    // 判断第一个字符是否与参数相等
    if (buf[0] == arg) {
        const char* prefix = "你正在与李四对话：";
        int total_len = strlen(prefix) + BUF_MAX_LEN + 1;
        char* answer = malloc(total_len);
        if (!answer) {
            perror("malloc");
            pthread_exit(NULL);
        }
        strcpy(answer, prefix);
        strcat(answer, buf);
        // 线程结束
        pthread_exit((void*)answer);
    } else {
        printf("你怎么不问李四？\n");
        pthread_exit(NULL);
    }
}

void thread_exit_test()
{
    // 创建两个线程，根据控制台输入进行判断
    // 两个线程的 pid 号
    pthread_t pid_zhangsan;
    pthread_t pid_lisi;
    // 传递给两个线程的 参数，通过此参数判断消息发给谁
    char zhangsan_flag = 'z';
    char lisi_flag = 'l';
    // 两个线程返回的结果
    char* zhangsan_result;
    char* lisi_result;
    while (1) {
        // 记录控制台输入
        printf("请输入你想说的话(以'z'/'l'开头)：");
        // 如果控制台没有输入，直接退出
        if (fgets(buf, 100, stdin) == NULL || buf[0] == ' ' || buf[0] == '\n' ||
            buf[0] == '\t')
            break;
        // 通过传递的参数与对应的函数进入线程中
        pthread_create(&pid_zhangsan, NULL, thread_zhangsan, &zhangsan_flag);
        pthread_create(&pid_lisi, NULL, thread_lisi, &lisi_flag);
        //获取两个线程的结果
        pthread_join(pid_zhangsan, (void**)&zhangsan_result);
        if (zhangsan_result) {
            printf("张三线程返回的结果：%s", zhangsan_result);
        }
        pthread_join(pid_lisi, (void**)&lisi_result);
        if (lisi_result) {
            printf("李四线程返回的结果：%s", lisi_result);
        }
    }
}

void* thread_detach(void* argv)
{
    printf("子线程开始！\n");
    sleep(1);
    printf("子线程结束！\n");
    return NULL;
}

void thread_detach_test()
{
    pthread_t pid_detach;
    printf("主线程开始！\n");
    pthread_create(&pid_detach, NULL, thread_detach, NULL);
    printf("主线程继续！\n");
    // 使用 pthread_detach 系统会等待子线程完成之后自动回收相关资源
    pthread_detach(pid_detach);
    // 保证主线程运行完成比子线程慢
    sleep(2);
    printf("主线程结束！\n");
}

void* thread_cancel_deferred(void* argv)
{
    printf("子线程开始！\n");
    // 禁用主线程内的 pthread_cancel 响应
    // pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    // 人为调用取消 点函数
    pthread_testcancel();
    // 不运行下面的代码，直接从当前位置退出子线程
    printf("子线程本身结束！\n");
    return NULL;
}

void thread_cancel_deferred_test()
{
    pthread_t pid_cancel;
    void* cancel_result;
    printf("主线程开始！\n");
    pthread_create(&pid_cancel, NULL, thread_cancel_deferred, NULL);
    printf("主线程继续！\n");
    // 使用 pthread_cancel 会调用子线程中的点函数退出子线程
    if (pthread_cancel(pid_cancel) != 0) {
        perror("pthread_cancel");
    }
    pthread_join(pid_cancel, &cancel_result);
    if (cancel_result == PTHREAD_CANCELED) {
        printf("子线程被主线程取消！\n");
    } else {
        printf("线程还没被取消！\n");
    }
    printf("主线程结束！\n");
}

void* thread_cancel_async(void* argv)
{
    printf("子线程开始！\n");
    // 更改取消类型为异步，表现为随机停止
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    int i = 0;
    while (1) {
        printf("i=%d\n", i++);
    }
    printf("子线程本身结束！\n");
}

void thread_cancel_async_test()
{
    pthread_t pid_cancel;
    void* cancel_result;
    printf("主线程开始！\n");
    pthread_create(&pid_cancel, NULL, thread_cancel_async, NULL);
    printf("主线程继续！\n");
    // 使用 pthread_cancel 会调用子线程中的点函数退出子线程
    if (pthread_cancel(pid_cancel) != 0) {
        perror("pthread_cancel");
    }
    pthread_join(pid_cancel, &cancel_result);
    if (cancel_result == PTHREAD_CANCELED) {
        printf("子线程被主线程取消！\n");
    } else {
        printf("线程还没被取消！\n");
    }
    printf("主线程结束！\n");
}

void* thread_num_add(void* argv)
{
    int choice = *((int*)argv);
    if (choice == 1) {
        for (int i = 0; i < 100000; i++) {
            // 累加之前上锁，确保同一时间只有一个线程能运行下列语句
            pthread_mutex_lock(&num_accumulate);
            num++;
            // 累加完释放锁
            pthread_mutex_unlock(&num_accumulate);
        }
    } else if (choice == 2) {
        for (int i = 0; i < 100000; i++) {
            // 累加之前添加信号量
            sem_wait(&unname_sem);
            num++;
            // 累加完唤醒信号量
            sem_post(&unname_sem);
        }
    } else {
        for (int i = 0; i < 100000; i++) {
            num++;
        }
    }

    return NULL;
}

void thread_race_condition_test()
{
    pthread_t pid_race1, pid_race2;
    int choice;
    // // 动态初始化互斥锁，可以更改锁的属性，锁使用完后需要主动销毁锁
    // pthread_mutex_init(&num_accumulate, NULL);
    printf("请选择添加方式(0:不添加 / 1:添加互斥锁 / 2:添加信号量)：");
    scanf("%d", &choice);
    if (choice == 2) {
        // 初始化信号量
        /*
        int sem_init (sem_t *__sem, int __pshared, unsigned int__value)
        sem_t *__sem: 信号量对象地址
        int __pshared: 使用方式 0：线程间使用 非0：进程间使用
        unsigned int__value: 初始值
        */
        int sem_r = sem_init(&unname_sem, 0, 1);
        if (sem_r == -1) {
            perror("sem_init");
            exit(EXIT_FAILURE);
        }
    }
    // 创建两个线程，对公共变量进行大规模的累加
    pthread_create(&pid_race1, NULL, thread_num_add, &choice);
    pthread_create(&pid_race2, NULL, thread_num_add, &choice);
    // 等待两个线程完成
    pthread_join(pid_race1, NULL);
    pthread_join(pid_race2, NULL);
    // // 线程完成之后销毁锁
    // pthread_mutex_destroy(&num_accumulate);
    // 输出两个线程的累加结果，会远远小于预期值
    if (choice == 2) {
        // 销毁信号量
        if (sem_destroy(&unname_sem) != 0) {
            perror("sem_destroy");
        };
    }
    printf("num = %d (expected %d)\n", num, 2 * 100000);
}

void* thread_rwlock_writer(void* argv)
{
    // 通过是否加读锁区分结果
    thread_writer writer = *((thread_writer*)argv);
    if (writer.use_lock) {
        // 加上写锁
        pthread_rwlock_wrlock(&num_add);
        // 为达到更好的效果，使用中间变量
        int temp = num + 1;
        sleep(1);
        num = temp;
        // 释放锁
        pthread_rwlock_unlock(&num_add);
    } else {
        // 为达到更好的效果，使用中间变量
        int temp = 1;
        sleep(1);
        num += temp;
    }
    printf("当前是%s,num++ = %d\n", writer.thread_name, num);
    return NULL;
}

void* thread_rwlock_reader(void* argv)
{
    // 读线程均加上读锁
    pthread_rwlock_rdlock(&num_add);
    printf("当前是%s,num = %d\n", (char*)argv, num);
    pthread_rwlock_unlock(&num_add);
    return NULL;
}

void thread_rwlock_test()
{
    // 创建两个写线程，六个读线程，其中写线程对公共变量进行加一操作，分为加锁和不加锁版本；读线程每个都是读取公共变量的数值
    // 创建不同线程的变量值
    int thread_writer_num, thread_reader_num, temp, solved_hungry_flag;
    // 使用内置函数对锁对象进行初始化
    printf("请输入是否通过定义锁对象属性解决写饥饿状态：(1:是 / "
           "0:否):\n");
    scanf("%d", &solved_hungry_flag);
    if (solved_hungry_flag) {
        // 创建读写锁属性对象
        pthread_rwlockattr_t attr;
        // 初始化
        pthread_rwlockattr_init(&attr);
        // 修改读写锁属性，设置写优先
        pthread_rwlockattr_setkind_np(
            &attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
        // 显示初始化读写锁
        pthread_rwlock_init(&num_add, &attr);
    } else {
        pthread_rwlock_init(&num_add, NULL);
    }

    // 定义多个线程
    thread_writer writers_info[] = {{"writer1", false},
                                    {"writer2", false},
                                    {"writer3", false},
                                    {"writer4", false}};
    char* reader_names[] = {"reader1", "reader2", "reader3",
                            "reader4", "reader5", "reader6"};
    // 通过动态数组求出对应的线程个数
    thread_writer_num = sizeof(writers_info) / sizeof(writers_info[0]);
    thread_reader_num = sizeof(reader_names) / sizeof(reader_names[0]);
    pthread_t writers[thread_writer_num], readers[thread_reader_num];
    // 与控制台交互
    printf("请输入是否使用读锁(1:加锁 / 0:不加锁)：");
    scanf("%d", &temp);
    for (int i = 0; i < thread_writer_num; i++) {
        writers_info[i].use_lock = (temp != 0);
    }
    // 开始读写线程
    for (int i = 0; i < thread_writer_num; i++)
        pthread_create(&writers[i], NULL, thread_rwlock_writer,
                       &writers_info[i]);

    for (int i = 0; i < thread_reader_num; i++)
        pthread_create(&readers[i], NULL, thread_rwlock_reader,
                       reader_names[i]);
    // 等待读写线程关闭
    for (int i = 0; i < thread_writer_num; i++)
        pthread_join(writers[i], NULL);
    for (int i = 0; i < thread_reader_num; i++)
        pthread_join(readers[i], NULL);
}

void* thread_producer(void* argv)
{
    int choice = *((int*)argv);
    if (choice == 0) {
        // 向缓存中不断写入数据，当缓存写满时触发条件信号，等待缓存没满条件，否则广播缓存不为空条件信号
        int num = 0;
        while (1) {
            pthread_mutex_lock(&cond_mutex);
            // 如果缓存区写满，
            // 使用while，防止虚假唤醒或被其它线程“抢走条件”
            while (count == BUF_MIN_LEN) {
                pthread_cond_wait(&not_full, &cond_mutex);
            }
            buf[count++] = num++;
            // 否则，广播缓存不为空信号，通知消费者缓存不为空
            // 在“可能有多个等待者需要都被唤醒”才用 broadcast。
            pthread_cond_signal(&not_empty);
            printf("生产者写入缓存数据：%d,count = %d\n", buf[count - 1],
                   count);
            pthread_mutex_unlock(&cond_mutex);
            // 模拟生产耗时
            sleep(1);
        }
    } else if (choice == 1) {
        while (1) {
            // 获取信号量 empty，初始值为 1，经过该函数之后 empty
            // 值减1，接着往下运行
            sem_wait(&empty);
            sem_buf = rand();
            printf("生产者写入缓存数据：%d\n", sem_buf);
            // 唤醒信号量 full，初始值为 0，经过该函数之后 full 值加1
            sem_post(&full);
            sleep(1);
        }
    }
    return NULL;
}

void* thread_consumer(void* argv)
{
    int choice = *((int*)argv);
    if (choice == 0) {
        // 从缓存中不断读取数据，当缓存为空时，等待缓存不为空条件信号，否则广播缓存没满条件信号
        while (1) {
            pthread_mutex_lock(&cond_mutex);
            // 如果当前缓存为空
            while (count == 0) {
                pthread_cond_wait(&not_empty, &cond_mutex);
            }
            int data = buf[--count];
            printf("消费者从缓存中取出的数据：%d,count = %d\n", data, count);
            // 广播缓存没满状态条件信号
            pthread_cond_signal(&not_full);
            pthread_mutex_unlock(&cond_mutex);
            // 模拟消费者耗时
            sleep(2);
        }
    } else if (choice == 1) {
        while (1) {
            // 获取信号量 full，初始值为 0，需要等待生产者对缓存进行输入之后，即
            // full=1，才能往下运行代码
            sem_wait(&full);
            sem_buf = rand();
            printf("消费者读取缓存数据！\n");
            // 唤醒信号量 empty，经过生产者之后，empty值从1变为0
            // 通过唤醒之后，empty再次变为1；
            sem_post(&empty);
            sleep(1);
        }
    }
    return NULL;
}

void thread_cond_sem_test()
{
    // 创建消费者和生产者线程
    pthread_t pid_producer, pid_consumer;
    int choice;
    printf("请输入选择(0:互斥锁+条件信号 / 1:信号量):");
    scanf("%d", &choice);
    if (choice == 0) {
        // 初始化互斥锁
        pthread_mutex_init(&cond_mutex, NULL);
        //初始化条件信号
        pthread_cond_init(&not_empty, NULL);
        pthread_cond_init(&not_full, NULL);
    } else if (choice == 1) {
        // 通过信号量设置，保证两个线程之间的顺序一定是
        // 先生产者往缓存中 写入数据，然后再消费者从缓存中取出数据
        // 设置随机数种子，保证每次输入缓存的数不同
        srand(time(NULL));
        // 初始化信号量
        sem_init(&full, 0, 0);
        sem_init(&empty, 0, 1);
    }

    pthread_create(&pid_producer, NULL, thread_producer, &choice);
    pthread_create(&pid_consumer, NULL, thread_consumer, &choice);

    // 等待子线程运行完成
    pthread_join(pid_producer, NULL);
    pthread_join(pid_consumer, NULL);
    if (choice == 0) {
        // 销毁互斥锁
        pthread_mutex_destroy(&cond_mutex);
        // 销毁条件信号
        pthread_cond_destroy(&not_empty);
        pthread_cond_destroy(&not_full);
    } else if (choice == 1) {
        // 销毁信号量
        sem_destroy(&full);
        sem_destroy(&empty);
    }
}

void thread_producer_sem()
{
    signal(SIGINT, cleanup_and_exit); // 捕获 Ctrl+C，清理资源

    // 1. 打开/创建共享内存
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_data)); // 设置共享内存大小
    data = mmap(0, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED,
                shm_fd, 0);
    data->in = data->out = 0; // 初始化循环队列指针

    // 2. 打开/创建有名信号量
    // empty 表示缓存区剩余的空位数，初始值 = BUF_MIN_LEN
    // full  表示缓存区已有的数据数，初始值 = 0
    sem_empty = sem_open(sem_empty_fd, O_CREAT, 0666, BUF_MIN_LEN);
    sem_full = sem_open(sem_full_fd, O_CREAT, 0666, 0);

    int item = 1; // 从 1 开始生产
    while (1) {
        sem_wait(sem_empty); // 如果没有空位，阻塞等待

        // 将数据写入共享内存的缓冲区
        data->buffer[data->in] = item;
        printf("生产者生产: %d\n", item);
        data->in = (data->in + 1) % BUF_MIN_LEN; // 环形缓冲写指针移动

        item = (item % BUF_MIN_LEN) + 1; // 数据循环在 1~BUF_MIN_LEN 之间

        sem_post(sem_full); // 通知消费者，有新数据可以消费

        sleep(1); // 模拟生产耗时
    }
}

void thread_consumer_sem()
{
    signal(SIGINT, cleanup_and_exit); // 捕获 Ctrl+C，清理资源

    // 1. 打开已存在的共享内存
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    data = mmap(0, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED,
                shm_fd, 0);

    // 2. 打开已存在的有名信号量
    sem_empty = sem_open(sem_empty_fd, 0);
    sem_full = sem_open(sem_full_fd, 0);

    while (1) {
        sem_wait(sem_full); // 如果没有数据，阻塞等待

        // 从共享内存的缓冲区取出数据
        int item = data->buffer[data->out];
        printf("消费者消费: %d\n", item);
        data->out = (data->out + 1) % BUF_MIN_LEN; // 环形缓冲读指针移动

        sem_post(sem_empty); // 通知生产者，有空位可写

        sleep(2); // 模拟消费耗时
    }
}

void cleanup_and_exit(int signo)
{
    printf("\n[清理] 捕获到 Ctrl+C，正在释放资源...\n");

    if (data) munmap(data, sizeof(shared_data));
    if (shm_fd > 0) close(shm_fd);
    shm_unlink(SHM_NAME);

    if (sem_empty != SEM_FAILED) {
        sem_close(sem_empty);
        sem_unlink(sem_empty_fd);
    }
    if (sem_full != SEM_FAILED) {
        sem_close(sem_full);
        sem_unlink(sem_full_fd);
    }

    printf("[清理] 完成，进程退出。\n");
    exit(0);
}
void thread_pool_func(gpointer data, gpointer user_data)
{
    int task_num = *(int*)data;
    // 每个任务结束时释放掉它自己的编号内存，不会造成内存泄漏。
    free(data);
    printf("开始执行任务%d\n", task_num);
    sleep(task_num);
    printf("任务%d执行完成!\n", task_num);
}

void thread_pool_test()
{
    // 创建线程池
    GThreadPool* pool =
        g_thread_pool_new(thread_pool_func, NULL, 5, TRUE, NULL);
    // 向线程池中添加任务
    for (int i = 0; i < 10; i++) {
        // 每个提交任务的编号
        // 每个任务都需要一份独立的任务编号，而堆内存分配能保证在整个任务生命周期中该值不会被其他任务覆盖。
        int* task_num = (int*)malloc(sizeof(int));
        *task_num = i + 1;
        g_thread_pool_push(pool, task_num, NULL);
    }

    // 销毁线程池
    g_thread_pool_free(pool, FALSE, TRUE);
    printf("所有任务完成！\n");
}