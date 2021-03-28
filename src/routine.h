#ifndef GB_Z80_ASM_ROUTINE_H
#define GB_Z80_ASM_ROUTINE_H

#include "list.h"

struct Routine {
    char name[64]; // Routine name(ej. sub_process:
    long address; // where the routine will be written
    int call_address; // where the routine is called(to use jr instead
    int return_address; // This will contain the address where the ret is found
};

struct List create_routines_list();

bool add_routine(struct List *list, char *name, long address);

struct Routine *pop_routine(struct List *list);

struct Routine *search_routine_by_name(const struct List *list, char *name);

void free_routines(struct List *list);


#endif //GB_Z80_ASM_ROUTINE_H
