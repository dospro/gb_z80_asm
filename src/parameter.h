#ifndef GB_Z80_ASM_PARAMETER_H
#define GB_Z80_ASM_PARAMETER_H

#include <stdbool.h>

enum ParamType {
    RESERVED,
    BYTE,
    WORD,
    BIT
};

struct Parameter {
    char string[32];
    int value;
    enum ParamType type;
};

struct Arguments {
    char opcode_args[64];
    struct Parameter arg1;
    struct Parameter arg2;
};

bool process_first_argument(struct Parameter *parameter, char *arg);
bool process_second_parameter(struct Parameter *parameter, char *arg);
bool scan_arguments(struct Arguments *operation, char *arg1, char *arg2);

#endif //GB_Z80_ASM_PARAMETER_H
