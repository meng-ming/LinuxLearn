#ifndef THREAD_H_
#define THREAD_H_

#include <fcntl.h>
#include <glib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define BUF_MAX_LEN 1024        // 缓存最大内存
#define BUF_MIN_LEN 5           // 缓存最小内存，便于条件信号测试
#define SHM_NAME "/shm_sem_buf" // 用于测试不同进程间的共享内存

typedef struct {
    int buffer[BUF_MIN_LEN];
    int in;
    int out;
} shared_data;

extern int num;               // 累加公共变量
extern char buf[BUF_MAX_LEN]; // thread_create_test 中线程共有的缓存区
extern pthread_mutex_t num_accumulate; // 互斥锁
extern pthread_rwlock_t num_add;       // 读写锁
// thread_cond_sem_test
extern pthread_cond_t not_full;  // 表示当前缓存没满的条件锁
extern pthread_cond_t not_empty; // 表示当前缓存不为空的条件锁
extern pthread_mutex_t cond_mutex;
extern int count;   // 当前缓存存入的数据个数
extern int sem_buf; // 使用信号量时 测试的缓存
extern sem_t full;  // 表示当前缓存已满
extern sem_t empty; // 表示当前缓存为空
// 不同进程间使用有名信号量+共享内存
extern int shm_fd;
extern shared_data* data;
extern sem_t *sem_empty, *sem_full;
extern char *sem_empty_fd, *sem_full_fd;

extern sem_t unname_sem; // 匿名信号量

typedef struct {
    char* thread_name; // 线程名字
    bool use_lock;     // 是否使用读锁，1表示使用，0表示不使用
} thread_writer;

int rand_num();            // 生成随机整数
void thread_create_test(); // 创建两个线程，一个线程从控制台往缓存输入，一个线程从缓存读取数据输出到控制台
void* thread_input(void* argv);  // 输入线程处理函数
void* thread_output(void* argv); // 输出线程处理函数
void thread_exit_test(); // 创建两个线程，根据从控制台输入的首字符判断与哪个线程进行链接，并将结果返回到主线程中
void* thread_zhangsan(void* argv);
void* thread_lisi(void* argv);
void thread_detach_test(); // 测试子线程以 pthread_detach()
                           // 结束，主线程不会等待，而是子线程运行完之后系统自动回收
void* thread_detach(void* argv);
void thread_cancel_deferred_test(); // 测试主线程使用 pthread_cancle()
                                    // 主动取消子线程,默认取消类型是
                                    // 延迟
void* thread_cancel_deferred(void* argv);
void thread_cancel_async_test(); // 测试主线程使用 pthread_cancle()
                                 // 主动取消子线程,默认取消类型是
                                 // 异步
void* thread_cancel_async(void* argv);
void thread_race_condition_test(); // 竞态条件例子，两个线程对一个公共变量不断的读写，会导致其中一个线程读到旧值，发生竞态条件，不能达到预期值
                                   // 通过输入选项选择是否加上互斥锁
void* thread_num_add(void* argv);
void thread_rwlock_test(); // 读写锁测试，写进程通过输入选项选择是否加上写锁，读锁保证读线程在读变量时，写进程无法对公共变量进行操作
void* thread_rwlock_writer(void* argv);
void* thread_rwlock_reader(void* argv);
void thread_cond_sem_test(); // 通过消费者和生产者两个线程测试条件变量或者信号量的作用
void* thread_producer(void* argv);
void* thread_consumer(void* argv);
// 有名信号量在不同进程中通讯

void thread_producer_sem();
void thread_consumer_sem();
void cleanup_and_exit(int signo); // Ctrl + C时，自动释放资源

void thread_pool_test(); // 线程池创建
void thread_pool_func(gpointer data,gpointer user_data);
#endif // THREAD_H_