cmake_minimum_required(VERSION 3.17)
project(gb_z80_asm C)

set(CMAKE_C_STANDARD 11)

add_executable(gb_z80_asm main.c routine.c jumps.c parameter.c opcode.c)