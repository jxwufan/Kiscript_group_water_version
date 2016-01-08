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
    activation_record_t *AR = activation_record_new(AR_parent, NULL);
    AR->static_link = AR;
    for (guint i  = 0; i < program_token->children->len; ++i) {
        return_struct_t *return_struct;
        return_struct = evaluate_token(token_get_child(program_token, i), AR);
        if (return_struct->mid_variable != NULL) {
            printf("%s\n", variable_to_string(return_struct->mid_variable));
        }
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
           token->id == TOKEN_EXPRESSION_EQUALITY_EXPRESSION             ||
           token->id == TOKEN_EXPRESSION_CALL_EXPRESSION                 ||
           token->id == TOKEN_EXPRESSION_SHIFT_EXPRESSION                ||
           token->id == TOKEN_EXPRESSION_EQUALITY_EXPRESSION             ||
           token->id == TOKEN_EXPRESSION_UNARY_EXPRESSION                ||
           token->id == TOKEN_EXPRESSION_POSTFIX_EXPRESSION              ||
           token->id == TOKEN_EXPRESSION_RELATIONAL_EXPRESSION           ||
           token->id == TOKEN_EXPRESSION_LOGICAL_AND_EXPRESSION          ||
           token->id == TOKEN_EXPRESSION_LOGICAL_OR_EXPRESSION           ||
           token->id == TOKEN_EXPRESSION_BITWISE_AND_EXPRESSION          ||
           token->id == TOKEN_EXPRESSION_BITWISE_OR_EXPRESSION           ||
           token->id == TOKEN_EXPRESSION_BITWISE_XOR_EXPRESSION          ||
           token->id == TOKEN_EXPRESSION_OBJECT_LITERAL                  ||
           token->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR               ||
           token->id == TOKEN_EXPRESSION_ASSIGNMENT_EXPRESSION;
//           token->id == TOKEN_EXPRESSION_ARGUMENT_LIST;
}

gboolean is_statement(token_t *token) {
    return  token->id == TOKEN_STATEMENT_EXPRESSION_STATEMENT ||
            token->id == TOKEN_STATEMENT_VARIABLE_DECLARATION ||
            token->id == TOKEN_STATEMENT_VARIABLE_STATEMENT;
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

    return_struct->status = STAUS_THROW;
    return NULL;
}

gboolean is_function(token_t *token) {
    return token->id == TOKEN_FUNCTION_FUNCTION_DECLARATION;
    return FALSE;
}

return_struct_t *evaluate_statement(token_t *statement_token, activation_record_t *AR_Parent) {
//    return_struct_t = return_struct_new();

    if (statement_token->id == TOKEN_STATEMENT_EXPRESSION_STATEMENT) {
        return evaluate_token(token_get_child(statement_token, 0), AR_Parent);
    } else if (statement_token->id == TOKEN_STATEMENT_VARIABLE_STATEMENT) {
        return evaluate_token(token_get_child(statement_token, 0), AR_Parent);
    } else if (statement_token->id == TOKEN_STATEMENT_VARIABLE_DECLARATION) {
        g_assert(token_get_child(statement_token, 0)->id == TOKEN_LEXICAL_IDENTIFIER);
        gchar *identifier_str = identifier_get_value(token_get_child(statement_token, 0))->str;

        activation_record_declare(AR_Parent, identifier_str);
        if (statement_token->children->len > 1) {
            return_struct_t *return_struct;
            return_struct = evaluate_token(token_get_child(statement_token, 1), AR_Parent);
            if (return_struct->status == STAUS_NORMAL) {
                activation_record_insert(AR_Parent, identifier_str, return_struct->mid_variable);
                return return_struct;
            } else if (return_struct->status == STAUS_THROW) {
                // TODO: handel exception
            }
        }
    }

    return NULL;
}

