#ifndef GAME
#define GAME
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINUX 

#if defined(WINDOWS)
    #include <windows.h>
#elif defined(LINUX)
    #include <pthread.h>
    #include <unistd.h>
#endif

#include <time.h>
#include "networking.h"

typedef uint8_t metric_t;

typedef struct Point
{
    metric_t x;
    metric_t y;
} Point_t;

struct Snake
{
    struct Snake* snake;
    Point_t point;
};
typedef struct Snake Snake_t;

#endif