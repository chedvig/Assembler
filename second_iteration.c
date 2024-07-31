/*** second_iteration.c file encode instruction assembly code and creates output files ***/
#include "compile.h"

/*** Global vars ***/
extern int cnt_line;	/* line number to show in errors */
extern bool has_error;	/* to prevent output file creation if an error was found */
extern int ic;
extern int dc;
extern char *file_am_name;

/*** functions prototypes ***/

/* Encode the command portion of an assembly line */
void encode_command(syntax_ast ast, int instruction[]);

/* Encode the operands of an assembly line */
void encode_operands(syntax_ast ast, node_ref symbol_head, node_ref extern_list, int instruction[]);

/* Add an entry symbol to the entry list */
void add_entry(char symbol[], node_ref symbol_head, node_ref entry_list);

/* Extract specific bits from an unsigned integer */
unsigned int extract_bits(unsigned int number, int startBit, int numBits);

/* Convert an unsigned integer to base-64 representation */
char *convert_to_base_64(unsigned int num);

/* Create an object file from encoded instructions and data */
void create_obj_file(char *name, int instruction[], int data_code[]);

/* Create entry/external files for symbols */
void create_ent_ext_file(char *name, node_ref list, char *suffix);


/*ecoding the instructions sentences according to symbol table an creates output files */
void second_iteration(char *file_name, FILE *am_file, node_ref symbol_head, int instruction[], int data_code[]) {
    syntax_ast line_ast = {0}; /* Syntax AST structure to hold parsed line */
    char line_buffer[MAX_LINE_LENGTH] = {0}; /* Buffer to read each line */
    node_ptr extern_list = NULL; /* List of external symbols */
    node_ptr entry_list = NULL; /* List of entry symbols */
    ic = 0; /* Initialize instruction counter */
    cnt_line = 0; /* Initialize line counter */
    
    /* Loop through each line in the assembly file */
    while (fgets(line_buffer, sizeof(line_buffer), am_file)) {
        cnt_line++; /* Increment line counter */

        /* Skip empty lines and comments */
        if (comment_empty_line(line_buffer)) {
            continue;
        }

        /* Parse the line and obtain the syntax AST */
        line_ast = syntax_ast_parse(line_buffer);

        switch (line_ast.opt_ast) {
            case instruct:
                /* Process and encode instruction lines */
                encode_command(line_ast, instruction);
                encode_operands(line_ast, symbol_head, &extern_list, instruction);
                break;
            case direct:
                if (line_ast.dir_inst.direct.dir_opt == opt_entry) {
                    /* Add labels as entry symbols */
                    add_entry(line_ast.dir_inst.direct.direct_params.symbol, symbol_head, &entry_list);
                } else {
                    continue; /* Skip other directive types */
                }
        }
    }

    /* Generate output files if no errors were encountered */
    if (!has_error) {
        create_obj_file(file_name, instruction, data_code);
        if (entry_list != NULL) {
            create_ent_ext_file(file_name, &entry_list, ENTRIES_FILE);
        }
        if (extern_list != NULL) {
            create_ent_ext_file(file_name, &extern_list, EXTERN_FILE);
        }
    }

    /* Clear the lists and free memory */
    clear_list(&extern_list);
    clear_list(&entry_list);
}



void encode_command(syntax_ast ast, int instruction []){
    unsigned int binary_coding = 0;
    binary_coding = ast.dir_inst.instruct.op_met[1] << 2; /*dest bits*/
    binary_coding |= ast.dir_inst.instruct.op_met[0] << 9;/*src bits*/
    binary_coding |= ast.dir_inst.instruct.inst_opt << 5;/*opcode*/
    instruction[ic++] = binary_coding;
}

void encode_operands(syntax_ast ast, node_ref symbol_head, node_ref extern_list, int instruction[]) {
    unsigned int binary_coding = 0; /* Binary coding for the operands */
    int i;
    symbol_node *found_symbol; /* Pointer to the found symbol node */

    /* Check if the instruction is with no operands */
    if (ast.dir_inst.instruct.inst_opt >= instruct_rts) 
    {
        return; /* Return if the instruction is a directive greater than or equal to RTS */
    }
    else
    {
        if (ast.dir_inst.instruct.op_met[1] == reg_num_op && ast.dir_inst.instruct.op_met[0] == reg_num_op) {
            /* Handle case where both operands are register numbers */
            binary_coding = ast.dir_inst.instruct.op_opt[1].reg_num << 2;/*dest register operand*/
            binary_coding |= ast.dir_inst.instruct.op_opt[0].reg_num << 7;/*src register operand*/
            instruction[ic++] = binary_coding;
            binary_coding = 0;
        } else {
            for (i = 0; i < 2; i++) {
                switch (ast.dir_inst.instruct.op_met[i]) {
                    case num_op:
                        /* Handle numeric operand */
                        binary_coding = ast.dir_inst.instruct.op_opt[i].num << 2; /*represent the num in 10 bits*/
                        instruction[ic++] = binary_coding;
                        binary_coding = 0;
                        break;

                    case reg_num_op:
                        /* Handle register number operand */
                        binary_coding = ast.dir_inst.instruct.op_opt[i].num << (7 - (i * 5));/*depends on i if the register is src or dest*/
                        instruction[ic++] = binary_coding;
                        binary_coding = 0;
                        break;

                    case label_op:
                        /* Handle symbol operand */
                        found_symbol = get_symbol(symbol_head, ast.dir_inst.instruct.op_opt[i].symbol);

                        if (!found_symbol) {
                            printf("\nError in %s, line %d: use of symbol as operand but the symbol was never defined\n", file_am_name,cnt_line);
                            has_error = TRUE; /* Set error flag */
                            return;
                        }

                        binary_coding = get_address(found_symbol) << 2; /*represent the address of the symbol in 10 bits*/

                        if (get_type(found_symbol) == external) {
                            binary_coding |= 1; /*A,R,E = E */
                            add_symbol(extern_list, ic + INITIAL, ast.dir_inst.instruct.op_opt[i].symbol, external);
                        } else {
                            binary_coding |= 2/*A,R,E = R */;
                        }

                        instruction[ic++] = binary_coding;
                        binary_coding = 0;
                        break;
                        

                    case zero_op:
                        continue; /* Skip zero operand */
                        break;
                }
            }
        }
    }
}


