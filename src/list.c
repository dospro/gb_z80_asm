#include "list.h"
#include <stdlib.h>

struct List create_list() {
    struct List list;
    list.root_node = NULL;
    list.end_node = NULL;
    list.size = 0;
    return list;
}

void delete_list(struct List *list) {
    struct Node *node = list->root_node;
    while (node != NULL) {
        struct Node *temp = node->next;
        free(temp->data);
        free(temp);
        node = temp;
        list->size--;
    }
    list->root_node = NULL;
    list->end_node = NULL;
}

bool list_push(struct List *list, void *data) {
    struct Node *newNode = NULL;
    newNode = (struct Node *) malloc(sizeof(struct Node));
    if (newNode == NULL) {
        return false;
    }
    newNode->data = data;
    newNode->next = NULL;

    if (list->root_node == NULL) {
        list->root_node = newNode;
        list->end_node = list->root_node;
    } else {
        list->end_node->next = newNode;
        list->end_node = newNode;
    }
    list->size++;
    return true;
}

struct Node list_pop(struct List *list) {
    struct Node *item;
    struct Node result;

    result.next = NULL;
    if (list->size == 0) {
        result.data = NULL;
        return result;
    }

    result.data = list->end_node->data;
    result.next = NULL;

    item = list->root_node;
    while (item->next != list->end_node) {
        item = item->next;
    }
    free(list->end_node);
    list->end_node = item;
    list->size--;

    return result;
}

void foreach(const struct List *list, void_func *funcp, void *context) {
    for (struct Node *node = list->root_node; node != NULL; node = node->next) {
        (*funcp)(node, context);
    }
}

struct List map(const struct List *list, func *funcp, void *context) {
    struct List newList;
    for (struct Node *node = list->root_node; node != NULL; node = node->next) {
        struct Node *mapped_node = (*funcp)(node, context);
        list_push(&newList, mapped_node->data);
    }
    return newList;
}

struct List filter(const struct List *list, bool_func *funcp, void *context) {
    struct List newList;
    for (struct Node *node = list->root_node; node != NULL; node = node->next) {
        if ((*funcp)(node, context)) {
            list_push(&newList, node->data);
        }
    }
    return newList;
}

struct Node *search(const struct List *list, bool_func *funcp, void *context) {
    for (struct Node *node = list->root_node; node != NULL; node = node->next) {
        if ((*funcp)(node, context)) {
            return node;
        }
    }
    return NULL;
}