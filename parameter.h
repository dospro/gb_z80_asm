#ifndef GB_Z80_ASM_PARAMETER_H
#define GB_Z80_ASM_PARAMETER_H

#include <stdbool.h>

struct Parameter {
    char string[32];
    int value;
    bool is_word;
    bool is_bit;
    int bits;
};

bool process_first_argument(struct Parameter *parameter, char *arg);
bool process_second_parameter(struct Parameter *parameter, char *arg);

#endif //GB_Z80_ASM_PARAMETER_H
