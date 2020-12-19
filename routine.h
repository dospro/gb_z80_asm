#ifndef GB_Z80_ASM_ROUTINE_H
#define GB_Z80_ASM_ROUTINE_H

struct RoutineList {
    struct Routine *root;
};

struct Routine {
    char name[64]; // Routine name(ej. sub_process:
    long address; // where the routine will be written
    int call_address; // where the routine is called(to use jr instead
    int return_address; // This will contain the addres where the ret is found
    struct Routine *next;
};

int add_routine(struct RoutineList *list, char *name, long address);

struct Routine *pop_routine(struct RoutineList list);

struct Routine *search_routine_by_name(struct RoutineList list, char *name);

int free_routines(struct RoutineList list);


#endif //GB_Z80_ASM_ROUTINE_H
