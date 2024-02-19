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

#if defined(WINDOWS)
    typedef SOCKET Socket_t;
#elif defined(LINUX)
    typedef int Socket_t;
#endif

typedef struct sockaddr_in SockAddr_t;
typedef SockAddr_t* SockAddrHandle_t;

bool Socket_init(void);
Socket_t Socket_createSocket(void);
bool Socket_bind(const Socket_t socket, const SockAddrHandle_t sockAddr);
bool Socket_listen(const Socket_t listenSocket, const int backlogCount);
Socket_t Socket_acceptSocket(const Socket_t sockenForAccepting, const SockAddrHandle_t sockAddr);
bool Socket_connectSocket(const Socket_t socket, const SockAddrHandle_t sockAddr);
bool Socket_readFullPacket(const Socket_t socket, char* packetBuffer, const uint8_t packetLength);
bool Socket_sendFullPacket(const Socket_t socket, const void* packetBuffer, const uint8_t packetLength);
bool Socket_close(const Socket_t socket);
#endif