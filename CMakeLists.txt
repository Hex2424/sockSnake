cmake_minimum_required(VERSION 3.3)
project("asciisnake" LANGUAGES C)


IF (WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lws2_32")
  add_compile_definitions(WINDOWS)
ELSE()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lncurses")
  add_compile_definitions(LINUX)
ENDIF()

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_executable(asciisnake 
    client/game.c 
    networking/networking.c)

# set_property(TARGET asciisnake PROPERTY CXX_STANDARD 17)