void add_entry(char symbol[], node_ref symbol_head, node_ref entry_list) {
    /* Add a symbol as an entry to the entry list */

    symbol_node *found_symbol; /* Pointer to the found symbol node */

    /* Find the symbol node in the symbol list */
    found_symbol = get_symbol(symbol_head, symbol);

    if (!found_symbol) {
        printf("\nError in %s, line %d: declaration of symbol as entry but symbol does not exist\n",file_am_name,cnt_line);
        has_error = TRUE; /* Set error flag */
        return;
    }

    set_type(found_symbol, entry); /* Set the type of the found symbol to entry */

    /* Add the symbol to the entry list with its address and name */
    add_symbol(entry_list, get_address(found_symbol), get_symbol_name(found_symbol), entry);
}


 /* Extract a specific range of bits from an unsigned integer */
unsigned int extract_bits(unsigned int  number, int startBit, int numBits) {
    /* Create a bitmask to extract the desired bits*/
    unsigned int bitmask = ((1 << numBits) - 1) << startBit;

    /* Extract the desired bits using bitwise AND*/
    unsigned int extractedBits = (number & bitmask) >> startBit;

    return extractedBits;
}

/* Converting a word to 2 digits in base 64 (as a string) */
char *convert_to_base_64(unsigned int num)
{
    char *obj; /* Pointer to the result string in base 64 */
    char base64[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; /* Base 64 characters */
    obj = (char*)calloc(3, sizeof(char)); /* Allocate memory for the result string */

    /* Convert the input number to base 64 representation */
    obj[0] = base64[extract_bits(num, 6, 6)]; /* Extract and map first 6 bits */
    obj[1] = base64[extract_bits(num, 0, 6)]; /* Extract and map next 6 bits */
    obj[2] = '\0'; /* Null-terminate the result string */

    return obj; /* Return the base 64 representation as a string */
}


void create_obj_file(char *name, int instruction[], int data_code[]) {
    /* Create and write to the object file */

    char *temp1; /* Temporary string for base 64 conversion */
    int i;
    FILE *obj_file;
    char *obj_file_name = name_file(name, OBJECT_FILE); /* Construct the object file name */
    obj_file = fopen(obj_file_name, "w"); /* Open the object file for writing */

    if (obj_file == NULL) {
        printf("Error: could not open objects file");
        exit(1); /* Exit if file opening fails */
    }

    free(obj_file_name); /* Free the allocated memory for file name */

    fprintf(obj_file, "\n%d %d", ic, dc); /* Write the values of ic and dc */

    /* Write instructions to the object file */
    for (i = 0; i < ic; i++) {
        temp1 = convert_to_base_64(instruction[i]); /* Convert instruction to base 64 */
        fprintf(obj_file, "\n%s", temp1); /* Write the converted instruction */
        free(temp1); /* Free the temporary string memory */
    }

    /* Write data code to the object file */
    for (i = 0; i < dc; i++) {
        temp1 = convert_to_base_64(data_code[i]); /* Convert data code to base 64 */
        fprintf(obj_file, "\n%s", temp1); /* Write the converted data code */
        free(temp1); /* Free the temporary string memory */
    }

    fclose(obj_file); /* Close the object file */
}


void create_ent_ext_file(char *name, node_ref list, char *suffix) {
    /* Create and write to an entry or external file */

    FILE *fp;
    symbol_node* curr_symbol;
    node_ptr temp; /* Iterator */
    char *file_name = name_file(name, suffix); /* Construct the file name */

    fp = fopen(file_name, "w"); /* Open the file for writing */

    if (fp == NULL) {
        printf("Error: could not open file");
        exit(1); /* Exit if file opening fails */
    }

    free(file_name); /* Free the allocated memory for file name */

    temp = *list; /* Start from the head of the list */

    if (temp == NULL)
        return; /* Return if the list is empty */

    do {
        curr_symbol = (symbol_node *)(get_data(temp)); /* Get the symbol node data */
        fprintf(fp, "\n%s\t%d", get_symbol_name(curr_symbol), get_address(curr_symbol));
        /* Write symbol name and address to the file */

    } while ((temp = get_next(temp)) != NULL); /* Iterate through the list */

    fclose(fp); /* Close the file */
}



 
