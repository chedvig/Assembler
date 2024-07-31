#ifndef _COMPILE_H_
#define _COMPILE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "symbol_table.h"
#include "syntax.h"

/* compiles the am_file */
void compile (char * file_name);

/* coding data code and build symbol table*/
void first_iteration(FILE *am_file, node_ptr *label_head ,int []);

/*ecoding the instructions sentences according to symbol table an creates output files */
void second_iteration(char *, FILE *, node_ref , int  [], int  []);

/*check for comment or empty line*/
int comment_empty_line(char* line_buffer);

#endif