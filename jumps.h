#ifndef GB_Z80_ASM_JUMPS_H
#define GB_Z80_ASM_JUMPS_H

struct JumpList {
    struct Jump *root;
};

struct Jump {
    char name[64]; // The name of this call
    long address; // Where is the call
    int value; // This will have a 0 as generic before getting the right value
    int bits; // 8 if its a byte, 16 if its a word(2 bytes addres)
    struct Jump *next; // Next node
};

int add_jump(struct JumpList *list, char *name, long address, int value, int bits);

struct Jump *pop_jump(struct JumpList list);

struct Jump *search_by_name(struct JumpList list, char *name);

int free_jumps(struct JumpList stack);

#endif //GB_Z80_ASM_JUMPS_H
