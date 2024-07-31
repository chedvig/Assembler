#ifndef _DEFINE_H
#define _DEFINE_H

/* This file contains declerations for all other files */

/* Constants */



#define INITIAL 100
#define MAX_CODE 1024
#define MAX_EXTENSION_LENGTH 5
#define NUM_OF_INSTRUCTS 16
#define NUM_OF_DIRECTS 4
#define NUM_OF_REGISTERS 8
#define MIN_INTEGER -512
#define MAX_INTEGER 511
#define MIN_DATA -2048
#define MAX_DATA 2047
#define INVALID_NUM -600
#define MAX_SYMBOL_LENGTH 32
#define MAX_LINE_LENGTH 82
#define SRC_OPS 7
#define DEST_OPS 2
#define TWO_OPS 1
#define ONE_OP 0

/* File extensions */
#define INPUT_FILE ".as"
#define MACRO_FILE ".am"
#define OBJECT_FILE ".ob" 
#define EXTERN_FILE ".ext"
#define ENTRIES_FILE ".ent"

#include <ctype.h>
/* Macros */
#define free_s(p) if(p!= NULL) free(p); p = NULL;
#define valid_allocate(buffer) if (#buffer == NULL) {printf("Cannot allocate memory for this operation.\n"); exit(1); }

/* Represents a boolean */

typedef enum  {
    FALSE,
    TRUE
} bool;

/*generates name file according to the type*/
char *name_file(char *original, char *type);

#endif
