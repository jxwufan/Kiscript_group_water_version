//
// Created by fan wu on 1/5/16.
//

#ifndef KISCRIPT_INTERPRETER_H
#define KISCRIPT_INTERPRETER_H

#include "variable.h"
#include "parser.h"

typedef enum {
    EXCEPTION,
    CONTINUE,
    BREAK
} return_staus_t;

typedef struct {
    return_staus_t  staus;
    variable_t      *mid_varible;
    variable_t      *end_varible;
} return_struct_t;

return_struct_t *return_struct_new();

return_struct_t *evaluate_token     (token_t *token,            activation_record_t *AR_parent);
return_struct_t *evaluate_lexcial   (token_t *lexical_token,    activation_record_t *AR_Parent);
return_struct_t *evaluate_expression(token_t *expression_token, activation_record_t *AR_Parent);
return_struct_t *evaluate_statement (token_t *statement_token,  activation_record_t *AR_Parent);
return_struct_t *evaluate_program   (token_t *program_token,    activation_record_t *AR_parent);

#endif //KISCRIPT_INTERPRETER_H
