#include "compile.h"
#include "define.h"

extern int cnt_line;	/* line number to show in errors */
extern bool has_error;	/* to prevent output file creation if an error was found */
extern int ic;  
extern int dc;
extern char *file_am_name;

/* reset global vars and code_arrays*/
void initialize_vars(int instruction[], int data_code[]);

/* compiles the am_file */
void compile(char *file_name) {
    FILE *fp;
    char *amfile_name;
   
    int instruction[MAX_CODE];
    int data_code[MAX_CODE];
    node_ptr symbol_head = NULL; 
    
    /* Generate the name of the macro file */
    amfile_name = name_file(file_name, MACRO_FILE);
    /* Open the macro file for reading */
    fp = fopen(amfile_name, "r");
    if (!fp) {
        printf("Error: couldn't open file");
        free(amfile_name);
        exit(1);
    } else {
        /* Initialize instruction and data_code arrays */
        initialize_vars(instruction, data_code);
        file_am_name = amfile_name;
        
        /* Perform the first iteration of the compilation process */
        first_iteration(fp, &symbol_head, data_code);
        
        /* If no errors occurred during the first iteration, proceed to the second iteration */
        if (!has_error) {
            /* Rewind the file pointer to the beginning of the macro file */
            rewind(fp);
            
            /* Perform the second iteration of the compilation process  */
            second_iteration(file_name, fp, &symbol_head, instruction, data_code);
        } 
         
    }
    fclose(fp);
    free(amfile_name);
    /* Clear the linked list of symbols and free memory */
    clear_list(&symbol_head); 
}

/* reset global vars and code_arrays*/
void initialize_vars(int instruction[], int data_code[]){
    int i;
    cnt_line = 0;	/* line number to show in errors */
    has_error = FALSE;	/* to prevent output file creation if an error was found */
    ic = 0;
    dc = 0;
    file_am_name = NULL;
    for(i = 0; i < MAX_CODE; i++)
    /*resetting the code_arrays*/
    {
        instruction[i] = 0;
        data_code[i] = 0;
    }
}

/*generates name file according to the type*/
char *name_file(char *original, char *type)
{
    char *modified = (char *) malloc(strlen(original) + MAX_EXTENSION_LENGTH);
 valid_allocate(modified);

    strcpy(modified, original); /* Copying original filename to the bigger string */

    /* Concatenating the required file extension */

    strcat (modified,type);
    return modified;
}

/*check for comment or empty line*/
int comment_empty_line(char* line_buffer){
	erase_spaces(line_buffer)
	if(line_buffer == NULL || *line_buffer == '\0' || *line_buffer == ';'){
		return 1;
	}
	return 0;
}
