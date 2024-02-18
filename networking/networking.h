#ifndef NETWORKING
#define NETWORKING

#define LINUX

#if defined(WINDOWS)
    #include <conio.h>
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#elif defined(LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>


#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 1024

#define SOCKET_ERROR        1

struct Networking
{
    char rxBuf[RX_BUFFER_SIZE];
    char txBuf[TX_BUFFER_SIZE];
    #if defined(WINDOWS)
        SOCKET socket;
    #elif defined(LINUX)
        int socket;
    #endif
};

typedef struct Networking Networking_t;
typedef Networking_t* NetworkingHandle_t;

bool Networking_init();
bool Networking_initializeSocket(NetworkingHandle_t handle);
bool Networking_connectSocket(NetworkingHandle_t handle, const char* serverAddress, const uint32_t port);
int Network_read(NetworkingHandle_t handle);
int Network_write(NetworkingHandle_t handle, size_t lengthToWrite);
int Network_close(NetworkingHandle_t handle);
int Network_readN(NetworkingHandle_t handle, const int n);
#endif