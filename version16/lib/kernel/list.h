#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H
#include "global.h"

struct list_elem {
    struct list_elem* prev;
    struct list_elem* next;
};

struct list {
    struct list_elem head;
    struct list_elem tail;
};


typedef bool (function) (struct list_elem*,int arg);


void list_init(struct list* list)
void list_insert_before(struct list_elem* before,struct list_elem* elem);
void lit_push(struct list* plist,struct list_elem* elem);
void list_append(struct list* plist,struct list_elem* elem);
void list_remove(struct list_elem* pelem);
bool elem_find(struct list* plist,struct list_elem* obj_elem)
struct list_elem* list_traversal(struct list* plist,function func,int arg);
uint32_t list_len(struct list* plist);
bool list_empty(struct list* plist);
#endif