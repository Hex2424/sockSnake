
#include "server.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <assert.h>
#include <unistd.h>


static bool validateLoginRecv_(const int socketfd, LoginRequestPacket_t* loginPacket, const char* serverPassword);


bool Server_begin(const ServerConfig_t* config)
{
    Networking_t networkObject;
    int* socketDescriptors;
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

        socketDescriptors = malloc(sizeof(int) * config->playerCap);
        
        if(socketDescriptors == NULL)
        {
            printf("Failed malloc of socket descriptor list %u\n", config->playerCap);
            return false;
        }
        // Stage of accepting logins
        for(uint8_t playerId = 0; playerId < config->playerCap; /*Do nothing*/ )
        {
            LoginRequestPacket_t loginPacket;
            LoginResponsePacket_t response;
            response.color_id = 0;
            response.body_ascii = '0';

            char responseBuffer[LOGIN_RESPONSE_PACKET_SIZE];

            int new_socket = accept(networkObject.socket, (struct sockaddr*)&server, &addresslen);
            if (new_socket < 0)
            {
                printf("Failed accept server socket\n");
                continue;
            }

            if(validateLoginRecv_(new_socket, &loginPacket, config->serverPassword))
            {
                response.status = OKAY;
                Protocol_encapLoginResponse(responseBuffer, &response);
                if(send(new_socket, responseBuffer, sizeof(responseBuffer), 0) >= 0)
                {
                    printf("Successful logged player: %s\n", loginPacket.username);
                    socketDescriptors[playerId] = new_socket;
                    playerId++;
                }else 
                {
                    close(new_socket); // closing connection
                    continue;
                }
            }else
            {
                // Failed login, need continue
                response.status = FAIL_PASSW;
                Protocol_encapLoginResponse(responseBuffer, &response);
                send(new_socket, responseBuffer, sizeof(responseBuffer), 0);
                close(new_socket); // closing connection
                continue;
            }
            
        }
 
     
    #elif defined(WINDOWS)
        // NOTHING FOR NOW
    #endif


    
    
}

static bool validateLoginRecv_(const int socketfd, LoginRequestPacketHandle_t loginPacket, const char* serverPassword)
{
    // reading password
    if(recv(socketfd, loginPacket, sizeof(LoginRequestPacket_t), 0) < 0)
    {
        printf("Read failure of socket\n");
        return false;
    }

    if(strstr(loginPacket->password, serverPassword) == NULL)
    {
        printf("Password is incorrect\n");
        return false;
    }

    return true;
}


// bool Server_close(void)
// {
    
// }