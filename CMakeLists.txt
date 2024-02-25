cmake_minimum_required(VERSION 3.3)
project("asciisnake" LANGUAGES C)


IF (WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lws2_32")
  add_compile_definitions(WINDOWS)
ELSE()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lncurses")
  add_compile_definitions(LINUX)
ENDIF()

add_compile_definitions(VERBOSE_LEVEL=3)

set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)

include_directories(
  utils
  client
  server
  logger
  networking
)

add_executable(asciisnake 
    client/game.c 
    networking/networking.c
    protocol/protocol.c
    server/server.c
    server/snake/snake.c
    logger/logger.c
    utils/crc32.c
    )

# set_property(TARGET asciisnake PROPERTY CXX_STANDARD 17)
