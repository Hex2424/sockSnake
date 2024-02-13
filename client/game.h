#ifndef GAME
#define GAME
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#if defined(WINDOWS)
    #include <windows.h>
#else
    #include <pthread.h>
	#include <termios.h>
	#include <unistd.h>
	#include <fcntl.h>
#endif

#include <time.h>
#include "networking.h"

typedef int8_t metric_t;

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