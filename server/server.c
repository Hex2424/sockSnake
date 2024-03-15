
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
#include "../utils/crc32.h"
#include "snake/snake.h"

const static char* TAG = "SERVER"; 

#define ARENA_WIDTH         30
#define ARENA_HEIGHT        20

#if MULTICAST
    #define MULTICAST_PORT      12345
    #define MULTICAST_ADDR      "239.0.0.1"
#endif

static SockAddr_t castAddr;
static SockIPv4* playerIPs;

static bool validateLoginRecv_(const Socket_t socket, LoginRequestPacketHandle_t loginPacket, const uint32_t serverToken);
static bool isPlayerValidOnSocket_(const int socketfd, const uint32_t serverToken);
static void notifyClientsFrameChange_(const Socket_t onSocket, SockMessageHandle_t sockMsgPtr);
static void serverLogicLoop_(void);
static void handleConfigurations_(void);

bool Server_begin(const ServerConfig_t* config)
{
    Socket_t listenSocket;
    int* socketDescriptors;
    SockAddr_t server;
    uint32_t serverToken;

    // Initializing server listener
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(config->serverPort);

    // Initializing multicast addressing
    memset(&castAddr, 0, sizeof(castAddr));
    castAddr.sin_family = AF_INET;

    #if MULTICAST
        castAddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDR); // Example multicast group address
        castAddr.sin_port = htons(MULTICAST_PORT);
    #endif
        

    assert(config->serverPassword);
    assert(config->playerCap != 0);

    playerIPs = malloc(sizeof(in_addr_t) * config->playerCap);
    if(playerIPs == NULL)
    {
        exit(EXIT_FAILURE);
    }
  
    serverToken = CRC32_calc((uint8_t*) config->serverPassword);
    
    // Stage of prepare
    if(!Socket_init())
    {
        Log_e(TAG, "Failed init network");
        exit(EXIT_FAILURE);
    }

    Log_d(TAG, "Successfuly initialized socket framework");
    
    if((listenSocket = Socket_createSocketTCP()) == -1)
    {
        Log_e(TAG, "Failed create socket");
        exit(EXIT_FAILURE);
    }

    Log_d(TAG, "Succesfuly initialized socket");
    
    if (!Socket_bind(listenSocket, &server)) 
    {
        Log_e(TAG,"Failed bindserver socket");
        exit(EXIT_FAILURE);
    }

    Log_d(TAG, "Succesfuly binded server socket");

    if (!Socket_listenTCP(listenSocket, LISTEN_BACKLOG)) 
    {
        Log_e(TAG,"Failed listen server socket");
        exit(EXIT_FAILURE);
    }

    Log_d(TAG, "Listening for incoming sockets");
    
    socketDescriptors = malloc(sizeof(int) * config->playerCap);
    
    if(socketDescriptors == NULL)
    {
        Log_e(TAG,"Failed malloc of socket descriptor list %u", config->playerCap);
        exit(EXIT_FAILURE);
    }

    Log_i(TAG, "Server sucessfuly started");

    // Stage of accepting logins
    for(uint8_t playerId = 0; playerId < config->playerCap; /*Do nothing*/ )
    {
        Socket_t new_socket = Socket_acceptSocketTCP(listenSocket, &server);
        Log_d(TAG, "Accepted new socket");
        // handling player password validation
        if(isPlayerValidOnSocket_(new_socket, serverToken))
        {
            SockIPv4 remoteIP;
            socketDescriptors[playerId] = new_socket;

            if(!Socket_getRemoteIP(new_socket, &remoteIP))
            {
                Log_e(TAG, "Remote ip failed to get");
                exit(EXIT_FAILURE);
            }
            playerIPs[playerId] = remoteIP;
            playerId++;
        }else
        {
            if(!Socket_close(new_socket))
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

    handleConfigurations_();

    // Game packets handling
    serverLogicLoop_();

}


static void serverLogicLoop_(void)
{

    Socket_t udp;
    SockMessage_t msg;

    memset(&msg, 0, sizeof(msg));

    msg.msg_name = &castAddr;
    msg.msg_namelen = sizeof(castAddr);
   
    if((udp = Socket_createSocketUDP()) == -1)
    {
        Log_e(TAG, "Failed create udp socket");
        exit(EXIT_FAILURE);
    }

    if(!SnakeNest_init(1))
    {
        Log_e(TAG, "Failed to create snake nest, mem alloc error");
        exit(EXIT_FAILURE);
    }

    SnakeNest_createSnakes(ARENA_WIDTH, ARENA_HEIGHT);

    while (true)
    {
        Log_d(TAG, "Loop frame start");
        notifyClientsFrameChange_(udp, &msg);
        Log_d(TAG, "Loop frame end");
        // TODO change with both platforms compatible function
        usleep(3000*1000);
    }
    
}


static void notifyClientsFrameChange_(const Socket_t onSocket, SockMessageHandle_t sockMsgPtr)
{

    sockMsgPtr->msg_iovlen = (size_t) SnakeNest_bufferizePosData();
    sockMsgPtr->msg_iov = SnakeNest_getSockVectors();

    for(uint8_t ipIdx = 0; ipIdx < 1; ipIdx++)
    {
        #if !MULTICAST
            castAddr.sin_addr = playerIPs[ipIdx]; // Example multicast group address
            castAddr.sin_port = htons(12345);
        #endif

        if (sendmsg(onSocket, (const SockMessageHandle_t) sockMsgPtr, 0) == -1)
        {
            Log_e(TAG, "Server send multicast failure");
            Socket_close(onSocket);
            exit(EXIT_FAILURE);
        }
    }

}




static bool isPlayerValidOnSocket_(const Socket_t socket, const uint32_t validityToken)
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

    if(validateLoginRecv_(socket, &loginPacket, validityToken))
    {
        response.status = OKAY;
        Protocol_encapLoginResponse(responseBuffer, &response);

        if(Socket_sendFullPacketTCP(socket, responseBuffer, sizeof(responseBuffer)))
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
        Socket_sendFullPacketTCP(socket, responseBuffer, sizeof(responseBuffer));
        return false;
    }
}


static void handleConfigurations_(void)
{
    
}

static bool validateLoginRecv_(const Socket_t socket, LoginRequestPacketHandle_t loginPacket, const uint32_t serverToken)
{
    // TODO: Need solve issue of server killing on player disconnect
    if(!Socket_readFullPacketTCP(socket, (char*) loginPacket, sizeof(LoginRequestPacket_t)))
    {
        Log_e(TAG,"Read failure of socket");
        return false;
    }

    if(loginPacket->passcrc32 != serverToken)
    {
        Log_d(TAG,"Password is incorrect %u != %u", loginPacket->passcrc32, serverToken);
        return false;
    }

    return true;
}


// bool Server_close(void)
// {
    
// }
