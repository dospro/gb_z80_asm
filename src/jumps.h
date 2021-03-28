#ifndef GB_Z80_ASM_JUMPS_H
#define GB_Z80_ASM_JUMPS_H

#include <stdio.h>
#include <stdbool.h>

struct Jump {
    char name[64]; // The name of this call
    long address; // Where is the call
    int value; // This will have a 0 as generic before getting the right value
    int bits; // 8 if its a byte, 16 if its a word(2 bytes addres)
};

struct List create_jumps_list();

bool add_jump(struct List *list, char *name, long address, int value, int bits);

struct Jump *pop_jump(struct List *list);

struct Jump *search_by_name(const struct List *list, char *name);

void free_jumps(struct List *list);

#endif //GB_Z80_ASM_JUMPS_H
