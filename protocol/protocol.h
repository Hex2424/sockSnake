#ifndef PROTOCOL
#define PROTOCOL

#include <stdint.h>
#include <stdbool.h>

#define SERVER_PASS_LENGTH          20
#define SERVER_USERNAME_LENGTH      8
#define MASK(ones) ((0x01 << ones) - 1)


// Packet Sizes
#define LOGIN_RESPONSE_PACKET_SIZE      2

typedef enum
{
    OK =            0x00,
    FAIL_PASSW =    0x01,
    LOBBY_FULL =    0x02,

    COUNT
}LoginStatus_e;

typedef struct
{
    char password[SERVER_PASS_LENGTH];
    char username[SERVER_USERNAME_LENGTH];
}LoginPacket_t;

typedef LoginPacket_t* LoginPacketHandle_t;


void Protocol_formatLoginResponse(char* buffer, const uint8_t status, const uint8_t color_id, const char body_ascii);

#endif