
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "define.h"

typedef struct node node_def, *node_ptr, **node_ref;

/* Creates a new node and adds it to the end of the list */
void add_node(node_ref head, void *data, int size);

/* Clears a list */
void clear_list(node_ptr *head);

/* Gets the next node in the list */
node_ptr get_next(node_ptr node);

/* Gets the data stored in the node */
void *get_data(node_ptr node);

#endif
