//
// Created by fan wu on 1/5/16.
//

#include "interpreter.h"
#include "parser.h"

return_struct_t *evaluate_token(token_t *token, activation_record_t *AR_parent) {
    return NULL;
}

return_struct_t *evaluate_program(token_t *program_token, activation_record_t *AR_parent) {
    activation_record_t *AR = activation_record_new(AR_parent);
    for (guint i  = 0; i < program_token->children->len; ++i) {
        return_struct_t *return_struct = return_struct_new();
        return_struct = evaluate_token(token_get_child(program_token, i), AR);
    }
    return NULL;
}

return_struct_t *return_struct_new() {
    return_struct_t *return_struct = GC_malloc(sizeof(return_struct_t));
    return return_struct;
}

gboolean is_lexical(token_t *token) {
    return token->id == TOKEN_LEXICAL_BOOLEAN_LITERAL   ||
           token->id == TOKEN_LEXICAL_IDENTIFIER        ||
           token->id == TOKEN_LEXICAL_NULL_LITERAL      ||
           token->id == TOKEN_LEXICAL_NUMERIC_LITERAL   ||
           token->id == TOKEN_LEXICAL_STRING_LITERAL;
}

gboolean is_expression(token_t *token) {
    return token->id == TOKEN_EXPRESSION_ADDITIVE_EXPRESSION;
}
