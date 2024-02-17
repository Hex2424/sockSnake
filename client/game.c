#include "game.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

#define SOCKET_ERROR        1

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
    #define CREATE_THREAD(handle, function) \
        pthread_create(&handle, NULL, function, NULL)

    #define KILL_THREAD(handle) pthread_exit(handle)
#endif

const char TITLE[] = "Ascii Snake v0.1";

const MenuItem_t menuItems[] = 
{
    {"SinglePlayer", NULL},
    {"Start Lobby", NULL},
    {"Join Lobby", NULL},
    {"Settings", NULL},
    {"Exit", NULL}
};

//**********************
// PRIVATE METHODS

static void handleBlockPainting_();
static Snake_t* addBodySnake_(const Snake_t* snake,const Point_t* newPoint);
static void paintArena_();
static void paintBorders_(WINDOW* window);
static void paintSnake_();
static void paintFood_();
static void paintEnemySnake_();
static void clearScreen_();
static void generateNewPos_(Point_t* oldPoint);
static void initGame_();
static void gameLoop_();
static bool isEating_();
static void handleEat_();
static ThreadRet_t handleInput_();
static void handleMovement_();
static void gameOver_();
static void transformSnake_();
static ThreadRet_t paintingThread_(void* data);
// static void flushBufferPrint_();
static ThreadRet_t networkReadLoop_();
static void closeThreads_();
static ThreadRet_t networkSendLoop_();
static bool networkInit_();
static void exitGame_();
static void processMenuWithSelection_();
static void handleDataPacket_(char dataPacketId, char length, char* dataBuffer);
//**********************
// PRIVATE VARIABLES



THREAD_HANDLE paintThread;
THREAD_HANDLE eventsThread;
THREAD_HANDLE networkingSendThread;
THREAD_HANDLE networkingReadThread;

bool isOnline = false;
bool isGameRunning = true;

Snake_t* snake;
Point_t foodPos;
int8_t currentDirection = 1;
Networking_t networkObject;
int readed = 0;
bool rxLock = false;
WINDOW* arena;
WINDOW* borderw;


//**********************
// IMPLEMENTATION

int main(int argc, char **argv)
{
    processMenuWithSelection_();
    // for (int i = 0; i < argc; ++i)
    // {
    //     if(strcmp(argv[i], "online") == 0)
    //     {
    //         isOnline = true;
    //     } 
    // }

    initGame_();
    gameLoop_();
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

            case KEY_ENTER:
            case ' ': 
            break;

        }
        break;
    }

    

    endwin();
}


static void gameLoop_()
{
    CREATE_THREAD(paintThread, paintingThread_);
    CREATE_THREAD(eventsThread, handleInput_);

    if(isOnline)
    {
        CREATE_THREAD(networkingSendThread, networkSendLoop_);
        CREATE_THREAD(networkingReadThread, networkReadLoop_);
    }

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
    if(isOnline)
    {
        if(networkInit_() != 0)
        {
            exitGame_();
        }
    }

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
    paintEnemySnake_();
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

    // for(metric_t i = 0; i < ARENA_WIDTH; i++)
    // {
    //     printBuffer[0][i] = CHAR_OBSTACKLE;
    //     printBuffer[ARENA_HEIGHT - 1][i] = CHAR_OBSTACKLE;
    // }
    
    // for(metric_t i = 0;i < ARENA_HEIGHT; i++)
    // {
    //     printBuffer[i][0] = '\n';
    //     printBuffer[i][1] = CHAR_OBSTACKLE;
    //     printBuffer[i][ARENA_WIDTH - 1] = CHAR_OBSTACKLE;
    // }

    

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

static void paintEnemySnake_()
{
    char* pointStart = strchr(networkObject.rxBuf, SYNC_BYTE);
    if(pointStart == NULL)
    {
        return;
    }
    pointStart++;
    while (*pointStart != SYNC_BYTE && (pointStart < (networkObject.rxBuf + readed)))
    {
        handleDataPacket_(pointStart[0], pointStart[1], pointStart + 2);
        mvwaddch(arena, pointStart[1], pointStart[0], CHAR_ENEMY);
        pointStart += 2;
    }
    
}

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

static bool networkInit_()
{   
    if(!Networking_init())
    {
        printf("Failed init network\n");
        return SOCKET_ERROR;
    }
    if(!Networking_initializeSocket(&networkObject))
    {
        printf("Failed init socket\n");
        return SOCKET_ERROR;
    }

    if(!Networking_connectSocket(&networkObject, "127.0.0.1", 4546))
    {
        printf("Failed connect socket\n");
        return SOCKET_ERROR;
    }
    printf("Connected to server...waiting others to join\n");
    if(Network_read(&networkObject) < 0)
    {
        printf("smth wrong on read\n");
    } // waiting response from server
    printf("Starting loop\n");
    
    return 0;
}

static ThreadRet_t networkSendLoop_()
{
    while (isGameRunning)
    {
        Snake_t* currentNode = snake;
        int byteIdx = 0;

        networkObject.txBuf[byteIdx] = (char) SYNC_BYTE;
        byteIdx++;
        while(currentNode != NULL)
        {
            networkObject.txBuf[byteIdx] = currentNode->point.x;
            networkObject.txBuf[byteIdx + 1] = currentNode->point.y;
            currentNode = currentNode->snake;
            byteIdx += 2;
        }
        int written;
        
        if(written = Network_write(&networkObject, byteIdx + 1) == 0) // SOCKET_ERROR
        {
            #if defined(WINDOWS)
                printf("Closing socket TX \n%d", WSAGetLastError());
            #endif

            Network_close(&networkObject);
            exitGame_();
            return NULL;
        }
        // printf("TX: %d %d %d %d...\n", networkObject.txBuf[0], networkObject.txBuf[1], networkObject.txBuf[2],  networkObject.txBuf[3]);
        PUT_SLEEP(NETWORK_SPEED);

    }
    
}

static ThreadRet_t networkReadLoop_()
{
    while (isGameRunning)
    {
   
        if((readed = Network_read(&networkObject)) == 0) // SOCKET_ERROR
        {
            #if defined(WINDOWS)
                printf("Closing socket RX\n %d", WSAGetLastError());
            #endif
            
            Network_close(&networkObject);
            exitGame_();
        }
        // printf("RX: %d %d %d...\n", networkObject.rxBuf[0], networkObject.rxBuf[1], networkObject.rxBuf[2]);
        PUT_SLEEP(NETWORK_SPEED);
    }
  
}