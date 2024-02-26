#include "game.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../server/server.h"
#include "../logger/logger.h"
#include "../utils/crc32.h"
//**********************
// CONST VARIABLES

// ARENA
#define ARENA_WIDTH         30
#define ARENA_HEIGHT        20

// MENU
#define MENU_WINDOW_WIDTH   30
#define MENU_ITEM_COUNT     (sizeof(menuItems) / sizeof(MenuItem_t))
#define MENU_WINDOW_HEIGHT  (MENU_ITEM_COUNT + 5)
#define ITEMS_OFFSET_COL    3


#define FLAG_LEFT           1
#define FLAG_DOWN           2
#define FLAG_RIGHT          3
#define FLAG_UP             4

#define GAME_SPEED          100
#define GAME_FPS            16
#define NETWORK_SPEED       10

#define SYNC_BYTE           0x55
#if defined(WINDOWS)
    #define CHAR_OBSTACKLE      (char) 0xB2
#elif defined(LINUX)
    #define CHAR_OBSTACKLE      (char) 0
#endif

#define CHAR_SNAKE          '0'
#define CHAR_ENEMY          '@'
#define CHAR_FOOD           'Q'
#define CHAR_EMPTY          ' '

#if defined(WINDOWS)
    #define THREAD_HANDLE HANDLE
    #define CLEAR_COMMAND "cls"
    typedef void ThreadRet_t;
    #define PUT_SLEEP(time) Sleep(time)

    #define CREATE_THREAD(handle, function) \
        handle = CreateThread(NULL, 0, function, NULL, 0, NULL)

    #define KILL_THREAD(handle) TerminateThread(handle, 0);

#elif defined(LINUX)
    #define THREAD_HANDLE pthread_t
    #define CLEAR_COMMAND "clear"
    typedef void* ThreadRet_t;
    #define PUT_SLEEP(time) usleep(time * 1000)
    #define CREATE_THREAD(handle, function, argument) \
        pthread_create(&handle, NULL, function, argument)

    #define KILL_THREAD(handle) pthread_exit(handle)
#endif

const static char TITLE[] = "Ascii Snake v0.1";
const static char* TAG = "CLIENT";

//**********************
// PRIVATE METHODS

static void handleBlockPainting_();
static Snake_t* addBodySnake_(const Snake_t* snake,const Point_t* newPoint);
static void paintArena_();
static void paintBorders_(WINDOW* window);
static void paintSnake_();
static void paintFood_();
// static void paintEnemySnake_();
static void clearScreen_();
static void generateNewPos_(Point_t* oldPoint);
static void initGame_();
static void gameLoop_();
static bool isEating_();
static void handleEat_();
static ThreadRet_t handleInput_();
static ThreadRet_t runServer(void* args);
static void handleMovement_();
static void gameOver_();
static void playSinglePlayer_();
static void transformSnake_();
static ThreadRet_t paintingThread_(void* data);
// static void flushBufferPrint_();
// static ThreadRet_t networkReadLoop_();
static void closeThreads_();
// static ThreadRet_t networkSendLoop_();
static void exitGame_();
static void processMenuWithSelection_();
static void handleDataPacket_(char dataPacketId, char length, char* dataBuffer);
static void playInServer_(const GameSettingsHandle_t settings);
//**********************
// PRIVATE VARIABLES

const MenuItem_t menuItems[] = 
{
    {"SinglePlayer", playSinglePlayer_},
    {"Start Lobby", NULL},
    {"Join Lobby", NULL},
    {"Settings", NULL},
    {"Exit", NULL}
};

THREAD_HANDLE paintThread;
THREAD_HANDLE eventsThread;
THREAD_HANDLE networkingSendThread;
THREAD_HANDLE networkingReadThread;

THREAD_HANDLE serverThread;

bool isOnline = false;
bool isGameRunning = true;

Snake_t* snake;
Point_t foodPos;
int8_t currentDirection = 1;
int readed = 0;
bool rxLock = false;
WINDOW* arena;
WINDOW* borderw;


//**********************
// IMPLEMENTATION

int main(int argc, char **argv)
{
    // Firstly initializing network

    //initGame_();
    //gameLoop_();

    processMenuWithSelection_();
}

