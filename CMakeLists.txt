cmake_minimum_required(VERSION 3.3)
project("asciisnake" LANGUAGES C)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lws2_32")

add_executable(asciisnake 
    client/game.c 
    client/networking.c)
# set_property(TARGET asciisnake PROPERTY CXX_STANDARD 17)
