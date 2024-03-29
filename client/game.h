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
#include "../networking/networking.h"
#include "../protocol/protocol.h"
#include "../server/server.h"

typedef int8_t metric_t;

typedef struct
{
    metric_t x;
    metric_t y;
} Point_t;

struct Snake
{
    struct Snake* snake;
    Point_t point;
};

typedef struct {
    char* itemName;
    void (*fun_ptr)(void);
}MenuItem_t;

typedef struct 
{
    uint8_t snake_color_id :     4;
    char snake_ascii :           8;
}PlayerConfig_t;

typedef struct
{
    LoginRequestPacket_t loginSettings;
    const ServerConfig_t* serverSettings;
    PlayerConfig_t playerConfig;
}GameSettings_t;

typedef struct Snake Snake_t;
typedef GameSettings_t* GameSettingsHandle_t;
#endif