static void processMenuWithSelection_(void)
{
    WINDOW* menuWindow;
    uint8_t selectedItemPos = 0;
    initscr();
    noecho();
    curs_set(0);
    menuWindow = newwin(MENU_WINDOW_HEIGHT, MENU_WINDOW_WIDTH, 0, 0);
    box(menuWindow, 0, 0);
    keypad(menuWindow, true);

    mvwaddstr(menuWindow, 1, (MENU_WINDOW_WIDTH - (sizeof(TITLE) - 1)) / 2, TITLE);


    for(uint8_t itemPos = 0; itemPos < MENU_ITEM_COUNT; itemPos++)
    {
        mvwaddstr(menuWindow, itemPos + ITEMS_OFFSET_COL, 3, menuItems[itemPos].itemName);
    }

    while (true) 
    {
        
        mvwaddch(menuWindow, selectedItemPos + 3, 1, '>');
        // mvwaddstr(menuWindow, selectedItemPos + 3, 3, menuItems[selectedItemPos].itemName);
        
        wrefresh(menuWindow);
        switch (wgetch(menuWindow)) 
        {
            case 'w': 
            case KEY_UP:
            {
                mvwaddch(menuWindow, selectedItemPos + ITEMS_OFFSET_COL, 1, ' ');
                if(selectedItemPos == 0)
                {
                    selectedItemPos = MENU_ITEM_COUNT - 1;
                }else
                {
                    selectedItemPos--;
                }

            } continue;

            case 's':
            case KEY_DOWN:
            {
                mvwaddch(menuWindow, selectedItemPos + ITEMS_OFFSET_COL, 1, ' ');
                selectedItemPos++;
                if(selectedItemPos == MENU_ITEM_COUNT)
                {
                    selectedItemPos = 0;
                }
            } continue;

            case KEY_ENTER:break;
            case ' ':break;

	    default:continue;

        }
        break;
    }

    endwin();
    // Calling item function
    menuItems[selectedItemPos].fun_ptr();
}


static void gameLoop_()
{
    CREATE_THREAD(paintThread, paintingThread_, NULL);
    CREATE_THREAD(eventsThread, handleInput_, NULL);

    // if(isOnline)
    // {
    //     CREATE_THREAD(networkingSendThread, networkSendLoop_, NULL);
    //     CREATE_THREAD(networkingReadThread, networkReadLoop_, NULL);
    // }

    while (isGameRunning)
    {
        PUT_SLEEP(GAME_SPEED);
        if(isEating_())
        {
            handleEat_();
        }
        handleMovement_();
    }
}

static ThreadRet_t paintingThread_(void* data) 
{
    while(isGameRunning)
    {
        paintArena_();
        PUT_SLEEP(GAME_FPS);
    }
}

static void initGame_()
{
    srand(time(NULL));
    generateNewPos_(&foodPos);

    isGameRunning = true;
    snake = malloc(sizeof(Snake_t));

    generateNewPos_(&snake[0].point);
    snake->snake = NULL;

    initscr();
    noecho();
    curs_set(0);
    
    borderw = newwin(ARENA_HEIGHT, ARENA_WIDTH, 0, 0);
    paintBorders_(borderw);
    wrefresh(borderw);
    
    arena = derwin(borderw, ARENA_HEIGHT - 2, ARENA_WIDTH - 2, 1, 1);
    
}

static void exitGame_()
{
    isGameRunning = false;
    delwin(arena);
    delwin(borderw);
    endwin();
    exit(0);
}

static void paintArena_()
{
    
    // clearScreen_();

    handleBlockPainting_();
    wrefresh(arena);
    werase(arena);
    // flushBufferPrint_();
}


static void handleBlockPainting_()
{
    // memset(printBuffer, CHAR_EMPTY, sizeof(printBuffer));
    paintFood_();
    paintSnake_();
    // paintEnemySnake_();
    // paintFood_();
}

static void clearScreen_()
{
    #if defined(WINDOWS)
        COORD coord = {0, 0};
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    #elif defined(LINUX)
        printf("%c[%d;%df",0x1B,0,0);
    #endif
}


static void paintBorders_(WINDOW* window)
{

    box(window, CHAR_OBSTACKLE, CHAR_OBSTACKLE);
}

static void paintSnake_()
{
    Snake_t* currentNode = snake;
    while(currentNode != NULL)
    {
        mvwaddch(arena, currentNode->point.y, currentNode->point.x, CHAR_SNAKE);
        // printBuffer[currentNode->point.y][currentNode->point.x] = CHAR_SNAKE;
        currentNode = currentNode->snake;
    }
}

static void paintFood_()
{
    mvwaddch(arena, foodPos.y, foodPos.x, CHAR_FOOD);
}

