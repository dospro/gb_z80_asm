#include "routine.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct RoutineList create_routines_list() {
    struct RoutineList list;
    list.root = NULL;
    return list;
}

int add_routine(struct RoutineList *list, char *name, long address) {
    struct Routine *pointer;

    if (list->root == NULL) {
        list->root = (struct Routine *) malloc(sizeof(struct Routine));
        pointer = list->root;
    } else {
        pointer = list->root;
        while (pointer->next != NULL) {
            pointer = pointer->next;
        }
        pointer->next = (struct Routine *) malloc(sizeof(struct Routine));
        pointer = pointer->next;
    }
    if (pointer == NULL) {
        printf("ERROR: Unable to allocate memory for new routine\n");
        return -1;
    }

    strcpy(pointer->name, strtok(name, ":"));
    pointer->address = address;
    pointer->next = NULL;
    return 0;
}

/*
 * Pops the last pushed routine from the list
 *
 * This function is safe in the sense that it will free
 * allocated memory and return a list-based struct instead
 * so it gets freed automatically.
 */
struct Routine *pop_routine(struct RoutineList *list) {
    struct Routine *pointer;

    pointer = list->root;

    if (pointer == NULL) {
        return NULL;
    }
    while (pointer->next != NULL) {
        pointer = pointer->next;
    }

    return pointer;
}

struct Routine *search_routine_by_name(struct RoutineList list, char *name) {
    struct Routine *pointer = list.root;

    while (pointer != NULL) {
        if (strcmp(pointer->name, name) == 0) {
            return pointer;
        }
        pointer = pointer->next;
    }
    return NULL;
}

int free_routines(struct RoutineList list) {
    struct Routine *pointer = list.root;
    while (pointer != NULL) {
        struct Routine *temp;
        temp = pointer->next;
        free(pointer);
        pointer = temp;
    }
    return 0;
}