#ifndef SPACIAL_H
#define SPACIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "list.h"
#include "structs.h"

typedef struct multi_list_t multi_list_t;

multi_list_t *new_partitioned_list(size_t width, size_t height,
                                   size_t num_lists);

void free_partitioned_list(multi_list_t *free_me, void (*freer)(void *));

/// @brief Get the list which this position corresponds to
/// @return `NULL` if the position is out of bounds; if the position is in
/// bounds it creates an empty list
queue_t *hard_get_list(multi_list_t *l, position_t p);

/// @brief Get the list which this position corresponds to
/// @return `NULL` if a list hasn't been instantiated or the position is out of
/// bounds
queue_t *soft_get_list(multi_list_t *l, position_t p);

void insert(multi_list_t *l, position_t p, void *data);

void remove(multi_list_t *l, position_t p, list_ele_t *elem,
            void (*freer)(void *));

void fix_list(multi_list_t *l, list_ele_t *elem, position_t old_pos,
              position_t new_pos);

#ifdef __cplusplus
}
#endif

#endif