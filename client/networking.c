#include "networking.h"


bool Networking_init()
{
    #if defined(WINDOWS)
        WSADATA wsa;
        printf("\nInitialising Winsock...");
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
        {
            printf("Failed. Error Code : %d.\n", WSAGetLastError());
            return false;
        }
    #elif defined(LINUX)
        // Nothing for LINUX :D
    #endif

    printf("Initialised.\n");
    return true;
}

bool Networking_initializeSocket(NetworkingHandle_t handle)
{
    #if defined(WINDOWS)

        if((handle->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
        {
            printf("Could not create socket : %d.\n", WSAGetLastError());
            WSACleanup();
            return false;
        }

    #elif defined(LINUX)

        if((handle->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("Could not create socket : %d.\n", handle->socket);
            return false;
        }

    #endif

 
    printf("socket initialized:\n");
    return true;
}

bool Networking_connectSocket(NetworkingHandle_t handle, const char* serverAddress, const uint32_t port)
{
    struct sockaddr_in server;
    memset(&server, 0, sizeof server);
    server.sin_addr.s_addr = inet_addr(serverAddress);
    server.sin_family = AF_INET;
    server.sin_port = htons(5000); 
    
    if (connect(handle->socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        #if defined(WINDOWS)
            printf("Connect error:%d.\n", WSAGetLastError());
            closesocket(handle->socket);
            WSACleanup();
        #elif defined(LINUX)
            // Nothing here
            printf("Connect error\n");
        #endif
        
        return false;
    }

    printf("Succesfuly connected socket");

    return true;

}

int Network_write(NetworkingHandle_t handle, size_t lengthToWrite)
{

    if(handle->txBuf == NULL)
    {
        printf("Tx buffer is null");
        return -1;
    }

    return send(handle->socket, handle->txBuf, lengthToWrite, 0);

}

int Network_read(NetworkingHandle_t handle)
{
    

    if(handle->rxBuf == NULL)
    {
        printf("rxBuf is null");
        return -1;
    }

    return recv(handle->socket, handle->rxBuf, sizeof(handle->rxBuf), 0);
}

int Network_close(NetworkingHandle_t handle)
{
    #if defined(WINDOWS)
        closesocket(handle->socket);
        WSACleanup();
    #elif defined(LINUX)
        // Nothing here
    #endif
}