#ifndef NETWORKING
#define NETWORKING

#include <bits/types/struct_iovec.h>
#include <arpa/inet.h>

#if defined(WINDOWS)
    #include <conio.h>
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#elif defined(LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 1024

#define SOCKET_ERROR        1

#if defined(WINDOWS)
    typedef SOCKET Socket_t;
    typedef        SOCKVECTORS
#elif defined(LINUX)
    typedef int Socket_t;
    typedef struct iovec SockVectors_t; 
    typedef struct msghdr SockMessage_t;
    typedef struct in_addr SockIPv4;
    
#endif

typedef struct sockaddr_in SockAddr_t;
typedef SockAddr_t* SockAddrHandle_t;
typedef SockMessage_t* SockMessageHandle_t;

bool Socket_init(void);
Socket_t Socket_createSocketTCP(void);
Socket_t Socket_createSocketUDP(void);

bool Socket_bind(const Socket_t socket, const SockAddrHandle_t sockAddr);
bool Socket_listenTCP(const Socket_t listenSocket, const int backlogCount);
Socket_t Socket_acceptSocketTCP(const Socket_t sockenForAccepting, const SockAddrHandle_t sockAddr);
bool Socket_connectSocketTCP(const Socket_t socket, const SockAddrHandle_t sockAddr);
bool Socket_readFullPacketTCP(const Socket_t socket, char* packetBuffer, const uint8_t packetLength);
bool Socket_sendFullPacketTCP(const Socket_t socket, const void* packetBuffer, const uint8_t packetLength);

bool Socket_getRemoteIP(const Socket_t socket, SockIPv4* ip);
bool Socket_close(const Socket_t socket);
#endif