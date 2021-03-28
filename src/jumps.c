#include "jumps.h"

#include <string.h>
#include <stdlib.h>

#include "list.h"

struct List create_jumps_list() {
    return create_list();
}

bool add_jump(struct List *list, char *name, long address, int value, int bits) {
    struct Jump *pointer = (struct Jump *) malloc(sizeof(struct Jump));
    strcpy(pointer->name, name);
    pointer->address = address;
    pointer->value = value;
    pointer->bits = bits;

    if (pointer == NULL) {
        return false;
    }
    return list_push(list, (void *) pointer);
}

struct Jump *pop_jump(struct List *list) {
    return (struct Jump *) (list_pop(list).data);
}

char *jump_global_name = NULL;

bool jump_compare_routine(struct Node *node) {
    return strcmp(((struct Jump *) node->data)->name, jump_global_name) == 0;
}

struct Jump *search_by_name(const struct List *list, char *name) {
    jump_global_name = name;
    struct Node *node = search(list, &jump_compare_routine);
    if (node != NULL) {
        jump_global_name = NULL;
        return (struct Jump *) node->data;
    }
    jump_global_name = NULL;
    return NULL;
}


void free_jumps(struct List *list) {
    delete_list(list);
}