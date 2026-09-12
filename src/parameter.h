#ifndef GB_Z80_ASM_PARAMETER_H
#define GB_Z80_ASM_PARAMETER_H
#include "opcode.h"

enum OperandType {
    RESERVED,
    BYTE,
    WORD,
    BIT
};

typedef struct Operand Operand;
struct Operand {
    char string[32];
    String string_new;
    int value;
    enum OperandType type;
};

typedef struct OpcodeOperands OpcodeOperands;
struct OpcodeOperands {
    char opcode_args[64];
    Operand arg1;
    Operand arg2;
};

bool process_first_argument(Operand *parameter, char *arg);
bool process_second_parameter(Operand *parameter, char *arg);
bool scan_arguments(OpcodeOperands *operation, char *arg1, char *arg2);

#endif //GB_Z80_ASM_PARAMETER_H
