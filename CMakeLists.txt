cmake_minimum_required(VERSION 3.7.2)
project(Praktikum C)

set(CMAKE_C_STANDARD 99)

add_executable(Praktikum main.c
        main.h
        keyValStore.c
        sub.h
        sub.c
        server.c
)
