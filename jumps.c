#include "jumps.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int add_jump(struct JumpList *list, char *name, long address, int value, int bits) {
    struct Jump *pointer;

    if (list->root == NULL) {
        list->root = (struct Jump *) malloc(sizeof(struct Jump));
        pointer = list->root;
    } else {
        pointer = list->root;
        while (pointer->next != NULL) {
            pointer = pointer->next;
        }
        pointer->next = (struct Jump *) malloc(sizeof(struct Jump));
        pointer = pointer->next;
    }
    if (pointer == NULL) {
        printf("ERROR: Unable to allocate memory for new routine\n");
        return -1;
    }

    strcpy(pointer->name, name);
    pointer->address = address;
    pointer->value = value;
    pointer->bits = bits;
    pointer->next = NULL;
    return 0;
}

/*
 * Removes the top element of the list and returns it.
 *
 * You must manually free the allocated memory.
 */
struct Jump *pop_jump(struct JumpList list) {
    struct Jump *pointer;

    pointer = list.root;
    list.root = pointer->next;
    return pointer;
}

struct Jump *search_by_name(struct JumpList list, char *name) {
    struct Jump *pointer = list.root;

    while (pointer != NULL) {
        if (strcmp(pointer->name, name) == 0) {
            return pointer;
        }
        pointer = pointer->next;
    }
    return NULL;
}


int free_jumps(struct JumpList stack) {
    struct Jump *pointer = stack.root;
    while (pointer != NULL) {
        struct Jump *temp;
        temp = pointer->next;
        free(pointer);
        pointer = temp;
    }
    return 0;
}