// static void paintEnemySnake_()
// {
//     char* pointStart = strchr(networkObject.rxBuf, SYNC_BYTE);
//     if(pointStart == NULL)
//     {
//         return;
//     }
//     pointStart++;
//     while (*pointStart != SYNC_BYTE && (pointStart < (networkObject.rxBuf + readed)))
//     {
//         handleDataPacket_(pointStart[0], pointStart[1], pointStart + 2);
//         mvwaddch(arena, pointStart[1], pointStart[0], CHAR_ENEMY);
//         pointStart += 2;
//     }
    
// }

//  if(snake->point.x == pointStart[0] && snake->point.y == pointStart[1])
//     {
//         gameOver_();
//     }

static void handleDataPacket_(char dataPacketId, char length, char* dataBuffer)
{

}



static Snake_t* addBodySnake_(const Snake_t* snake,const Point_t* newPoint)
{
    Snake_t* snakeHead = malloc(sizeof(Snake_t));
    snakeHead->point = *newPoint; 
    snakeHead->snake = (Snake_t*) snake;
    return snakeHead;
}

static void generateNewPos_(Point_t* oldPoint)
{
    oldPoint->x = (rand() % (ARENA_WIDTH - 4)) + 2;
    oldPoint->y = (rand() % (ARENA_HEIGHT - 3)) + 1;
}

static ThreadRet_t handleInput_()
{
    while(isGameRunning)
    {
        // if (kbhit()) 
        // {
            switch (wgetch(borderw)) {
            case 'a':
            {
                if(currentDirection != FLAG_RIGHT)
                {
                    currentDirection = FLAG_LEFT;
                }

            }break;

            case 's':
            {
                if(currentDirection != FLAG_UP)
                {
                    currentDirection = FLAG_DOWN;
                }

            }break;
                
            
            case 'd':
            {
                if(currentDirection != FLAG_LEFT)
                {
                    currentDirection = FLAG_RIGHT;
                }
            }break;
                
            
            case 'w':
            {
                if(currentDirection != FLAG_DOWN)
                {
                    currentDirection = FLAG_UP;
                }
            }break;
                
            
            case 'x':
            {

                isGameRunning = false;
            }break;
                
        }
        
    }
 
}
static void closeThreads_()
{
    KILL_THREAD(&paintThread);
    KILL_THREAD(&eventsThread);
    KILL_THREAD(&networkingSendThread);
    KILL_THREAD(&networkingReadThread);
    KILL_THREAD(&serverThread);
}

static void handleMovement_()
{
    transformSnake_();

    switch (currentDirection)
    {
        case FLAG_LEFT:
        {
            snake->point.x -= 1;
            if(snake->point.x < 0)
            {
                snake->point.x = (ARENA_WIDTH - 3);
            }
            
        }break;
           
        case FLAG_DOWN:
        {
            snake->point.y += 1;
            if(snake->point.y > ARENA_HEIGHT - 3)
            {
                snake->point.y = 0;
            }
        }break;

        case FLAG_RIGHT:
        {
            snake->point.x += 1;
            if(snake->point.x > ARENA_WIDTH - 3)
            {
                snake->point.x = 2;
            }
        }break;

        case FLAG_UP:
        {
            snake->point.y -= 1;
            if(snake->point.y < 0)
            {
                snake->point.y = (ARENA_HEIGHT- 3);
            }
        }break;

    }

}
static void gameOver_()
{
    // Network_close(&networkObject);
    // closeThreads_();
    system(CLEAR_COMMAND);
    printf("Game Over!");
    exitGame_();
}
static bool isEating_()
{
    return snake->point.x == foodPos.x &&
        snake->point.y == foodPos.y;
}


static void handleEat_()
{
    snake = addBodySnake_(snake, &foodPos);
    generateNewPos_(&foodPos);
}

static void transformSnake_()
{
    Snake_t* currentNode;
    currentNode = snake;
    Point_t prevCoord = snake->point;

    currentNode = currentNode -> snake;
    
    while (currentNode) {
        Point_t temp = currentNode->point;
        currentNode->point = prevCoord;
        prevCoord = temp;
        currentNode = currentNode->snake;
    }
    
}
// static void flushBufferPrint_()
// {
//     fwrite(printBuffer, 1, sizeof(printBuffer), stdout);
// }

// static ThreadRet_t networkSendLoop_()
// {
//     while (isGameRunning)
//     {
//         Snake_t* currentNode = snake;
//         int byteIdx = 0;

