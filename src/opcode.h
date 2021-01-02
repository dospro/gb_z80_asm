#ifndef GB_Z80_ASM_OPCODE_H
#define GB_Z80_ASM_OPCODE_H

#include <stdbool.h>

struct Opcode {
    char name[6];
    char arg_string[12];
    int value, high_value;
};

struct MachineCode {
    int opcode;
    int params_size;
    int params[3];
};

bool get_general_opcode(struct MachineCode *machine_code_out, char *opcode_name, char *arg1, char *arg2);

int search_opcode(char *opcode_name, char *arguments);


#endif //GB_Z80_ASM_OPCODE_H
