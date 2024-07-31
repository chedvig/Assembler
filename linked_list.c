/* linkedList.c handles all linked list functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"

#define remove_node(node) free_s(node->data); free_s(node);

/* a struct to represent a node of the linked list*/
struct node {
    struct node *next;
    void *data;
};

/* Creates a new node and adds it to the end of the list */
void add_node(node_ref head, void *data, int size) 
{
	node_ptr new_node; /* node to add */
	node_ptr temp; /* iterator */
	void *buffer; /* data buffer */

    /* Creates the data buffer and the node */
    buffer = calloc(1, size);
	new_node = (node_ptr)calloc(1, sizeof(node_def));
	
	/* Checks if buffer and node were created successfully*/
        valid_allocate(buffer);
        valid_allocate(new_node);
	
    memcpy(buffer, data, size); /* data copied into buffer */

   	/* Sets the values of the new node */
    new_node->data = buffer;
   	new_node->next = NULL;

	/* If the list is empty */
	if ((*head) == NULL) {
		(*head) = new_node;
			return;
	}

	/* Adds the node to the end of the list */
    temp = (*head);
   	while (temp->next != NULL)
      	 	temp = temp->next;
    temp->next = new_node;
} /* end of add_node */

/* Clears a list */
void clear_list(node_ptr *head)
{
	node_ptr temp; /* iterator */
	node_ptr node;

	if (*head == NULL)
		return;

	/* Clears all nodes */
	temp = *head;
	while (temp != NULL)
	{
		node = temp;
		temp = temp->next;

	        remove_node(node);
	}
	/* Clears the pointer */
	*head = NULL;
} /* end of clear_list */

/* Gets the next node in the list */
node_ptr get_next(node_ptr node) {
    return node->next;
}

/* Gets the data stored in the node */
void *get_data(node_ptr node) {
    return node->data;
}/*end of get_next*/