return_struct_t *evaluate_expression(token_t *expression_token, activation_record_t *AR_Parent) {
    return_struct_t *return_struct = return_struct_new();
    if (expression_token->id == TOKEN_EXPRESSION_ASSIGNMENT_EXPRESSION) {
        gchar *identifier;
        GHashTable *storage_hash_table;
        resolve_assignment_identifier(token_get_child(expression_token, 0), AR_Parent, &identifier,
                                      &storage_hash_table);

        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_Parent);
        // TODO: check return status
        g_hash_table_insert(storage_hash_table, identifier, return_struct_rhs->mid_variable);

        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = return_struct_rhs->mid_variable;
        return return_struct;
    } else if (expression_token->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR) {
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);

        if (return_struct_lhs->status == STAUS_NORMAL) {
            g_assert(return_struct_lhs->mid_variable->variable_type == VARIABLE_OBJECT);

            return_struct->mid_variable = variable_object_lookup(return_struct_lhs->mid_variable, token_get_child(expression_token, 1));
            if (return_struct->mid_variable != NULL) {
                return_struct->status = STAUS_NORMAL;
                return return_struct;
            } else {
                // TODO: handel exception
                return_struct->status = STAUS_THROW;
                return return_struct;
            }
        } else if (return_struct_lhs->status == STAUS_THROW) {
            // TODO: handel exception
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_OBJECT_LITERAL) {
        variable_t *object_varialbe = variable_object_new();

        for (guint i = 0; i < expression_token->children->len; ++i) {
            token_t *attribute_token = token_get_child(expression_token, i);

            token_t *lhs = token_get_child(attribute_token, 0);
            return_struct_t *return_struct_rhs = evaluate_token(token_get_child(attribute_token, 1), AR_Parent);

            if (return_struct_rhs->status == STAUS_NORMAL) {
                variable_object_insert(object_varialbe, lhs, return_struct_rhs->mid_variable);
            } else {
                // TODO: handel exception
            }
        }

        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = object_varialbe;

        return return_struct;
    } else if (expression_token->id == TOKEN_EXPRESSION_ADDITIVE_EXPRESSION) {
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
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_PERCENT) {
                gint lhs_int = (int)(*lhs_value);
                gint rhs_int = (int)(*rhs_value);

                result = lhs_int % rhs_int;
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
    } else if (expression_token->id == TOKEN_EXPRESSION_EQUALITY_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_EQUALS) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                    || lhs->variable_type == VARIABLE_STRING
                    || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)==0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_DOES_NOT_EQUAL) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)==0) {
                    result = FALSE;
                } else {
                    result = TRUE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_STRICT_EQUALS) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL)
                && lhs->variable_type == rhs->variable_type) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)==0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_STRICT_DOES_NOT_EQUAL) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)==0 && lhs->variable_type == rhs->variable_type) {
                    result = FALSE;
                } else {
                    result = TRUE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_UNARY_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        // TODO: Is the child 1 always a identifier?
        g_assert(token_get_child(expression_token, 1)->id == TOKEN_LEXICAL_IDENTIFIER);

        return_struct_t *return_struct_operand = evaluate_token(token_get_child(expression_token, 1), AR_Parent);
        variable_t *operand = return_struct_operand->mid_variable;
        // TODO: check return status
        if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_INCREMENT) {
            if (operand->variable_type == VARIABLE_NUMERICAL) {
                gdouble *operand_value = operand->variable_data;
                gdouble result;
                result = (*operand_value) + 1;

                variable_t *result_variable = variable_numerical_new(&result);

                activation_record_insert(AR_Parent, identifier_get_value(token_get_child(expression_token, 1))->str, result_variable);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = result_variable;
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_DECREMENT) {
            if (operand->variable_type == VARIABLE_NUMERICAL) {
                gdouble *lhs_value = operand->variable_data;
                gdouble result;
                result = (*lhs_value) - 1;

                variable_t *result_variable = variable_numerical_new(&result);

                activation_record_insert(AR_Parent, identifier_get_value(token_get_child(expression_token, 1))->str, result_variable);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_PLUS) {
            if (operand->variable_type == VARIABLE_NUMERICAL) {
                gdouble *lhs_value = operand->variable_data;
                gdouble result;
                result = *lhs_value;

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_MINUS) {
            if (operand->variable_type == VARIABLE_NUMERICAL) {
                gdouble *lhs_value = operand->variable_data;
                gdouble result;
                result = 0 - (*lhs_value);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_TILDE) {
            if (operand->variable_type == VARIABLE_NUMERICAL) {
                gdouble *lhs_value = operand->variable_data;
                gdouble result;
                gint lhs_value_int = (int)(*lhs_value);
                result = ~lhs_value_int;

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_EXCLAMATION) {
            if (operand->variable_type == VARIABLE_NUMERICAL) {
                gdouble *lhs_value = operand->variable_data;
                gdouble result;
                result = !(*lhs_value);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (operand->variable_type == VARIABLE_BOOL) {
                gboolean  *lhs_value = operand->variable_data;
                gboolean result;
                result = !(*lhs_value);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_POSTFIX_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_operand = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *operand = return_struct_operand->mid_variable;
        // TODO: check return status
        if (operand->variable_type == VARIABLE_NUMERICAL) {
            gdouble *operand_value = operand->variable_data;
            gdouble result;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_INCREMENT) {
                result = *operand_value + 1;

                variable_t *result_variable = variable_numerical_new(&result);

                activation_record_insert(AR_Parent, identifier_get_value(token_get_child(expression_token, 0))->str, result_variable);

                result = result - 1;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_DECREMENT) {
                result = *operand_value - 1;

                variable_t *result_variable = variable_numerical_new(&result);

                activation_record_insert(AR_Parent, identifier_get_value(token_get_child(expression_token, 0))->str, result_variable);

                result = result + 1;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_RELATIONAL_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_ANGLE_BRACKET_LEFT) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)<0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_ANGLE_BRACKET_RIGHT) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)>0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_LESS_THAN_OR_EQUAL) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL)
                && lhs->variable_type == rhs->variable_type) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)<=0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_GREATER_THAN_OR_EQUAL) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)>=0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        }/* else if (*keyword_get_id(token_get_child(expression_token, 1)) == KEYWORD_INSTANCEOF) {

        }*/
    } else if (expression_token->id == TOKEN_EXPRESSION_LOGICAL_AND_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_BOOL)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_BOOL)) {
            gdouble  *lhs_value = lhs->variable_data;
            gdouble  *rhs_value = rhs->variable_data;
            gboolean result;
            result = (*lhs_value) && (*rhs_value);

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_bool_new(&result);
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_LOGICAL_OR_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_BOOL)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_BOOL)) {
            gdouble  *lhs_value = lhs->variable_data;
            gdouble  *rhs_value = rhs->variable_data;
            gboolean result;
            result = (*lhs_value) || (*rhs_value);

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_bool_new(&result);
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_BITWISE_AND_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL)
            && (rhs->variable_type == VARIABLE_NUMERICAL)) {
            gdouble *lhs_value = lhs->variable_data;
            gdouble *rhs_value = rhs->variable_data;
            gint lhs_int = (int)(*lhs_value);
            gint rhs_int = (int)(*rhs_value);
            gdouble result;
            result = lhs_int & rhs_int;

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_numerical_new(&result);
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_BITWISE_OR_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL)
            && (rhs->variable_type == VARIABLE_NUMERICAL)) {
            gdouble *lhs_value = lhs->variable_data;
            gdouble *rhs_value = rhs->variable_data;
            gint lhs_int = (int)(*lhs_value);
            gint rhs_int = (int)(*rhs_value);
            gdouble result;
            result = lhs_int | rhs_int;

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_numerical_new(&result);
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_BITWISE_XOR_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_Parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_Parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL)
            && (rhs->variable_type == VARIABLE_NUMERICAL)) {
            gdouble *lhs_value = lhs->variable_data;
            gdouble *rhs_value = rhs->variable_data;
            gint lhs_int = (int)(*lhs_value);
            gint rhs_int = (int)(*rhs_value);
            gdouble result;
            result = lhs_int ^ rhs_int;

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_numerical_new(&result);
            return return_struct;
        }
    }

    return_struct->status = STAUS_THROW;
    // TODO: handel exception
    return return_struct;
}

return_struct_t *resolve_assignment_identifier(token_t *lhs_token, activation_record_t *AR, gchar **identifier,
                                               GHashTable **storage_hash_table) {
    return_struct_t *return_struct = return_struct_new();

    if (lhs_token->id == TOKEN_LEXICAL_IDENTIFIER) {
        *identifier = identifier_get_value(lhs_token)->str;
        *storage_hash_table = AR->AR_hash_table;

        return_struct->status = STAUS_NORMAL;
        return return_struct;
    } else if (lhs_token->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR) {
        token_t *object_token = token_get_child(lhs_token, 0);
        return_struct_t *object_return_struct = evaluate_token(object_token, AR);
        if (object_return_struct->status == STAUS_NORMAL) {
            g_assert(object_return_struct->mid_variable->variable_type == VARIABLE_OBJECT);
            // TODO: get object hash table
        }
    }

    return_struct->status = STAUS_THROW;
    // TODO: handel exception
    return NULL;
}
