#ifndef GB_Z80_ASM_LIST_H
#define GB_Z80_ASM_LIST_H

#include <stdio.h>
#include <stdbool.h>


struct Node {
    void *data;
    struct Node *next;
};

struct List {
    struct Node *root_node;
    struct Node *end_node;
    size_t size;
};

typedef void (void_func)(struct Node *, void *data);
typedef struct Node *(func)(struct Node *, void *data);
typedef bool (bool_func)(struct Node *, void *data);

struct List create_list();
void delete_list(struct List *list);
bool list_push(struct List *list, void *data);
struct Node list_pop(struct List *list);

void foreach(const struct List *list, void_func *funcp, void *context);
struct List map(const struct List *list, func *funcp, void *context);
struct List filter(const struct List *list, bool_func *funcp, void *context);
struct Node *search(const struct List *list, bool_func *funcp, void *context);


#endif //GB_Z80_ASM_LIST_H
