#ifndef SNAKE
#define SNAKE
#include <stdint.h>
#include <stdbool.h>
#include "../../networking/networking.h"

#define MAX_ALLOWED_SNAKE_BENDS 248
#define DIRECTION_OFFSET        -2

typedef int8_t Metric_t;
typedef int8_t Direction_t;
typedef uint8_t SnakeId_t ;
typedef Metric_t Bend_t;
typedef Bend_t* BendCursor_t;
typedef uint32_t SnakeMetaData_t;

typedef enum
{
    UP =     0xb00,
    RIGHT =  0xb01,
    LEFT =   0xb10,
    DOWN =   0xb11,
}Direction_e;


typedef struct
{
   Bend_t bends[MAX_ALLOWED_SNAKE_BENDS];
   BendCursor_t head;
   BendCursor_t tail;
   Direction_t direction;
}Snake_t;

uint16_t SnakeNest_bufferizePosData(void);
SockVectors_t* SnakeNest_getSockVectors(void);
bool SnakeNest_snakesTickUpdate(void);
void SnakeNest_snakeChangeDirection(const SnakeId_t snakeId, const Direction_t direction);
void SnakeNest_createSnakes(const uint8_t arenaWidth, const uint8_t arenaHeight);
bool SnakeNest_init(const uint8_t snakeCount);

#endif


