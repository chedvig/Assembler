
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "pre_processor.h"

/** Macro struct **/
typedef struct Macros {
    char macro_name[MAX_LINE_LENGTH]; /* The title of the macro */
    char *macro_sentence; /* The sentences of the macro */
    struct Macros* next; /* Next macro on the list */
} Macro;


/*** functions prototypes ***/

/* Create a new node for the macro */
Macro* create_node();

/* Check if the given macro name is valid */
int is_macro_name_valid(char* macro_name, char** invalid_macro_names);

/* Check if a macro with the given name already exists in the list */
int is_macro_exist(char* nameOfMacro, Macro* head);

/* Check if the provided line contains either "macro" or "endmacro" */
int macro_validation(char line[]);

/* Insert macros from the specified file into the linked list */
int insert_macro(char* file_name, Macro** head);

/* Insert the name of a macro into the macro list */
int insert_macro_name(Macro* temp, char line[]);

/* Insert the sentences of a macro into the macro list */
void insert_macro_sentence(Macro* temp, FILE* fpR);

/* Add a new node containing a macro to the macro list */
void add_macro_node(Macro* temp, Macro** head);

/* Search for a macro in the line and write its sentences to a file */
int find_macro(char line[], FILE* fpW, Macro** head);

/* Replace macro names with their sentences and write to a new file */
void macro_open(char* file_name, Macro** head);

/* Clear the macro list and free associated memory */
void clear_macro_list(Macro** list);


/*pre_pricess the as_file and creayes the am_file*/
int pre_processor(char *file_name ){	
	int f;
	Macro *head = NULL;

	f = insert_macro(file_name,&head);
	if(!f)
	{	
		macro_open(file_name, &head);
		clear_macro_list(&head);
		return 1;
	}
	else
	    clear_macro_list(&head);
		return 0;

}

/* Create a node for the macro */ 
Macro *create_node() {
    Macro *newNode = (Macro *)malloc(sizeof(Macro)); /* Allocate memory for the new node using malloc*/
    if (newNode == NULL) {
        printf("Error: Unable to allocate memory.\n");
        exit(1);
    }
    newNode->macro_sentence = NULL;
    memset(newNode->macro_name, '\0', MAX_LINE_LENGTH);
    newNode->next = NULL; /* Make next point to NULL*/
    return newNode; /* Return the new node*/
}

/* Check if macro's name is valid, return 1 if valid and 0 if invalid */
int is_macro_name_valid(char* macro_name, char** invalid_macro_names)
{
	
	char** p = invalid_macro_names;
	/* Compare the macro name with each invalid macro name in the array */
	while (*p != NULL)
	{
		if (strcmp(macro_name, *p) == 0)
			return 0;
		p++;
	}
	return 1;
}

/* Check if the macro already exists in the data, return 1 if it does and 0 if not */
int is_macro_exist(char* nameOfMacro, Macro* head)
{
	Macro* curr = head;
	while (curr != NULL)
	{
		if (strcmp(curr->macro_name, nameOfMacro) == 0)
		{
			return 1;
		}
		curr = curr->next;
	}
	return 0;
}

/* Check if the line contains the keyword "macro" or "endmacro" */
int macro_validation(char line[])
{
    int lineIndex = 0;
    int macroIndex = 0;
    char macro[MAX_LINE_LENGTH];
    memset(macro, '\0', MAX_LINE_LENGTH);

    /* Skip leading spaces */
    while (isspace(line[lineIndex]))
        lineIndex++;

    /* Extract the first word from the line */
    while (!isspace(line[lineIndex]) && line[lineIndex] != '\n')
    {
        macro[macroIndex] = line[lineIndex];
        macroIndex++;
        lineIndex++;
    }

    if (strcmp(macro, "mcro") == 0)
    {
        return 1; /* Return 1 if it is "macro" */
    }
    if (strcmp(macro, "endmcro") == 0)
    {
        return 2; /* Return 2 if it is "endmacro" */
    }
    return 0; /* Return 0 if it is neither "macro" nor "endmacro" */
}

