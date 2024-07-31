/* symbol.c manages the symbol lists and contains all symbol related functions. */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "symbol_table.h"

extern int cnt_line;	/* line number to show in errors */
extern bool has_error;	/* to prevent output file creation if an error was found */
extern int ic;
extern int dc;
extern char *file_am_name;

struct symbol_node
{
	unsigned int address; /* adrress of the symbol */
	char symbol_name[MAX_SYMBOL_LENGTH]; 
	symbol_type type;
};


/* Adds a new symbol to the list */
void add_symbol(node_ref list_head, int address, char symbol_name[MAX_SYMBOL_LENGTH], symbol_type type)
{
	symbol_node new_symbol;
	symbol_node *found; 
	/* If the symbol already exists */
		if ((found = get_symbol(list_head, symbol_name)) != NULL)
		{
            if(type == entry){
				printf("**Warning**: redeclaration of a symbol as an entry in line %d\n", cnt_line);
				return;
			}
			else if(type == external && address == 0 && found->type != external){
				printf("\nError in %s, line %d: declartion of label as extern  but label already was defined locally\n", file_am_name, cnt_line);
				has_error = TRUE;
				return;
			}
			else if(type != external){
				if(found->type == external){
					printf("\nError in %s, line %d: defining label  but label already was declared as extern\n", file_am_name, cnt_line);
					has_error = TRUE;
					return;
				}
				else{
					printf("\nError in %s, line %d: defining label ibut label was already defined\n", file_am_name, cnt_line);
					has_error = TRUE;
					return;
				}
			}
		}

	/* Creates the new symbol */
	new_symbol.address = address;
	new_symbol.type = type;
	strcpy(new_symbol.symbol_name, symbol_name);

	/* Adds the new symbol to the symbol list */
	add_node(list_head, &new_symbol, sizeof(symbol_node));
} /* end of add_symbol */

/* Retrieves a symbol with the specified name from the symbol list */
symbol_node *get_symbol(node_ref list_head, char symbol_name[MAX_SYMBOL_LENGTH]) 
{
	symbol_node *curr; /* Current symbol node being checked */
	node_ptr temp; /* Iterator for traversing the list */


	temp = *list_head;
	/* If the list is empty, return null */
	if (temp == NULL)
		return NULL;

	/* Iterate through the list to find the matching symbol */
	do
	{
		curr = (symbol_node *)(get_data(temp));

		/* If the symbol matches the specified name, return a pointer to it */
		if (strcmp(curr->symbol_name, symbol_name) == 0)
			return curr;

	} while ((temp = get_next(temp)) != NULL);

	/* If no matching symbol was found, return null */
	return NULL;
} /* End of get_symbol */



/* Getter function to retrieve the address of a symbol */
unsigned int get_address(symbol_node *symbol) {
    return symbol->address;
}

/* Getter function to retrieve the symbol name */
char *get_symbol_name(symbol_node *symbol) {
    return symbol->symbol_name;
}

/* Getter function to retrieve the type of a symbol */
symbol_type get_type(symbol_node *symbol) {
    return symbol->type;
}

/* Function to modify the address of a symbol by a given offset */
void set_address(symbol_node *symbol, int offset){
    symbol->address += offset;
}

/* Function to modify the type of a symbol */
void set_type(symbol_node *symbol, symbol_type new_type) {
    symbol->type = new_type;
}




