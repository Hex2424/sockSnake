#include "networking.h"
#include "../logger/logger.h"
#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

const static char* TAG = "NETWORKING"; 

bool Socket_init(void)
{
    #if defined(WINDOWS)
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
        {
            return false;
        }
    #elif defined(LINUX)
        // Nothing for LINUX :D
    #endif

    return true;
}

Socket_t Socket_createSocketTCP(void)
{
    Socket_t initedSocket;
    #if defined(WINDOWS)
        initedSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(initedSocket == INVALID_SOCKET)
        {
            initedSocket = -1;
        }

    #elif defined(LINUX)
        initedSocket = socket(AF_INET, SOCK_STREAM, 0);
    #endif
    
    return initedSocket;
}

Socket_t Socket_createSocketUDP(void)
{
    Socket_t initedSocket;
    #if defined(WINDOWS)
        initedSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(initedSocket == INVALID_SOCKET)
        {
            initedSocket = -1;
        }

    #elif defined(LINUX)
        initedSocket = socket(AF_INET, SOCK_DGRAM, 0);
    #endif
    
    return initedSocket;
}


bool Socket_bindTCP(const Socket_t socket, const SockAddrHandle_t sockAddr)
{
    #if defined(WINDOWS)
        return bind(socket, (SOCKADDR *) sockAddr, sizeof(SockAddr_t)) != SOCKET_ERROR;
    #elif defined(LINUX)
        return bind(socket, (const struct sockaddr*) sockAddr, (socklen_t) sizeof(SockAddr_t)) >= 0;
    #endif
}

bool Socket_listenTCP(const Socket_t listenSocket, const int backlogCount)
{
    #if defined(WINDOWS)
        return listen(listenSocket, SOMAXCONN) != SOCKET_ERROR;
    #elif defined(LINUX)
        return listen(listenSocket, backlogCount) >= 0;
    #endif
}

Socket_t Socket_acceptSocketTCP(const Socket_t sockenForAccepting, const SockAddrHandle_t sockAddr)
{
    Socket_t newSocket;
    #if defined(WINDOWS)
        newSocket = accept(ListenSocket, NULL, NULL);
        if(newSocket == INVALID_SOCKET)
        {
            newSocket = -1;
        }

    #elif defined(LINUX)
        socklen_t addrLen = sizeof(SockAddr_t);
        return accept(sockenForAccepting, (struct sockaddr*)&sockAddr, &addrLen);
    #endif
}


bool Socket_connectSocketTCP(const Socket_t socket, const SockAddrHandle_t sockAddr)
{
    #if defined(WINDOWS)
        return connect(socket, (SOCKADDR *)&sockAddr, sizeof(SockAddr_t)) != SOCKET_ERROR
    #elif defined(LINUX)
        return connect(socket, (struct sockaddr *)sockAddr, sizeof(SockAddr_t)) == 0;
    #endif    
    
}



bool Socket_sendFullPacketTCP(const Socket_t socket, const void* packetBuffer, const uint8_t packetLength)
{
    assert(packetBuffer);
    assert(packetLength > 0);

    return send(socket, packetBuffer, packetLength, 0);

}

bool Socket_readFullPacketTCP(const Socket_t socket, char* packetBuffer, const uint8_t packetLength)
{
    char* packetBufferEnd;

    assert(packetBuffer);

    packetBufferEnd = packetBuffer + packetLength;

    while (packetBuffer < packetBufferEnd) 
    {
        uint8_t readedBytes = recv(socket, packetBuffer, (packetBufferEnd - packetBuffer), 0);
        if(readedBytes >= 0)
        {
            packetBuffer += readedBytes;
        }else
        {
            Log_e(TAG, "Failed to read packet fully");
            return false;
        }

    }

    return true;
}

bool Socket_close(const Socket_t socket)
{
    #if defined(WINDOWS)
        closesocket(socket);
        WSACleanup();
    #elif defined(LINUX)
        return close(socket) != -1;
    #endif
}