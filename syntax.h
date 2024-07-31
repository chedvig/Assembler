#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#include "define.h"
#define erase_spaces(s)  while(*s!='\0'&& isspace(*s) ) s++;

/*** Assumptions ***/
/* There is always a space after directive word (.entry/.extern/.data/.string) and after instructive commands*/
/*.entry and .extern accept only one symbol as operand*/

/*different instruct options*/
enum instruct_opt{
    instruct_mov,
    instruct_cmp,
    instruct_add,
    instruct_sub,
    instruct_not,
    instruct_clr,
    instruct_lea,
    instruct_inc,
    instruct_dec,
    instruct_jmp,
    instruct_bne,
    instruct_red,
    instruct_prn,
    instruct_jsr,
    instruct_rts,
    instruct_stop
};

/*differnt direct options*/
enum direct_opt{
    opt_entry,
    opt_extern,
    opt_data,
    opt_string
};

/* different operand methods*/
enum op_method{
    zero_op=0,
    num_op=1,
    label_op=3,
    reg_num_op=5
};

/*handling data*/
typedef struct{
    int data_arr[80];
    int cnt_data;
}data;

/*ast which represents directions sentences*/
typedef struct{
    enum direct_opt dir_opt;
    union{
        char symbol[MAX_SYMBOL_LENGTH];
        char string[MAX_LINE_LENGTH];
        data data_s;
    }direct_params;
}direct_ast;

/*ast which represents instruction sentences*/
typedef struct{
    enum instruct_opt inst_opt;
    union{
        int num;
        char symbol[MAX_SYMBOL_LENGTH+1];
        int reg_num;
        }op_opt[2];
        enum op_method op_met[2];
}instruct_ast;

/*struct represents the ast of the assembly language*/
typedef struct{
    char syntax_error[100];
    char label_name[MAX_SYMBOL_LENGTH+1];
    enum{
        direct,
        instruct
    }opt_ast;
    union{
        instruct_ast instruct;
        direct_ast direct;
    }dir_inst;
}syntax_ast;

/*build abstract syntax tree from the given line*/
syntax_ast syntax_ast_parse (char *line);

#endif