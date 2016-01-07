//
// Created by fan wu on 1/5/16.
//

#include "interpreter.h"
#include "parser.h"
#include "lexical_parser.h"

return_struct_t *evaluate_token(token_t *token, activation_record_t *AR_parent) {
    if (is_lexical(token)) {
        return evaluate_lexicial(token, AR_parent);
//    } else if (is_expression(token)) {
//        return evaluate_expression(token, AR_parent);
//    } else if (is_statement(token)) {
//        return evaluate_expression(token, AR_parent);
    }
    return NULL;
}

return_struct_t *evaluate_program(token_t *program_token, activation_record_t *AR_parent) {
//    activation_record_t *AR = activation_record_new(AR_parent);
    for (guint i  = 0; i < program_token->children->len; ++i) {
//        return_struct_t *return_struct = return_struct_new();
//        return_struct = evaluate_token(token_get_child(program_token, i), AR);
    }
    return NULL;
}

return_struct_t *return_struct_new() {
    return_struct_t *return_struct = GC_malloc(sizeof(return_struct_t));
    return_struct->staus       = STAUS_UNDEFINED;
    return_struct->mid_varible = NULL;
    return_struct->end_varible = NULL;
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
    return token->id == TOKEN_EXPRESSION_ADDITIVE_EXPRESSION    ||
           token->id == TOKEN_EXPRESSION_EQUALITY_EXPRESSION    ||
           token->id == TOKEN_EXPRESSION_CALL_EXPRESSION;//        ||
//           token->id == TOKEN_EXPRESSION_ARGUMENT_LIST;
}

gboolean is_statement(token_t *token) {
    return FALSE;
}

return_struct_t *evaluate_lexicial(token_t *lexical_token, activation_record_t *AR_Parent) {
    return_struct_t *return_struct = return_struct_new();

    if (lexical_token->id == TOKEN_LEXICAL_BOOLEAN_LITERAL) {
        return_struct->staus = STAUS_NORMAL;
        return_struct->mid_varible = variable_bool_new(boolean_literal_get_value(lexical_token));

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_IDENTIFIER) {
        return_struct->mid_varible = activation_record_lookup(AR_Parent, identifier_get_value(lexical_token)->str);
        if (return_struct->mid_varible != NULL) {
            return_struct->staus = STAUS_NORMAL;
        } else {
            return_struct->staus = STAUS_THROW;
        }

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_NULL_LITERAL) {
        return_struct->staus = STAUS_NORMAL;
        return_struct->mid_varible = variable_null_new();

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_NUMERIC_LITERAL) {
        return_struct->staus = STAUS_NORMAL;
        return_struct->mid_varible = variable_numerical_new(numeric_literal_get_value(lexical_token));

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_STRING_LITERAL) {
        return_struct->staus = STAUS_NORMAL;
        return_struct->mid_varible = variable_string_new(string_literal_get_value(lexical_token)->str);

        return return_struct;
    }
    return NULL;
}

gboolean is_funciton(token_t *token) {
//    if (token->id == TOKEN_FUNCTION_FUNCTION_DECLARATION)
    return FALSE;
}
