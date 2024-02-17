#include "protocol.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>


void Protocol_formatLoginResponse(char* buffer, const uint8_t status, const uint8_t color_id, const char body_ascii)
{
    assert(status < COUNT);
    assert(color_id < (1 << 4)); 
    assert(isprint(body_ascii));

    uint8_t response[LOGIN_RESPONSE_PACKET_SIZE]; // 2 bytes packet according protocol
    
    response[0] = 0;      // making all bytes 0

    // Putting status
    response[0] |= status & MASK(2); 
    response[0] <<= 2;
    
    //Putting reserved
    response[0] |= 0 & MASK(2); 
    response[0] <<= 2;

    //Putting color_id
    response[0] |= color_id & MASK(4); 
    response[0] <<= 4;

    //Putting body_ascii
    response[1] = body_ascii;
}