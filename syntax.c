/** syntax.c handles all of the functions regarding syntax. given a line, analyze it and returns ast which represents it*/
#include "syntax.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*** functions prototypes ***/

static int find_label(char **line, syntax_ast *sa);
/* Find a label in the given line and update the syntax_ast accordingly */

static int valid_label(char* symbol, syntax_ast *sa);
/* Validate if the given symbol is a valid label and update the syntax_ast accordingly */

static int word_exist_inst(char* word);
/* Check if the given word exists in the instruction set */

static int word_exist_direct(char* word);
/* Check if the given word exists in the directive set */

static void valid_ops_instruct(char* ops_str, char* src_ops, char* dest_ops, syntax_ast *sa);
/* Validate operands for an instruction syntax and update the syntax_ast accordingly */

int valid_num(char *op, int min, int max, bool is_negative);
/* Validate if the given string is a valid number within the specified range */

char* trim_string(char* str);
/* Trim leading and trailing whitespace from the given string */

static char validate_op(char* op, syntax_ast *sa, int i);
/* Validate an operand and update the syntax_ast accordingly */

static void valid_ops_direct(char *ops_str, enum direct_opt dir_opt, syntax_ast *sa);
/* Validate operands for a directive syntax and update the syntax_ast accordingly */

static void valid_data(char *ops_str, syntax_ast *sa);
/* Validate data operands and update the syntax_ast accordingly */

static void valid_string(char* ops_str, syntax_ast *sa);
/* Validate string operands and update the syntax_ast accordingly */

char get_content(char* s);
/* Get the content of a character pointer */

static char* split_ops(char **ops, char *src_ops, char *dest_ops, syntax_ast *sa);
/* Split and extract operands from the given string and update the syntax_ast accordingly */

bool is_printable(char *str);
/* Check if the given string contains only printable characters */



#define find_first_space(ops) while (ops != NULL && *ops != '\0' && !isspace(*ops)) ops++; 

typedef struct {
    const char *inst_name;     /* Name of the assembly instruction */
    enum instruct_opt inst_opt; /* Enum representing the instruction's option */
    char *source_ops;          /* Allowed types of source operands */
    char *dest_ops;            /* Allowed types of destination operands */
} instruct_ast_lex;

/* Define an array of instruct_ast_lex structures to store instruction information */
static instruct_ast_lex inst_ast_lex[NUM_OF_INSTRUCTS] = {
    {"mov", instruct_mov, "ILR", "LR"},
    {"cmp", instruct_cmp, "ILR", "ILR"},
    {"add", instruct_add, "ILR", "LR"},
    {"sub", instruct_sub, "ILR", "LR"},
    {"not", instruct_not, NULL, "LR"},
    {"clr", instruct_clr, NULL, "LR"},
    {"lea", instruct_lea, "L", "LR"},
    {"inc", instruct_inc, NULL, "LR"},
    {"dec", instruct_dec, NULL, "LR"},
    {"jmp", instruct_jmp, NULL, "LR"},
    {"bne", instruct_bne, NULL, "LR"},
    {"red", instruct_red, NULL, "LR"},
    {"prn", instruct_prn, NULL, "ILR"},
    {"jsr", instruct_jsr, NULL, "LR"},
    {"rts", instruct_rts, NULL, NULL},
    {"stop", instruct_stop, NULL, NULL}
};

typedef struct {
    const char *direct_name;   /* Name of the directive */
    enum direct_opt dir_opt;   /* Enum representing the directive's option */
} direct_ast_lex;

/* Define an array of direct_ast_lex structures to store directive information */
static direct_ast_lex dir_ast_lex[NUM_OF_DIRECTS] = {
    {".entry", opt_entry},    
    {".extern", opt_extern},  
    {".data", opt_data},      
    {".string", opt_string}   
};

