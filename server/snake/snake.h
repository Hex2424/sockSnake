#ifndef SNAKE
#define SNAKE
#include <stdint.h>
#include <stdbool.h>

#define MAX_ALLOWED_SNAKE_BENDS 504
#define DIRECTION_OFFSET        -2

typedef int8_t Metric_t;
typedef int8_t Direction_t;
typedef uint8_t snakeId_t ;
typedef Metric_t Bend_t;
typedef Bend_t* BendCursor_t;

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
   bool isStartX;
}Snake_t;

typedef struct
{
    
}SnakeSocketData_t;

#endif