/* Insert all macros into the list */
int insert_macro(char* file_name, Macro** head)
{
    char line[MAX_LINE_LENGTH];
    char* input_fname;
    FILE* fpR;
    input_fname = name_file(file_name, INPUT_FILE); /* Get input file name */
    memset(line, '\0', MAX_LINE_LENGTH);

    fpR = fopen(input_fname, "r");
    if (fpR == NULL)
    {
        printf("Error: Can't open file: %s\n", input_fname);
        return 1; /* Return error if unable to open input file */
    }

    while (fgets(line, MAX_LINE_LENGTH, fpR))
    {
        if (strlen(line) == MAX_LINE_LENGTH - 1 && line[MAX_LINE_LENGTH - 2] != '\n')
        {
            /* Read and discard remaining characters of the long line */
            int ch;
            while ((ch = fgetc(fpR)) != EOF && ch != '\n')
                ; /* Consume characters until newline or EOF */
        }

        if (macro_validation(line) == 1) /* Check if line contains a macro */
        {
            Macro* temp = create_node(); /* Create a new macro node */
            if (insert_macro_name(temp, line)) /* Insert macro name into the node */
            {
                if (is_macro_exist(temp->macro_name, *head)) /* Check if macro name already exists */
                {
                    printf("Error: Macro name - %s already exists.\n", temp->macro_name);
                    return 1; /* Return error if macro name already exists */
                }
                insert_macro_sentence(temp, fpR); /* Insert macro sentences into the node */
                add_macro_node(temp, head); /* Add the node to the macro list */
            }
            else
            {
                return 1; /* Return error if unable to insert macro name */
            }
        }
    }
    free(input_fname); /* Free allocated memory for input file name */
    fclose(fpR); /* Close input file */
    return 0; /* Return success */
}

 /* Insert macro name into the list */