/*build abstract syntax tree from the given line*/
syntax_ast syntax_ast_parse(char *line_str) {
    char *line = NULL, *ops = NULL, *initial = NULL;
    int i = 0;
    syntax_ast sa = {0};

    /* Allocate memory for line and verify allocation */
    line = (char*) calloc(MAX_LINE_LENGTH, sizeof(char));
    valid_allocate(line);
    
    /* Copy the input line to 'line' */
    strncpy(line, line_str, strlen(line_str));
    initial = line;

    /* Trim leading spaces from line */
    if (line != NULL) {
        erase_spaces(line);
    }

    /* Check for label existence and handle error case */
    if (find_label(&line, &sa) == -1) {
        free(initial);
        return sa;
    }

    /* Trim leading spaces from line after processing label */
    if (line != NULL) {
        erase_spaces(line);
    }

    /* Handle cases with no data or instructions after label declaration */
    if ((line == NULL || *line == '\0') && *(sa.label_name) != '\0') {
        sprintf(sa.syntax_error, "Error: Declaration of symbol with no data or instructions");
        free(initial);
        return sa;
    }

    /* Divide the line into command and ops */
    ops = line;
    find_first_space(ops);
    if (ops != NULL) {
        *ops = '\0';
        ops++; /* Move ops pointer to the start of operands */
    }

    /* Check for directives starting with '.' */
    if (*line == '.') {
        i = word_exist_direct(line);
        if (i >= 0) {
            direct_ast_lex dir = dir_ast_lex[i];
            sa.opt_ast = direct;
            sa.dir_inst.direct.dir_opt = dir.dir_opt;
            valid_ops_direct(ops, dir.dir_opt, &sa);
            free(initial);
            return sa;
        } else {
            sprintf(sa.syntax_error, "Error: unknown command name");
            free(initial);
            return sa;
        }
    }
    /* Check for instructions */
    i = word_exist_inst(line);
    if (i >= 0) {
        instruct_ast_lex inst = inst_ast_lex[i];
        sa.opt_ast = instruct;
        sa.dir_inst.instruct.inst_opt = inst.inst_opt;
        valid_ops_instruct(ops, inst.source_ops, inst.dest_ops, &sa);
        free(initial);
        return sa;
    } else {
        sprintf(sa.syntax_error, "Error: unknown command name");
        free(initial);
        return sa;
    }
}


/* Find a label in the given line and update the syntax_ast accordingly */
static int find_label(char **line, syntax_ast *sa) {
    char *symbol;
    char *end_of_label;

    /* Find the position of the ':' character*/
    end_of_label = strchr(*line, ':');

    /* If ':' is not found, return 0 indicating no symbol*/
    if (end_of_label == NULL) {
        return 0;
    }
    /* Store the symbol part in 'symbol'*/
    symbol = *line;
    *end_of_label = '\0'; /* Null-terminate the symbol part*/

    /* Move the 'line' pointer to the next character after ':'*/
    *line = end_of_label + 1;

    if (valid_label(symbol, sa)) {
        strcpy(sa->label_name, symbol);
        return 1; /* Label found and valid*/
    } else {
        return -1; /* Label found but invalid*/
    }
}

/* Validate if the given symbol is a valid label and update the syntax_ast accordingly */
static int valid_label(char* symbol, syntax_ast *sa) {
    char* str;
    char* trimmed_label = trim_string(symbol);

    /* Check for maximum symbol length */
    if (strlen(trimmed_label) > MAX_SYMBOL_LENGTH) {
        sprintf(sa->syntax_error, "Error: invalid length of label");
        return 0;
    }
    /* Check if the first character is alphabetic */
    else if (!isalpha(*trimmed_label)) {
        sprintf(sa->syntax_error, "Error: first character of symbol is not alphabetic");
        return 0;
    }
    
    str = trimmed_label;
    /* Check for non-alphanumeric characters */
    while (*str != '\0') {
        if (!isalnum(*str)) {
            sprintf(sa->syntax_error, "Error: symbol includes non-alphanumeric character, not valid");
            return 0;
        }
        str++;
    }
    
    /* Check if the label is a reserved keyword for instructions */
    if (word_exist_inst(trimmed_label) >= 0) {
        sprintf(sa->syntax_error, "Error: symbol name is not valid");
        return 0;
    }else{
        return 1;
    }
}

/* Validate operands for an instruction syntax and update the syntax_ast accordingly */
static void valid_ops_instruct(char *ops_str, char *src_ops, char *dest_ops, syntax_ast *sa) {
    char *op1 = NULL, *op2 = NULL, *str = NULL, *op = NULL;
    int num_of_ops = ONE_OP, i = 0;;
    
    op1 = ops_str;

    /* If both source and destination operands are NULL, no validation needed */
    if (src_ops == NULL && dest_ops == NULL) {
        if(ops_str != NULL){
            erase_spaces(ops_str)
            if(*ops_str != '\0'){
                sprintf(sa->syntax_error, "Error: extranous text after non operand instruction");
                return;
            }
        }
        return;
    }

    /* Split ops_str to 2 operands */
    op2 = split_ops(&op1, src_ops, dest_ops, sa);
    if (op2 != NULL) {
        if (strcmp(op2, "ERROR") == 0) {
            return;
        } else {
            num_of_ops = TWO_OPS; /**/
        }
    }

    op = op1; /*for the while iterartion the pointer *str is for knowing the expected operands type*/
    do {
        if (num_of_ops == ONE_OP) {
            str = dest_ops; /* one opernad is always dest*/
            sa->dir_inst.instruct.op_met[i] = zero_op; /* Indicate zero operand */
            i++;
        } 
        else 
        { /*handling two operands*/
            if (i == 0) {
                str = src_ops; 
            }else {
                str = dest_ops;
                op = op2;
            }
        }
        switch (validate_op(op, sa, i)) {
            case 'E':
                return;
                break;
            case 'I':
                if (strchr(str, 'I') == NULL) {
                    sprintf(sa->syntax_error, "Error: unexpected type of operand");
                    return;
                }
                break;
            case 'R':
                if (strchr(str, 'R') == NULL) {
                    sprintf(sa->syntax_error, "Error: unexpected type of operand");
                    return;
                }
                break;
            case 'L':
                if (strchr(str, 'L') == NULL) {
                    sprintf(sa->syntax_error, "Error: unexpected type of operand");
                    return;
                }
                break;
        }
    } while (i++ < num_of_ops);
}

