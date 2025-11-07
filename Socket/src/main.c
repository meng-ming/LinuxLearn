#include "socket.h"

int main(int argc,const char* argv[])
{
    system("clear");

    // num_endianess_convert();
    // inet_endianess_convert();
    // socket_test_server();
    // socket_test_client();
    // multi_connection_threads();
    // multi_connection_processes();
    // UDP_test_client();
    // UDP_test_server();
    // socket_ipc_test(argc,argv);
    multi_connection_epoll();
    return 0;
}