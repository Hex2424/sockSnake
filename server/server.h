
#ifndef SERVER
#define SERVER
#include <stdbool.h>
#include <stdint.h>
#include "../networking/networking.h"
#include "string.h"


typedef struct
{
    char* serverPassword;
    bool friendlyDeathAllowed;
    uint8_t playerCap;

}ServerConfig_t;


bool Server_begin(const ServerConfig_t* config);



#endif