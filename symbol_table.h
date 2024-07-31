/* symbol.c manages the symbol lists and contains all symbol related functions. */
#ifndef _LABEL_H_
#define _LABEL_H_


#include "linked_list.h"
#include "define.h"


typedef enum 
{
	data_relocatable,
	code_relocatable,
	external,
	entry
} symbol_type;

typedef struct symbol_node symbol_node;

/* Adds a new symbol to the list */
void add_symbol(node_ref list_head, int node_number, char symbol_name[MAX_SYMBOL_LENGTH], symbol_type type);

/* Gets a symbol by name from the symbol list */
symbol_node *get_symbol(node_ref list_head, char symbol_name[MAX_SYMBOL_LENGTH]);

/* Clears all symbol lists */
void clear_symbol_list();

/* Getter function to retrieve the address of a symbol */
unsigned int get_address( symbol_node *);
   

/* Getter function to retrieve the symbol name */
char *get_symbol_name(symbol_node *);
 

/* Getter function to retrieve the type of a symbol */
symbol_type get_type(symbol_node *);

/* Function to modify the address of a symbol by a given offset */
void set_address(symbol_node *symbol, int offset);

/* Function to modify the type of a symbol */
void set_type(symbol_node *symbol, symbol_type new_type);

#endif