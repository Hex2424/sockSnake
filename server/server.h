
#ifndef SERVER
#define SERVER
#include <stdbool.h>
#include <stdint.h>
#include "../networking/networking.h"
#include "string.h"
#include "../protocol/protocol.h"

#define DEFAULT_PORT                2442


typedef struct
{
    char* serverPassword;
    uint16_t serverPort;
    
    bool friendlyDeathAllowed;
    uint8_t playerCap;

}ServerConfig_t;


bool Server_begin(const ServerConfig_t* config);



#endif