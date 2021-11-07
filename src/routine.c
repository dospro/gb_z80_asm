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

bool compare_routine(struct Node *node, void *context) {
    return strcmp(((struct Routine *) node->data)->name, (char *)context) == 0;
}

struct Routine *search_routine_by_name(const struct List *list, char *name) {
    struct Node *node = search(list, &compare_routine, name);
    if (node != NULL) {
        return (struct Routine *) node->data;
    }
    return NULL;
}

void free_routines(struct List *list) {
    delete_list(list);
}