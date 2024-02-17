#include "protocol.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>


void Protocol_encapLoginResponse(char* buffer, const LoginResponsePacketHandle_t packet)
{
    assert(packet->status < COUNT);
    assert(packet->color_id < (1 << 4)); 
    assert(isprint(packet->body_ascii));

    buffer[0] = 0;      // making byte 0

    // Putting status
    buffer[0] |= packet->status & MASK(2); 
    buffer[0] <<= 2;
    
    //Putting reserved
    buffer[0] |= packet->reserved & MASK(2); 
    buffer[0] <<= 2;

    //Putting color_id
    buffer[0] |= packet->color_id & MASK(4); 
    buffer[0] <<= 4;

    //Putting body_ascii
    buffer[1] = packet->body_ascii;
}

void Protocol_decapLoginResponse(LoginResponsePacketHandle_t packet, const char* buffer)
{

    // Putting status
    packet->status = (buffer[0] >> 6) & MASK(2); 
    
    //Putting reserved
    packet->reserved = (buffer[0] >> 4) & MASK(2); 
    
    //reading color_id
    packet->color_id = (buffer[0] >> 0) & MASK(4); 

    //reading body_ascii
    packet->body_ascii = buffer[1];

}