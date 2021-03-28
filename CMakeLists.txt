cmake_minimum_required(VERSION 3.17)
project(gb_z80_asm C)

set(CMAKE_C_STANDARD 11)

set(SOURCES
        src/main.c
        src/routine.c
        src/jumps.c
        src/parameter.c
        src/opcode.c
        src/list.c
        src/cli.c
        )

add_executable(gb_z80_asm ${SOURCES})