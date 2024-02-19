
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
    Socket_t listenSocket;
    int* socketDescriptors;
    SockAddr_t server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(config->serverPort);
    assert(config->serverPassword);
    assert(config->playerCap != 0);
    

    // Stage of prepare
    if(!Socket_init())
    {
        Log_e(TAG, "Failed init network");
        return SOCKET_ERROR;
    }

    Log_d(TAG, "Successfuly initialized socket framework");
    
    if((listenSocket = Socket_createSocket()) == -1)
    {
        Log_e(TAG, "Failed create socket");
        return SOCKET_ERROR;
    }

    Log_d(TAG, "Succesfuly initialized socket");
    
    if (!Socket_bind(listenSocket, &server)) 
    {
        Log_e(TAG,"Failed bindserver socket");
        return SOCKET_ERROR;
    }

    Log_d(TAG, "Succesfuly binded server socket");

    if (!Socket_listen(listenSocket, LISTEN_BACKLOG)) 
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

    Log_i(TAG, "Server sucessfuly started");

    // Stage of accepting logins
    for(uint8_t playerId = 0; playerId < config->playerCap; /*Do nothing*/ )
    {
        Socket_t new_socket = Socket_acceptSocket(listenSocket, &server);
        Log_d(TAG, "Accepted new socket");
        // handling player password validation
        if(isPlayerValidOnSocket_(new_socket, config->serverPassword))
        {
            socketDescriptors[playerId] = new_socket;
            playerId++;
        }else
        {
            if(Socket_close(new_socket))
            {
                Log_d(TAG, "Closing player socket");
            }else
            {
                Log_e(TAG, "Failed to close player socket");
            }
            
        }
        
    }

        // Successfuly found all players
        // Continue to configuration page
        
}

static bool isPlayerValidOnSocket_(const Socket_t socket, const char* validityPassword)
{
    LoginRequestPacket_t loginPacket;
    LoginResponsePacket_t response;
    char responseBuffer[LOGIN_RESPONSE_PACKET_SIZE];

    response.color_id = 0;
    response.body_ascii = '0';

    if (socket == -1)
    {
        Log_e(TAG, "Failed accept server socket");
        return false;
    }

    if(validateLoginRecv_(socket, &loginPacket, validityPassword))
    {
        response.status = OKAY;
        Protocol_encapLoginResponse(responseBuffer, &response);

        if(Socket_sendFullPacket(socket, responseBuffer, sizeof(responseBuffer)))
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
        Socket_sendFullPacket(socket, responseBuffer, sizeof(responseBuffer));
        return false;
    }
}


static void handleConfigurations_()
{
    
}

static bool validateLoginRecv_(const Socket_t socket, LoginRequestPacketHandle_t loginPacket, const char* serverPassword)
{
    // reading password
    // TODO: Need solve issue of server killing on player disconnect
    if(!Socket_readFullPacket(socket, (char*) loginPacket, sizeof(LoginRequestPacket_t)))
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