//         networkObject.txBuf[byteIdx] = (char) SYNC_BYTE;
//         byteIdx++;
//         while(currentNode != NULL)
//         {
//             networkObject.txBuf[byteIdx] = currentNode->point.x;
//             networkObject.txBuf[byteIdx + 1] = currentNode->point.y;
//             currentNode = currentNode->snake;
//             byteIdx += 2;
//         }
//         int written;
        
//         if(written = Network_writeFullPacket(&networkObject, networkObject.txBuf,byteIdx + 1) == 0) // SOCKET_ERROR
//         {
//             #if defined(WINDOWS)
//                 printf("Closing socket TX \n%d", WSAGetLastError());
//             #endif

//             Network_close(&networkObject);
//             exitGame_();
//             return NULL;
//         }
//         // printf("TX: %d %d %d %d...\n", networkObject.txBuf[0], networkObject.txBuf[1], networkObject.txBuf[2],  networkObject.txBuf[3]);
//         PUT_SLEEP(NETWORK_SPEED);

//     }
    
// }

// static ThreadRet_t networkReadLoop_()
// {
//     while (isGameRunning)
//     {
   
//         if((readed = Network_readFullPacket(&networkObject, networkObject.rxBuf, 1)) == 0) // SOCKET_ERROR
//         {
//             #if defined(WINDOWS)
//                 printf("Closing socket RX\n %d", WSAGetLastError());
//             #endif
            
//             Network_close(&networkObject);
//             exitGame_();
//         }
//         // printf("RX: %d %d %d...\n", networkObject.rxBuf[0], networkObject.rxBuf[1], networkObject.rxBuf[2]);
//         PUT_SLEEP(NETWORK_SPEED);
//     }
  
// }


static void playSinglePlayer_()
{
    const ServerConfig_t serverSettings = 
    {
        .serverPassword = "123",
        .serverPort = DEFAULT_PORT,
        .friendlyDeathAllowed = false,
        .playerCap = 1
    };

    GameSettings_t settings = 
    {
        .loginSettings = 
        {   
            .passcrc32 = CRC32_calc((uint8_t*) "123"),
            .loginUsername = "Hex24"
        },
        .playerConfig = 
        {
            .snake_ascii = '0',
            .snake_color_id = COLOR_BLUE
        },
        .serverSettings = &serverSettings
    };
    
    playInServer_(&settings);
}

static void playInServer_(const GameSettingsHandle_t settings)
{
    char loginResponseBuffer[LOGIN_RESPONSE_PACKET_SIZE];
    Socket_t connectSocket;
    LoginResponsePacket_t loginResponse;

    SockAddr_t clientAddr;
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(DEFAULT_PORT); 

    if(!Socket_init())
    {
        Log_e(TAG, "Failed init network");
        return;
    }

    Log_d(TAG, "Succesful initialized socket framework");

    if((connectSocket = Socket_createSocket()) == -1)
    {
        Log_e(TAG, "Failed init network");
        return;
    }
    
    Log_d(TAG, "Succesful initialized socket");

    if(settings->serverSettings != NULL)
    {
	Log_d(TAG,"Server initializing");
        CREATE_THREAD(serverThread, runServer, (void*) settings->serverSettings);
        PUT_SLEEP(10); // waitime to initialize server
    }

    if(!Socket_connectSocket(connectSocket, &clientAddr))
    {
        Log_e(TAG,"Failed connect server\n");
        return;
    }
    Log_i(TAG, "Succesfuly connected to server");

    // Sending login request
    if(Socket_sendFullPacket(connectSocket, (const char*) &settings->loginSettings, sizeof(LoginRequestPacket_t)))
    {
        Log_i(TAG,"Successfuly sent Login request");
    }else
    {
        Log_i(TAG,"Failed to send login request");
        Socket_close(connectSocket);
        return;
    }

    // listening login response
    if(Socket_readFullPacket(connectSocket, loginResponseBuffer, sizeof(loginResponseBuffer)))
    {

        Protocol_decapLoginResponse(&loginResponse, loginResponseBuffer);
        Log_d(TAG, "Received login response with status %u", loginResponse.status);
   
        if(loginResponse.status == 0)
        {
            Socket_close(connectSocket);
        }else 
        {
            Socket_close(connectSocket);
        }

    }else
    {
        Socket_close(connectSocket);
    }


    return;
}


static ThreadRet_t runServer(void* args)
{
    Server_begin((const ServerConfig_t*) args);
}
