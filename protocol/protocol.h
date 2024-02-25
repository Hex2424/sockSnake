#ifndef PROTOCOL
#define PROTOCOL

#include <stdint.h>
#include <stdbool.h>

#define SERVER_PASS_LENGTH          20
#define SERVER_USERNAME_LENGTH      8
#define MASK(ones) ((0x01 << ones) - 1)
#define LISTEN_BACKLOG              3

// Packet Sizes
#define LOGIN_RESPONSE_PACKET_SIZE      2

typedef enum
{
    OKAY =            0x00,
    FAIL_PASSW =    0x01,
    LOBBY_FULL =    0x02,

    COUNT
}LoginStatus_e;

typedef struct
{
    uint32_t passcrc32;
    char loginUsername[SERVER_USERNAME_LENGTH];
}LoginRequestPacket_t;

typedef struct
{
    LoginStatus_e status :  2;
    uint8_t reserved :      2;
    uint8_t color_id :      4;
    char body_ascii :       8;
}LoginResponsePacket_t;

typedef LoginRequestPacket_t* LoginRequestPacketHandle_t;
typedef LoginResponsePacket_t* LoginResponsePacketHandle_t;

void Protocol_encapLoginResponse(char* buffer, const LoginResponsePacketHandle_t packet);
void Protocol_decapLoginResponse(LoginResponsePacketHandle_t packet, const char* buffer);

#endif