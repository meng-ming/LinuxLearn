#include "thread.h"

int main(int argc, char* argv[])
{
    system("clear");
    // thread_create_test();
    // thread_exit_test();
    // thread_detach_test();
    // thread_cancel_deferred_test();
    // thread_cancel_async_test();
    // thread_race_condition_test();
    // thread_rwlock_test();
    // thread_cond_sem_test();
    thread_pool_test();

    return 0;
}