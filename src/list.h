#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct list_t list_t;

typedef struct elem_t elem_t;

list_t* new_list();

void push_front(list_t*, void* elem);
void push_back(list_t*, void* elem);

size_t size(const list_t*);

elem_t* pop_front(list_t*);
elem_t* pop_back(list_t*);

void remove(elem_t*);

#endif
