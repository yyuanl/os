#include "list.h"
#include "interrupt.h"

// 首尾串接
void list_init(struct list* list){
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.next = NULL;
    list->tail.prev = &list->head;
}

// 把链表元素elem插入元素before之前
void list_insert_before(struct list_elem* before,struct list_elem* elem){
    enum instr_status old_status = intr_disable();

    before->prev->next = elem;

    elem->prev = before->prev->prev;
    elem->next = before;

    before->prev = elem;

    intr_set_status(old_status);
}

void lit_push(struct list* plist,struct list_elem* elem){
    list_insert_before(plist->head.next,elem);
}

void list_append(struct list* plist,struct list_elem* elem){
    list_insert_before(&plist->tail,elem);
}

void list_remove(struct list_elem* pelem){
    enum intr_status old_status = intr_disable();

    pelem->prev->next = pelem->next;
    pelem->next->prev = pelem->prev;

    intr_set_status(old_status);
}

bool elem_find(struct list* plist,struct list_elem* obj_elem){
    struct list_elem* elem = plist->head.next;
    while( elem != &plist->tail){
        if (elem == obj_elem) {
            return true;
        }
        elem = elem->next;
    }
    return false;
}

struct list_elem* list_traversal(struct list* plist,function func,int arg){
    struct list_elem* elem = plist->head.next;
    if (list_empty(plist)) {
        return NULL;
    }

    while( elem != &plist->tail){
        if (func(arg)) {
            return elem;
        }
        elem = elem->next;
    }
    return NULL;
}

uint32_t list_len(struct list* plist){
    struct list_elem* elem = plist->head.next;
    uint32_t len = 0;
    while(elem != &plist->tail){
        len++;
        elem = elem->next;
    }
    return len;
}

bool list_empty(struct list* plist){
    if (plist->head.next == &plist->tail) {
        return true;
    }
    return false;
}