#include "routine.h"

#include <string.h>
#include <stdlib.h>

struct List create_routines_list() {
    return create_list();
}

bool add_routine(struct List *list, char *name, long address) {
    struct Routine *pointer = (struct Routine *) malloc(sizeof(struct Routine));
    strcpy(pointer->name, strtok(name, ":"));
    pointer->address = address;
    return list_push(list, (void *) pointer);
}

struct Routine *pop_routine(struct List *list) {
    struct Node result = list_pop(list);
    return result.data;
}

char *global_name = NULL;

bool compare_routine(struct Node *node) {
    return strcmp(((struct Routine *) node->data)->name, global_name) == 0;
}

struct Routine *search_routine_by_name(const struct List *list, char *name) {
    global_name = name;
    struct Node *node = search(list, &compare_routine);
    if (node != NULL) {
        global_name = NULL;
        return (struct Routine *) node->data;
    }
    global_name = NULL;
    return NULL;
}

void free_routines(struct List *list) {
    delete_list(list);
}