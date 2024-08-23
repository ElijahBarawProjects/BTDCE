#include "spacial_partition.h"

#include <math.h>
#include <stdlib.h>

#include "list.h"
#include "structs.h"

struct multi_list_t {
    size_t width;     // width of space in terms of `box_size`
    size_t height;    // height of the space in terms of `box_size`
    size_t box_size;  // size of the squares we break the space into
    size_t num_boxes;
    queue_t **boxes;
};

size_t ceil(size_t a, size_t b) { return (a + b - 1) / b; }

multi_list_t *new_partitioned_list(size_t width, size_t height,
                                   size_t box_size) {
    multi_list_t *multi_l = malloc(sizeof(multi_l));
    multi_l->box_size = box_size;
    multi_l->height = ceil(height, box_size);
    multi_l->width = ceil(width, box_size);
    multi_l->boxes =
        calloc(sizeof(queue_t *),
               multi_l->num_boxes = (multi_l->height * multi_l->width));

    return multi_l;
}

void free_partitioned_list(multi_list_t *free_me, void (*freer)(void *)) {
    queue_t *curr_q;
    for (size_t i = 0; i < free_me->num_boxes; i++) {
        if ((curr_q = (&(free_me->boxes)[i])) == NULL) continue;

        queue_free(curr_q, freer);
    }

    free(free_me);
}

queue_t *hard_get_list(multi_list_t *l, position_t p) {
    // find which box the position is in
    int box_col = p.x / l->box_size;
    int box_row = p.y / l->box_size;
    int box_ind = (box_col * l->width) + box_row;

    if (box_ind >= l->num_boxes) return NULL;

    // lazily create boxes
    queue_t *box = l->boxes[box_ind];
    if (box == NULL) {
        box = queue_new();
        l->boxes[box_ind] = box;
    }

    return box;
}

queue_t *soft_get_list(multi_list_t *l, position_t p) {
    // find which box the position is in
    int box_col = p.x / l->box_size;
    int box_row = p.y / l->box_size;
    int box_ind = (box_col * l->width) + box_row;

    if (box_ind >= l->num_boxes) return NULL;

    return l->boxes[box_ind];
}

void insert(multi_list_t *l, position_t p, void *v) {
    queue_insert_head(hard_get_list(l, p), v);
}

void remove(multi_list_t *l, position_t p, list_ele_t *elem,
            void (*freer)(void *)) {
    remove_and_delete(hard_get_list(l, p), elem, freer);
}

void fix_list(multi_list_t *l, list_ele_t *elem, position_t old_pos,
              position_t new_pos) {
    void *v = elem->value;
    insert(l, new_pos, v);
    remove(l, old_pos, elem, NULL);
}