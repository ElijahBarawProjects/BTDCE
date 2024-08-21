#include "list.h"
#include <stdlib.h>

// Definition of the list_t struct
struct list_t {
    // Your internal list data, e.g., pointers, size, etc.
};

// Definition of the elem_t struct
struct elem_t {
    void* data;
    struct elem_t* next;
    struct elem_t* prev;
};

list_t* new_list() {
    list_t* list = malloc(sizeof(list_t));
    // Initialize the list's internal structure
    return list;
}

void push_front(list_t* list, void* elem) {
    // Implement the logic for pushing an element to the front
}

void push_back(list_t* list, void* elem) {
    // Implement the logic for pushing an element to the back
}

size_t size(const list_t* list) {
    // Return the size of the list
}

elem_t* pop_front(list_t* list) {
    // Implement the logic for popping an element from the front
}

elem_t* pop_back(list_t* list) {
    // Implement the logic for popping an element from the back
}

void remove(elem_t* elem) {
    // Implement the logic for removing an element
    free(elem);
}