/* Split and extract operands from the given string and update the syntax_ast accordingly */
static char *split_ops(char **ops1, char *src_ops, char *dest_ops, syntax_ast *sa){
    char *comma = NULL;
    char *op1 = NULL, *op2 = NULL;
    op1 = *ops1;
    /* Remove spaces from ops_str */
    if (op1 != NULL) {
        erase_spaces(op1);
    }

    /* Check if the first character is a comma */
    if (*op1 == ',') {
        sprintf(sa->syntax_error, "Error: illegal comma");
        return "ERROR";
    }

    /* Check if ops_str is empty */
    if (*op1 == '\0') {
            sprintf(sa->syntax_error, "Error: no operands");
            return "ERROR";
        }
    
    /* Find the position of the first comma in ops_str */
    comma = strchr(op1, ',');
    if (comma) {
        /* If there is a comma, check if src_ops is NULL (since we have two operands) */
        if (src_ops == NULL) {
            comma++;
            erase_spaces(comma);
            if(*comma=='\0'){
                sprintf(sa->syntax_error, "Error: illegal comma");
                 return "ERROR";
            }
            else{
                sprintf(sa->syntax_error, "Error: extarnous operands than expected");
                return "ERROR";
            }
        }
        *comma = '\0'; /* Null-terminate op1 string */
        op2 = comma + 1; /* op2 points to the character after the comma */
    }
    if(op2 == NULL || *op2 == '\0'){
        if(src_ops!=NULL){
           sprintf(sa->syntax_error, "Error: missing operand");
           return "ERROR";
        }
    }
    return op2;
}


/* Validate an operand and update the syntax_ast accordingly */
static char validate_op(char* op, syntax_ast *sa, int i) {
    int num;
    bool is_negative = FALSE;

    /* Remove leading spaces from the input operand */
    if (op != NULL) {
        erase_spaces(op);
    }

    /* Check for missing operand */
    if (*op == '\0') {
        sprintf(sa->syntax_error, "Error: missing operand");
        return 'E';
    }

    /* Check for sign indicators (+/-) */
    if (*op == '-') {
        op++;
        is_negative = TRUE;
    }
    if (*op == '+') {
        op++;
    }

    /* Check if the operand is a valid number */
    if (isdigit(get_content(op))) {
        num = valid_num(op, MIN_INTEGER, MAX_INTEGER, is_negative);
        if (num == INVALID_NUM) {
            sprintf(sa->syntax_error, "Error: illegal number");
            return 'E';
        }
        sa->dir_inst.instruct.op_opt[i].num = num;
        sa->dir_inst.instruct.op_met[i] = num_op;
        return 'I'; /* Indicate a valid number operand */
    }

    /* Check if the operand is a valid symbol */
    if (isalpha(*op)) {
        if (!valid_label(op, sa)) {
            sprintf(sa->syntax_error, "Error: illegal symbol as operand");
            return 'E';
        } else {
            strcpy(sa->dir_inst.instruct.op_opt[i].symbol, op);
            sa->dir_inst.instruct.op_met[i] = label_op;
            return 'L'; /* Indicate a valid label operand */
        }
    }

    /* Check for register operands */
    if (*op == '@') {
        int reg_nm;
        char curr;
        op++;
        /* Check for illegal register */
        curr = get_content(op);
        if (curr == 'E') {
            sprintf(sa->syntax_error, "Error: illegal register");
            return 'E';
        }
        /* Check for register name syntax */
        if (curr != 'r') {
            sprintf(sa->syntax_error, "Error: illegal char in register name");
            return 'E';
        }
        op++;
        /* Check for illegal register */
        curr = get_content(op);
        if (curr == 'E') {
            sprintf(sa->syntax_error, "Error: illegal register");
            return 'E';
        }
        reg_nm = curr - '0';
        /* Check for valid register number range */
        if (reg_nm < 0 || reg_nm > 7) {
            sprintf(sa->syntax_error, "Error: illegal number of register");
            return 'E';
        }
        op++;
        /* Check for trailing spaces after register */
        if (op != NULL) {
            erase_spaces(op);
        }
        if (*op != '\0') {
            sprintf(sa->syntax_error, "Error: illegal char after register");
            return 'E';
        } else {
            sa->dir_inst.instruct.op_opt[i].reg_num = reg_nm;
            sa->dir_inst.instruct.op_met[i] = reg_num_op;
            return 'R'; /* Indicate a valid register operand */
        }
    }
    /* Indicate an undefined operand */
    sprintf(sa->syntax_error, "Error: undefined operand");
    return 'E';
}

