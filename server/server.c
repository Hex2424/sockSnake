
#include "server.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <assert.h>

#define DEFAULT_PORT 2442


bool Server_begin(const ServerConfig_t* config)
{
    Networking_t networkObject;
    struct sockaddr_in server;
    socklen_t addresslen = sizeof(server); 
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((uint16_t) DEFAULT_PORT);

    assert(config->serverPassword);
    assert(config->playerCap != 0);
    

    // Stage of prepare

    if(!Networking_init())
    {
        printf("Failed init network\n");
        return SOCKET_ERROR;
    }
    if(!Networking_initializeSocket(&networkObject))
    {
        printf("Failed init socket\n");
        return SOCKET_ERROR;
    }

    #if defined(LINUX)
        if (bind(networkObject.socket, (const struct sockaddr*) &server, addresslen) < 0) 
        {
            printf("Failed bindserver socket\n");
            return SOCKET_ERROR;
        }

        if (listen(networkObject.socket, 3) < 0) 
        {
            printf("Failed listen server socket\n");
            return SOCKET_ERROR;
        }

        // Catching and validating connections
        while (true)
        {
            int new_socket = accept(networkObject.socket, (struct sockaddr*)&server, &addresslen);
            if (new_socket < 0)
            {
                printf("Failed accept server socket\n");
                return SOCKET_ERROR;
            }

        }
     
    #elif defined(WINDOWS)
        // NOTHING FOR NOW
    #endif


    // Stage of accepting logins
    for(uint8_t playerId = 0; playerId < config->playerCap; /*Do nothing*/ )
    {
        
    }
    
}


bool Server_close(void)
{
    
}