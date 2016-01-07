//
// Created by fan wu on 1/5/16.
//

#include "interpreter.h"
#include "parser.h"
#include "lexical_parser.h"
#include "variable.h"

return_struct_t *evaluate_token(token_t *token, activation_record_t *AR_parent) {
    if (is_lexical(token)) {
        return evaluate_lexicial(token, AR_parent);
    } else if (is_expression(token)) {
        return evaluate_expression(token, AR_parent);
    } else if (is_statement(token)) {
        return evaluate_statement(token, AR_parent);
    }
    return NULL;
}

return_struct_t *evaluate_program(token_t *program_token, activation_record_t *AR_parent) {
    activation_record_t *AR = activation_record_new(AR_parent);
    for (guint i  = 0; i < program_token->children->len; ++i) {
        return_struct_t *return_struct;
        return_struct = evaluate_token(token_get_child(program_token, i), AR);
        printf("%s\n", variable_to_string(return_struct->mid_variable));
    }
    return NULL;
}

return_struct_t *return_struct_new() {
    return_struct_t *return_struct = GC_malloc(sizeof(return_struct_t));
    return_struct->status = STAUS_UNDEFINED;
    return_struct->mid_variable = NULL;
    return_struct->end_variable = NULL;
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
    return token->id == TOKEN_EXPRESSION_ADDITIVE_EXPRESSION             ||
           token->id == TOKEN_EXPRESSION_MULTIPLICATIVE_EXPRESSION       ||
           token->id == TOKEN_EXPRESSION_EQUALITY_EXPRESSION                ||
           token->id == TOKEN_EXPRESSION_CALL_EXPRESSION;//        ||
//           token->id == TOKEN_EXPRESSION_ARGUMENT_LIST;
}

gboolean is_statement(token_t *token) {
    return token->id == TOKEN_STATEMENT_EXPRESSION_STATEMENT;
}

return_struct_t *evaluate_lexicial(token_t *lexical_token, activation_record_t *AR_Parent) {
    return_struct_t *return_struct = return_struct_new();

    if (lexical_token->id == TOKEN_LEXICAL_BOOLEAN_LITERAL) {
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = variable_bool_new(boolean_literal_get_value(lexical_token));

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_IDENTIFIER) {
        return_struct->mid_variable = activation_record_lookup(AR_Parent, identifier_get_value(lexical_token)->str);
        if (return_struct->mid_variable != NULL) {
            return_struct->status = STAUS_NORMAL;
        } else {
            return_struct->status = STAUS_THROW;
            // TODO: return exception
        }

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_NULL_LITERAL) {
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = variable_null_new();

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_NUMERIC_LITERAL) {
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = variable_numerical_new(numeric_literal_get_value(lexical_token));

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_STRING_LITERAL) {
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = variable_string_new(string_literal_get_value(lexical_token)->str);

        return return_struct;
    }
    return NULL;
}

gboolean is_funciton(token_t *token) {
    return token->id == TOKEN_FUNCTION_FUNCTION_DECLARATION;
    return FALSE;
}

return_struct_t *evaluate_statement(token_t *statement_token, activation_record_t *AR_Parent) {
//    return_struct_t = return_struct_new();

    if (statement_token->id == TOKEN_STATEMENT_EXPRESSION_STATEMENT) {
        return evaluate_token(token_get_child(statement_token, 0), AR_Parent);
    }

    return NULL;
}

return_struct_t *evaluate_expression(token_t *expression_token, activation_record_t *AR_Parent) {
    return_struct_t *return_struct = return_struct_new();

    if (expression_token->id == TOKEN_EXPRESSION_ADDITIVE_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if (lhs->variable_type == VARIABLE_NUMERICAL && rhs->variable_type == VARIABLE_NUMERICAL) {
            gdouble *lhs_value = lhs->variable_data;
            gdouble *rhs_value = rhs->variable_data;
            gdouble result;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_PLUS) {
                result = *lhs_value + *rhs_value;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_MINUS) {
                result = *lhs_value - *rhs_value;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_MULTIPLICATIVE_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if (lhs->variable_type == VARIABLE_NUMERICAL && rhs->variable_type == VARIABLE_NUMERICAL) {
            gdouble *lhs_value = lhs->variable_data;
            gdouble *rhs_value = rhs->variable_data;
            gdouble result;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_SLASH) {
                result = *lhs_value / *rhs_value;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_ASTERISK) {
                result = *lhs_value * *rhs_value;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_SHIFT_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if (lhs->variable_type == VARIABLE_NUMERICAL && rhs->variable_type == VARIABLE_NUMERICAL) {
            gdouble *lhs_value = lhs->variable_data;
            gdouble *rhs_value = rhs->variable_data;
            gint lhs_value_int = (int)(*lhs_value);
            gint rhs_value_int = (int)(*rhs_value);
            gdouble result;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_LEFT_SHIFT) {
                result = lhs_value_int << rhs_value_int;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_SIGNED_RIGHT_SHIFT) {
                result = lhs_value_int >> rhs_value_int;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_UNSIGNED_RIGHT_SHIFT) {
                result = lhs_value_int >> rhs_value_int;
                if (result<0) {
                    result = 2147483647 + result + 1;
                }
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        }
    }

    return NULL;
}