/* Validate operands for a directive syntax and update the syntax_ast accordingly */
static void valid_ops_direct(char *ops_str, enum direct_opt dir_opt, syntax_ast *sa) {
    /* Remove leading spaces from the input string */
    if (ops_str != NULL) {
        erase_spaces(ops_str);
    }
    /* Check for an empty operand string */
    if (*ops_str == '\0') {
        sprintf(sa->syntax_error, "Error: missing operands");
        return;
    }

    /* Handle different types of directives */
    if (sa->dir_inst.direct.dir_opt == opt_data) {
        valid_data(ops_str, sa); /* Validate data operands */
        return;
    }
    if (sa->dir_inst.direct.dir_opt == opt_entry || sa->dir_inst.direct.dir_opt == opt_extern) {
        char symbol[MAX_SYMBOL_LENGTH];
        strcpy(symbol, trim_string(ops_str)); /* Trim and store the symbol */
        if (valid_label(symbol, sa)) {
            strcpy(sa->dir_inst.direct.direct_params.symbol, symbol);
            return;
        }
        return;
    }
    if (sa->dir_inst.direct.dir_opt == opt_string) {
        valid_string(ops_str, sa); /* Validate string operands */
        return;
    }
}

/* Validate if the given string is a valid number within the specified range */
int valid_num(char *op, int min, int max, bool is_negative) {
    char *endptr;
    long num;

    /* Check if the input string is NULL */
    if (op == NULL) {
        return INVALID_NUM;
    }
    /* Convert the input string to a long integer */
    num = strtol(op, &endptr, 10);
    /* Handle negative numbers */
    if (is_negative == TRUE) {
        num = -num;
    }
    /* Remove leading spaces from the remaining portion of the string */
    if (endptr != NULL) {
        erase_spaces(endptr);
    }
    /* Check if there are any characters left after the number */
    if (*endptr != '\0') {
        return INVALID_NUM;
    }else if (num < min || num > max) {
        return INVALID_NUM;
    }else{
        return (int) num; /* Return the valid number as an integer */
    }
}

/* Validate a data operand and store it in the syntax_ast */
static void valid_data(char *ops_str, syntax_ast *sa) {
    int num = 0, cnt = 0, prev_comma = 1 , is_negative = 0; 
    char curr; 
    curr = get_content(ops_str);
    
    /* Check if the content of the string is 'E', indicating an error or empty string */
    if (curr == 'E') /*was empty string*/
    {
        sprintf(sa->syntax_error, "Error: missing data");
        return;
    }
    
    /* Check if the first character is a digit, minus sign, or plus sign */
    if (isdigit(curr) || curr == '-' || curr == '+') {
    /* Iterate through the input string */
    while (ops_str != NULL && *ops_str != '\0') {
        erase_spaces(ops_str);

        /* Check for end of string */
        if (*ops_str == '\0') {
            break;
        }

        /* Check for comma separator */
        if (*ops_str == ',') {
            if (prev_comma) {
                sprintf(sa->syntax_error, "Error: illegal comma");
                return;
            } else {
                prev_comma = 1; /* Mark that a comma was encountered */
                ops_str++;
            }
        }
        /* Process numeric data elements */
        else if (isdigit(*ops_str) || *ops_str == '-' || *ops_str == '+') {
            if (prev_comma == 0) {
                sprintf(sa->syntax_error, "Error: missing comma");
                return;
            }
            if (*ops_str == '+') {
                ops_str++; /* Move past the plus sign */
            }
            if (*ops_str == '-') {
                is_negative = 1; /* Mark the number as negative */
                ops_str++; /* Move past the minus sign */
            }
            /* Convert digits to form a number */
            while (*ops_str != '\0' && isdigit(*ops_str)) {
                num = num * 10 + (*ops_str - '0');
                ops_str++;
            }
            if (is_negative) {
                num = -num; /* Apply the negative sign if necessary */
                is_negative = 0;
            }
            /* Check if the number is within valid range */
            if (num > MIN_DATA && num < MAX_DATA) {
                sa->dir_inst.direct.direct_params.data_s.data_arr[cnt++] = num;
                prev_comma = 0; /* Reset the comma flag */
                num = 0; /* Reset temporary number storage */
            } else {
                sprintf(sa->syntax_error, "Error: illegal integer");
                return;
            }
        } else {
            sprintf(sa->syntax_error, "Error: illegal char in data operands");
            return;
        }
    }
}
else 
{
    /* Handle cases where the input does not start with a valid data element */
    if (curr == ',') {
        sprintf(sa->syntax_error, "Error: illegal comma");
        return;
    } else {
        sprintf(sa->syntax_error, "Error: illegal char");
        return;
    }
}
    sa->dir_inst.direct.direct_params.data_s.cnt_data = cnt; /* Store the count of data elements */
}

