cmake_minimum_required(VERSION 3.17)
project(gb_z80_asm C)

set(CMAKE_C_STANDARD 23)

include(c_tests/CMakeLists.txt)

set(SOURCES
        src/main.c
        src/routine.c
        src/jumps.c
        src/parameter.c
        src/opcode.c
        src/list.c
        src/cli.c
        src/string.c
        src/string.h
        src/error_handler.c
        src/error_handler.h
)

add_executable(gb_z80_asm ${SOURCES})