int insert_macro_name(Macro* temp, char line[])
{
    char macro[MAX_LINE_LENGTH];
    char* invalid_macro_names[] = { "mov", "cmp", "add", "sub", "lea", "not", "clr", "lea", "inc", "dec", "jmp", "bne", "red", "prn","jsr","rts","stop", ".string", ".data", ".entry", ".extern", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", NULL };

    int lineIndex = 0;
    int macroIndex = 0;

    memset(macro, '\0', MAX_LINE_LENGTH);

    /* Skip leading spaces */
    while (isspace(line[lineIndex]))
        lineIndex++;

    /* Skip the first word (macro keyword) */
    while (!isspace(line[lineIndex]))
        lineIndex++;

    /* Skip any intermediate spaces */
    while (isspace(line[lineIndex]))
        lineIndex++;

    /* Extract the macro name */
    while (!isspace(line[lineIndex]) && line[lineIndex] != '\n')
    {
        macro[macroIndex] = line[lineIndex];
        macroIndex++;
        lineIndex++;
    }

    /* Check if the extracted macro name is valid */
    if (is_macro_name_valid(macro, invalid_macro_names) == 1)
        strcpy(temp->macro_name, macro); /* Copy the valid macro name */
    else
    {
        printf("Error: The macro name %s is a reserved keyword.\n", temp->macro_name);
        free(temp); /* Free the allocated memory for the node */
        return 0;
    }

    return 1; /* Successfully inserted the macro name */
}

/* Insert the sentences of the macro into the list */
void insert_macro_sentence(Macro *temp, FILE *fpR) {
  
    char line[MAX_LINE_LENGTH];
    char *sentence = NULL, *new_sentence = NULL;/* Declare a pointer for the sentence */
    unsigned int  sentence_length = 0; /* To keep track of the sentence length */
	unsigned int new_length = 0;
	

    memset(line, '\0', MAX_LINE_LENGTH);
    fgets(line, MAX_LINE_LENGTH, fpR);

	sentence = (char *)calloc(MAX_LINE_LENGTH, sizeof(char)); /* Allocate memory for macro_sentence*/
    if (sentence == NULL) {
        printf("Error: Unable to allocate memory for macro_sentence.\n");
        exit(1);
    }

    while (macro_validation(line) != 2) /* Return 2 if it is "endmacro" */ {
        /* Calculate the new length for the sentence */
        new_length = sentence_length + strlen(line);

        /* Allocate memory for the new sentence using realloc */
        new_sentence = (char *)realloc(sentence, new_length + 1);
        if (new_sentence == NULL) {
            /* Handle allocation failure */
            printf("Error: Unable to allocate memory for sentence.\n");
            exit(1);
        }

        /* Copy the new line to the end of the sentence */
        strcpy(new_sentence + sentence_length, line);

        sentence = new_sentence; /* Update the sentence pointer */
        sentence_length = new_length; /* Update the sentence length */

        fgets(line, MAX_LINE_LENGTH, fpR);
    }

    /* Null-terminate the sentence */
    sentence[sentence_length] = '\0';

    /* Assign the dynamically allocated sentence to the node */
    temp->macro_sentence = sentence;
}

/* Add node to the macro list */
void add_macro_node(Macro* temp, Macro** head)
{
    
    Macro* node = *head; /* Start from the head of the list */

    if (*head == NULL)
    {
        /* If the list is empty, make temp the first node */
        *head = temp;
    }
    else
    {
        /* Traverse the list to find the last node */
        while (node->next != NULL)
        {
            node = node->next;
        }
        /* Append the new node to the end of the list */
        node->next = temp;
    }
}

 /* Search if there is a macro in the line */
int find_macro(char line[], FILE* fpW, Macro** head)
{
    int lineIndex = 0, macroIndex = 0;
    Macro* temp;
    char macro[MAX_LINE_LENGTH];
    memset(macro, '\0', MAX_LINE_LENGTH);
    temp = *head;

    /* Skip leading spaces */
    while (isspace(line[lineIndex]))
        lineIndex++;

    /* Extract macro name from the line */
    while (!isspace(line[lineIndex]) && line[lineIndex] != '\n')
    {
        macro[macroIndex] = line[lineIndex];
        macroIndex++;
        lineIndex++;
    }

    /* Iterate through the macro list to find a matching macro name */
    while (temp != NULL)
    {
        if (strcmp(temp->macro_name, macro) == 0)
        {
            fprintf(fpW, "%s", temp->macro_sentence); /* Write macro sentences to the output file */
            return 1; /* Macro found and written */
        }
        temp = temp->next; /* Move to the next macro in the list */
    }

    return 0; /* No matching macro found in the line */
}

/* Write to a new file the text and replace the macro name with the sentences */
void macro_open(char *file_name, Macro** head)
{
    int macroFlag = 0; /* Flag to track if inside a macro definition */
    FILE* fpR;
    FILE* fpW;
    char line[MAX_LINE_LENGTH];
    char *input_fname;
    char *output_fname;
    memset(line, '\0', MAX_LINE_LENGTH);

    input_fname = name_file(file_name, INPUT_FILE);
    output_fname = name_file(file_name, MACRO_FILE);

    fpR = fopen(input_fname, "r");
    fpW = fopen(output_fname, "w");
    if (fpR == NULL)
    {
        printf("Error: Can't open file: %s\n", input_fname);
        return;
    }

    while (fgets(line, MAX_LINE_LENGTH, fpR))
    {
        if (!macroFlag)
        {
            if (!find_macro(line, fpW, head)) /* Check if the line contains a macro call */
            {
                if (macro_validation(line) == 0) /* If not a macro call, write the line as is */
                {
                    fprintf(fpW, "%s", line);
                }
                else
                {
                    macroFlag = 1; /* Set the flag as entering a macro */
                }
            }
        }
        else
        {
            if (macro_validation(line) == 2) /* Check for the end of a macro */
                macroFlag = 0; /* Reset the flag as leaving the macro */
        }
    }

    free(input_fname);
    free(output_fname);
    fclose(fpR);
    fclose(fpW);
}


/* Clear the macro list and free the memory */
void clear_macro_list(Macro **list) {
    Macro *curr = *list; /* Start from the beginning of the list */
    Macro *nextNode;

    while (curr != NULL) {
        /* Preserve the pointer to the next node */
        nextNode = curr->next;

        /* Free the dynamically allocated macro_sentence */
        free(curr->macro_sentence);

        /* Free the curr node */
        free(curr);

        /* Move to the next node */
        curr = nextNode;
    }

    /* Set the list pointer to NULL to indicate an empty list */
    *list = NULL;
}