/* Validate a string operand and store it in the syntax_ast */
static void valid_string(char* ops_str, syntax_ast *sa) {
    char *endptr; /* Pointer to the end of the string */
    char curr; /* Current character */

    curr = get_content(ops_str);
    
    /* Check if the content of the string is 'E', indicating an error or empty string */
    if (curr == 'E') {
        sprintf(sa->syntax_error, "Error: missing string as data");
        return;
    }
    
    /* Check if the first character is a double quotation mark */
    if (curr != '"') {
        sprintf(sa->syntax_error, "Error: missing opening quotation mark");
        return;
    }
    
    ops_str++; /* Move to the next character after the opening quotation mark */
    
    /* Find the closing quotation mark */
    endptr = strrchr(ops_str, '"');
    
    /* Check if the closing quotation mark is missing */
    if (endptr == NULL) {
        sprintf(sa->syntax_error, "Error: missing closing quotation mark");
        return;
    }
    
    *endptr = '\0'; /* Replace the closing quotation mark with a null terminator */
    
    /* Check if the string contains unprintable characters */
    if (!is_printable(ops_str)) {
        sprintf(sa->syntax_error, "Error: string includes unprintable characters");
        return;
    }
    
    /* Copy the validated string to the syntax_ast */
    strcpy(sa->dir_inst.direct.direct_params.string, ops_str);
    
    endptr++; /* Move to the next character after the string's closing quotation mark */
    
    /* Check if there is any extraneous text after the string */
    while (*endptr != '\0') {
        if (!isspace(*endptr)) {
            sprintf(sa->syntax_error, "Error: extraneous text after string");
            return;
        }
        endptr++;
    }
    return;
}

/* Check if the given string contains only printable characters */
bool is_printable(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        /* Check if the ASCII value of the character is less than 32 or greater than 126*/
        if (str[i] < 32 || str[i] > 126) {
            return FALSE; /* Unprintable character found*/
        }
    }
    return TRUE; /* No unprintable characters found*/
}

/* Retrieve the first character from a string safely */
char get_content(char *s) {
    char content; /* Variable to store the content of the string */

    /* Check if the input string is not NULL and not an empty string */
    if (s != NULL && *s != '\0') {
        content = *s; /* Get the first character from the string */
        return content; /* Return the character content */
    } else {
        return 'E'; /* Return 'E' to indicate an error or empty string */
    }
}

/* Check if the given word exists in the instruction set */
static int word_exist_inst(char* str){
    int i;
    for (i = 0; i < NUM_OF_INSTRUCTS; i++) {
        if (strcmp(str, inst_ast_lex[i].inst_name) == 0) {
            return i;  /* Found the string at index i*/
        }
    }
    return -1;  /* String not found in the array*/
}

/* Check if the given word exists in the directive set */
static int word_exist_direct(char* str){
    int i;
    for (i = 0; i < NUM_OF_DIRECTS; i++) {
        if (strcmp(str, dir_ast_lex[i].direct_name) == 0) {
            return i;  /* Found the string at index i*/
        }
    }
    return -1;  /* String not found in the array*/
}

/* Trim leading and trailing whitespace from the given string */
char* trim_string(char* str) {
    int len;
    if (str == NULL) {
        return str;
    }
    
    /* Return the string if it is empty */
    if (*str == '\0') {
        return str;
    }
    erase_spaces(str);
    len = strlen(str);
    /* Trim trailing spaces */
    while (len > 0 && isspace(str[len - 1])) {
        str[--len] = '\0';
    }
    return str;
}
