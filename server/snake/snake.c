
#include "snake.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

static Snake_t* snakes;
static uint8_t count;

#define SIGN(value) (((value) > 0) - ((value) < 0))


static inline const BendCursor_t snakeGetNextBend_(const Snake_t* snake, const BendCursor_t cursor);
static inline void snakeMove_(const snakeId_t snakeId);
static void snakeMoveHead_(const Snake_t* snake);
static void snakeMoveTail_(Snake_t* snake);
static inline void directionToIncrementor_(const Direction_e direction, BendCursor_t xIncr, BendCursor_t yIncr);
static inline const Direction_t incrementorToDirecton_(const BendCursor_t xIncr, const BendCursor_t yIncr);

bool SnakeNest_init(const uint8_t snakeCount)
{
    // Allocating memory for snakes
    count = snakeCount;
    snakes = malloc(sizeof(Snake_t) * count);
    if(snakes == NULL)
    {
        return false;
    }else
    {
        return true;
    }
    srand(0);
}

void SnakeNest_createSnakes(const uint8_t arenaWidth, const uint8_t arenaHeight)
{
    assert(count > 0);
    assert(arenaWidth > 10);
    assert(arenaHeight > 10);

    //TODO: for now random, need smarter way to determine spawn locations so that snakes start far from each other
    for(uint8_t snakeId = 0; snakeId < count; snakeId++)
    {
        Metric_t x, y;
        Snake_t* snake;

        snake = &snakes[snakeId];

        x = rand() % arenaWidth - (arenaWidth / 2);
        y = rand() % arenaHeight - (arenaHeight / 2);

        snake->tail = snake->bends; // tail at first position firstly
        snake->head = snake->tail;

        // initial is x, y, y + 1 which is (x, y) -> (x, y + 1) 
        snakes[snakeId].bends[0] = x; 
        snakes[snakeId].bends[1] = y;
        snakes[snakeId].bends[2] = (y + 1);
    }
}

void SnakeNest_snakeChangeDirection(const snakeId_t snakeId, const Direction_t direction)
{
    // checking if not equal oposite direction
    if((direction & snakes[snakeId].direction) == 0)
    {
        snakes[snakeId].direction = direction;
    }

}

bool SnakeNest_snakesTickUpdate(void)
{
    //TODO: for now random, need smarter way to determine spawn locations so that snakes start far from each other
    for(snakeId_t snakeId = 0; snakeId < count; snakeId++)
    {
        snakeMove_(snakeId);
    }

    return true;
}

static inline bool isSnakeCursorOnX(const Snake_t* snake, const BendCursor_t cursor)
{
    return ((cursor - snake->bends) % 2 == 0);
}

//    x1 y1y2  x2x3 y3y4 x5
// 0, 0,    3,   1,   5


// 0,0 -> right
// 3,0
// 3,1
// 5,1
// 1, 0
// x  y  x  y
// 0, 0, 0, 0 

static inline void snakeMove_(const snakeId_t snakeId)
{
    const Snake_t* snake = &snakes[snakeId];
    snakeMoveHead_(snake);
    snakeMoveTail_((Snake_t*) snake);
}

static void snakeMoveHead_(const Snake_t* snake)
{
    BendCursor_t x1, y1, x2, y2;

    const BendCursor_t value1 = snake->head;
    const BendCursor_t value2 = snakeGetNextBend_(snake, value1);
    const BendCursor_t value3 = snakeGetNextBend_(snake, value2);
    
    const bool isXFirstInHead =  isSnakeCursorOnX(snake, snake->head);

    if(isXFirstInHead)
    {
        x1 = value1;
        y1 = value2;
        x2 = value3;
        y2 = y1;
    }else 
    {
        y1 = value1;
        x1 = value2;
        y2 = value3;
        x2 = x1;
    }

    const Metric_t xDiff = SIGN((*x2 - *x1));
    const Metric_t yDiff = SIGN((*y2 - *y1));
    const Direction_e currentDirection = incrementorToDirecton_((BendCursor_t) &xDiff, (BendCursor_t) &yDiff);
    // x1 - x2  = -1 = RIGHT
    // x1 - x2 = 1 = LEFT
    // y1 - y2 = -1 = DOWN
    // y1 - y2 = 1 = UP

    // (-1, 0) = 01
    // (1, 0) = 10
    // (0, -1) = 11
    // (0, 1) = 00


    if(currentDirection == snake->direction)
    {
        *x2 += xDiff;
        *y2 += yDiff; 
    }else 
    {
        BendCursor_t valueNew;
        Metric_t xIncr;
        Metric_t yIncr;

        directionToIncrementor_(snake->direction, &xIncr, &yIncr);
        valueNew = snakeGetNextBend_(snake, value3);
        
        if(isXFirstInHead)
        {
            *valueNew = *y2 + yIncr;
        }else 
        {
            *valueNew = *x2 + xIncr;
        }

        ((Snake_t*) snake)->head = (BendCursor_t) value2; // moving cursor of head one further
    }

}

static void snakeMoveTail_(Snake_t* snake)
{
    BendCursor_t incrementValue = snake->tail;
    const BendCursor_t secondthValue = snakeGetNextBend_(snake, incrementValue);
    BendCursor_t destinationValue = snakeGetNextBend_(snake, secondthValue);
    
    const Metric_t incrementor = SIGN((*destinationValue - *incrementValue));

    *incrementValue += incrementor;

    if(incrementor == 0)
    {
        // moving tail cursor by 1
        snake->tail = secondthValue;
    }
}

static inline const Direction_t incrementorToDirecton_(const BendCursor_t xIncr, const BendCursor_t yIncr)
{
    Direction_e direction;
    // X POS
    if(*xIncr == 1)
    {
        direction = RIGHT;

    }else if(*xIncr == -1)
    {
        direction = LEFT;
    }

    // Y POS
    if(*yIncr == 1)
    {
        direction = UP;

    }else if(*yIncr == -1)
    {
        direction = DOWN;
    }

    return (Direction_t) direction;
} 

static inline void directionToIncrementor_(const Direction_e direction, BendCursor_t xIncr, BendCursor_t yIncr)
{
    // X POS
    switch (direction) 
    {
        case RIGHT: 
        {
            *xIncr = 1;
        }break;

        case LEFT:
        {
            *xIncr = -1;
        }break;

        case UP:
        {
            *yIncr = 1;
        }break;

        case DOWN:
        {
            *yIncr = -1;
        }break;
    }
} 


// Handling ring buffer overflow more efficiently
static inline const BendCursor_t snakeGetNextBend_(const Snake_t* snake, const BendCursor_t cursor)
{
    const BendCursor_t overflowCursor = (const BendCursor_t) (snake->bends + sizeof(snake->bends));
    const BendCursor_t comparisonCursor = &cursor[1];

    if(overflowCursor > comparisonCursor)
    {
        return comparisonCursor;
    }else
    {
        return &snakes->bends[1];
    }
}
