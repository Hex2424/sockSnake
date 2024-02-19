
#include "server.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <assert.h>
#include <unistd.h>
#include "../logger/logger.h"


const static char* TAG = "SERVER"; 


static bool validateLoginRecv_(const int socketfd, LoginRequestPacket_t* loginPacket, const char* serverPassword);
static bool isPlayerValidOnSocket_(const int socketfd, const char* validityPassword);

bool Server_begin(const ServerConfig_t* config)
{
    Networking_t networkObject;
    int* socketDescriptors;
    struct sockaddr_in server;
    socklen_t addresslen = sizeof(server); 
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(config->serverPort);
    assert(config->serverPassword);
    assert(config->playerCap != 0);
    

    // Stage of prepare
    if(!Networking_init())
    {
        Log_e(TAG, "Failed init network");
        return SOCKET_ERROR;
    }

    Log_d(TAG, "Successfuly initialized socket framework");
    
    if(!Networking_initializeSocket(&networkObject))
    {
        Log_e(TAG, "Failed init socket");
        return SOCKET_ERROR;
    }

    Log_d(TAG, "Succesfuly initialized socket");

    #if defined(LINUX)
        if (bind(networkObject.socket, (const struct sockaddr*) &server, addresslen) < 0) 
        {
            Log_e(TAG,"Failed bindserver socket");
            return SOCKET_ERROR;
        }

        Log_d(TAG, "Succesfuly binded server socket");

        if (listen(networkObject.socket, 3) < 0) 
        {
            Log_e(TAG,"Failed listen server socket");
            return SOCKET_ERROR;
        }

        Log_d(TAG, "Listening for incoming sockets");
        
        socketDescriptors = malloc(sizeof(int) * config->playerCap);
        
        if(socketDescriptors == NULL)
        {
            Log_e(TAG,"Failed malloc of socket descriptor list %u", config->playerCap);
            return false;
        }
        // Stage of accepting logins
        for(uint8_t playerId = 0; playerId < config->playerCap; /*Do nothing*/ )
        {
            int new_socket = accept(networkObject.socket, (struct sockaddr*)&server, &addresslen);
            Log_d(TAG, "Accepted new socket");
            // handling player password validation
            if(isPlayerValidOnSocket_(new_socket, config->serverPassword))
            {
                socketDescriptors[playerId] = new_socket;
                playerId++;
            }else
            {
                close(new_socket);
            }
            
        }

        // Successfuly found all players
        // Continue to configuration page
        
 
     
    #elif defined(WINDOWS)
        // NOTHING FOR NOW
    #endif
}

static bool isPlayerValidOnSocket_(const int socketfd, const char* validityPassword)
{
    LoginRequestPacket_t loginPacket;
    LoginResponsePacket_t response;
    char responseBuffer[LOGIN_RESPONSE_PACKET_SIZE];

    response.color_id = 0;
    response.body_ascii = '0';

    if (socketfd < 0)
    {
        Log_e(TAG, "Failed accept server socket");
        return false;
    }

    if(validateLoginRecv_(socketfd, &loginPacket, validityPassword))
    {
        response.status = OKAY;
        Protocol_encapLoginResponse(responseBuffer, &response);

        if(send(socketfd, responseBuffer, sizeof(responseBuffer), 0) >= 0)
        {
            Log_i(TAG, "Successful logged player: %s", loginPacket.loginUsername);
            return true;
        }else 
        {
            return false;
        }
    }else
    {
        // Failed login, need continue
        response.status = FAIL_PASSW;
        Protocol_encapLoginResponse(responseBuffer, &response);
        send(socketfd, responseBuffer, sizeof(responseBuffer), 0);
        return false;
    }
}


static void handleConfigurations_()
{
    
}

static bool validateLoginRecv_(const int socketfd, LoginRequestPacketHandle_t loginPacket, const char* serverPassword)
{
    // reading password
    if(recv(socketfd, loginPacket, sizeof(LoginRequestPacket_t), 0) <= 0)
    {
        Log_e(TAG,"Read failure of socket");
        return false;
    }

    if(strcmp(loginPacket->loginPassword, serverPassword) != 0)
    {
        Log_d(TAG,"Password is incorrect %s != %s", loginPacket->loginPassword, serverPassword);
        return false;
    }

    return true;
}


// bool Server_close(void)
// {
    
// }
