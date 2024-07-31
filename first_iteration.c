#include "compile.h"
#include <stdio.h>
#include <string.h>

/*** Function prototypes ***/

/* Build the symbol table based on the provided line_ast, update data_code if needed */
void build_symbol_table(syntax_ast line_ast, node_ref symbol_head, int data_code[]);

/* Update the address of symbols in the symbol table */
void update_symbol_address(node_ref symbol_head);

/* Check if the provided syntax_ast has an error */
int is_error(syntax_ast);

/* Check if the provided syntax_ast defines a label */
int def_label(syntax_ast);

/* Encode an array of integers as data into the data_code array */
void encode_data(int data[], int cnt_data, int data_code[]);

/* Encode a string as data into the data_code array */
void encode_string(char string[], int data_code[]);



extern int cnt_line;	/* line number to show in errors */
extern bool has_error;	/* to prevent output file creation if an error was found */
extern int ic;
extern int dc;
extern char *file_am_name;


/* coding data code and build symbol table*/
void first_iteration(FILE *am_file, node_ref symbol_head, int data_code []){
    syntax_ast line_ast = {0};
	char line_buffer[MAX_LINE_LENGTH] = {0};
	char ch;
    /* Process each line */
    while (fgets(line_buffer,sizeof(line_buffer),am_file))
	{
		cnt_line++; /* line counter */
		if (strlen(line_buffer) == MAX_LINE_LENGTH - 1 && line_buffer[MAX_LINE_LENGTH - 2] != '\n') {
			printf("\n Error in %s, line %d: line in input file is longer than 80 chars ", file_am_name, cnt_line);
			has_error = TRUE;

            /* Read and discard remaining characters of the long line*/
            while ((ch = fgetc(am_file)) != EOF && ch != '\n')
                ; /* Consume characters until newline or EOF*/
			continue;
        }

	    /* skip comment and empty line*/
		if (comment_empty_line(line_buffer)){
			continue;
		}
		line_ast = syntax_ast_parse(line_buffer);
		build_symbol_table(line_ast, symbol_head,data_code);	
    }
	if(ic + dc + INITIAL> MAX_CODE){
		printf("Error: memory required exceeds the limits of the RAM in the machine");
		has_error = TRUE;
	}
	update_symbol_address(symbol_head);
}
	
/* Build the symbol table based on the provided line_ast, update data_code if needed */
void build_symbol_table(syntax_ast line_ast, node_ref symbol_head, int data_code[]) {
    /* Check if the line has an error */
    if (is_error(line_ast)) {
        printf("\nError in %s, line %d: %s \n",file_am_name, cnt_line, line_ast.syntax_error);
        has_error = TRUE;
        return;

    }else if (def_label(line_ast)) {
        /* Check if the line defines a label */
        if (line_ast.opt_ast == instruct)
            add_symbol(symbol_head, ic + INITIAL, line_ast.label_name, code_relocatable);

        else if (line_ast.opt_ast == direct) {
            if (line_ast.dir_inst.direct.dir_opt == opt_string || line_ast.dir_inst.direct.dir_opt == opt_data)
                add_symbol(symbol_head, dc + INITIAL, line_ast.label_name, data_relocatable);

            else
                printf("\n *Warning* in %s, line %d: declaration of symbol before entry or extern instructions  \n",file_am_name ,cnt_line);
        }
    }
    switch (line_ast.opt_ast) {
        case instruct:
            /* Process instruction */
            ic++;
            if ((line_ast.dir_inst.instruct.inst_opt >= instruct_mov && line_ast.dir_inst.instruct.inst_opt <= instruct_sub) ||
                line_ast.dir_inst.instruct.inst_opt == instruct_lea) {
                if (line_ast.dir_inst.instruct.op_met[1] == reg_num_op && line_ast.dir_inst.instruct.op_met[0] == reg_num_op)
                    ic++;/*only one word for two registers as opeands*/
                else
                    ic += 2;/*two opearnds to be added*/
            } else if (line_ast.dir_inst.instruct.inst_opt == instruct_not ||
                       line_ast.dir_inst.instruct.inst_opt == instruct_clr ||
                       (line_ast.dir_inst.instruct.inst_opt >= instruct_inc &&
                        line_ast.dir_inst.instruct.inst_opt <= instruct_jsr)) {
                ic++;
            }
            break;
        case direct:
            /* Process directive */
            if (line_ast.dir_inst.direct.dir_opt == opt_data) {
                encode_data(line_ast.dir_inst.direct.direct_params.data_s.data_arr,
                            line_ast.dir_inst.direct.direct_params.data_s.cnt_data, data_code);
            } else if (line_ast.dir_inst.direct.dir_opt == opt_string) {
                encode_string(line_ast.dir_inst.direct.direct_params.string, data_code);
            } else if (line_ast.dir_inst.direct.dir_opt == opt_extern) {
                add_symbol(symbol_head, 0, line_ast.dir_inst.direct.direct_params.symbol, external);
            }
            break;
    }
}

/* Update the address of symbols in the symbol table */
void update_symbol_address(node_ref symbol_head){
	symbol_node *current;
	node_ptr temp; /* to go over the list */

	temp = *symbol_head;
	/* If the list is empty return null */
	if (temp == NULL)
		return;
	do
	/*iterates over the list to find data symbols*/
	{
		current = (symbol_node*)(get_data(temp));
		if (get_type(current) == data_relocatable)
			set_address(current, ic); /*update address*/
		
	} while ((temp = get_next(temp)) != NULL);
}


/* Check if there is a syntax error in the line_ast */
int is_error(syntax_ast line_ast) {
    if (*(line_ast.syntax_error) != '\0') {
        return 1; /* Return 1 if there's a syntax error*/
    } else {
        return 0; /* Return 0 if there's no syntax error*/
    }
}

/* Check if a label is defined in the line_ast */
int def_label(syntax_ast line_ast) {
    if (*(line_ast.label_name) != '\0') {
        return 1; /*Return 1 if a label is defined*/
    } else {
        return 0; /* Return 0 if no label is defined*/
    }
}

/* Encode an array of integers as data into the data_code array */
void encode_data(int data[], int cnt_data, int data_code[]) {
    int i;
    unsigned short int coding = 0;
    
    for (i = 0; i < cnt_data; i++) {
        /* Convert each data element to its value */
        coding = data[i];
        
        /* Check if there's space in the data_code array to store the value */
        if (dc < MAX_CODE) {
            data_code[dc++] = coding;
        }
    }
}

/* Encode a string as data into the data_code array */
void encode_string(char string[], int data_code[]) {
    int i;
    int len = strlen(string);
    unsigned short int coding = 0;
    
    for (i = 0; i < len; i++) {
        /* Convert each character to its ASCII value */
        coding = string[i];
        
        /* Check if there's space in the data_code array to store the value */
        if (dc < MAX_CODE) {
            data_code[dc++] = coding;
        }
    }
    
    /* Add a null terminator character to signify the end of the string */
    data_code[dc++] = 0;
}






