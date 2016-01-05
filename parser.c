//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include "parser.h"

static void token_init(token_t *token, token_id_t id, gpointer data,
                       clone_func_t data_clone_func,
                       free_func_t data_free_func, GPtrArray *children) {
    token->id = id;
    token->data = data;
    token->data_clone_func = data_clone_func;
    token->data_free_func = data_free_func;
    token->children = children;
}

static token_t *token_new_with_children(token_id_t id, gpointer data,
                                        clone_func_t data_clone_func,
                                        free_func_t data_free_func,
                                        GPtrArray *children) {
    token_t *token = g_new_1(token_t);
    token_init(token, id, data, data_clone_func, data_free_func, children);
    return token;
}

token_t *token_new(token_id_t id, gpointer data, clone_func_t data_clone_func,
                   free_func_t data_free_func) {
    return token_new_with_children(id, data, data_clone_func, data_free_func,
                                   token_list_new());
}

static gpointer token_data_gstring_clone_func(gpointer string) {
    return g_string_clone(string);
}

static void token_data_gstring_free_func(gpointer string) {
    g_string_free(string, TRUE);
}

token_t *token_new_gstring(token_id_t id, GString *string) {
    return token_new(id, string, token_data_gstring_clone_func,
                     token_data_gstring_free_func);
}

token_t *token_new_no_data(token_id_t id) {
    return token_new(id, NULL, NULL, NULL);
}

token_t *token_clone(token_t *token) {
    gpointer data = NULL;
    if (token->data) {
        if (token->data_clone_func) {
            data = token->data_clone_func(token->data);
        } else {
            g_error("data_clone_func is NULL");
        }
    }
    return token_new_with_children(token->id, data, token->data_clone_func,
                                   token->data_free_func,
                                   token_list_clone(token->children));
}

static void token_final(token_t *token) {
    if (token->data) {
        if (token->data_free_func) {
            token->data_free_func(token->data);
            token->data = NULL;
        } else {
            g_free(token->data);
        }
    }
    token_list_free(&token->children);
}

DEFINE_FREE_FUNCS(token)

void token_add_child(token_t *token, token_t *child) {
    g_ptr_array_add(token->children, child);
}

gboolean token_has_child(token_t *token, gsize position) {
    return position < token->children->len;
}

token_t *token_get_child(token_t *token, gsize position) {
    g_assert(token_has_child(token, position));
    return g_ptr_array_index(token->children, position);
}

token_t *token_get_last_child(token_t *token) {
    g_assert(token->children->len > 0);
    return token_get_child(token, token->children->len - 1);
}


static void token_free_func(gpointer token) {
    if (token) {
        token_free_no_nullify(token);
    }
}

GPtrArray *token_list_new() {
    return g_ptr_array_new_with_free_func(token_free_func);
}

GPtrArray *token_list_clone(GPtrArray *token_list) {
    GPtrArray *token_list_clone = g_ptr_array_new_full(token_list->len,
                                                       token_free_func);
    for (gsize i = 0; i < token_list->len; ++i) {
        g_ptr_array_add(token_list_clone,
                        token_clone(g_ptr_array_index(token_list, i)));
    }
    return token_list_clone;
}

void token_list_free_no_nullify(GPtrArray *token_list) {
    g_ptr_array_free(token_list, TRUE);
}

void token_list_free(GPtrArray **token_list_p) {
    token_list_free_no_nullify(*token_list_p);
    *token_list_p = NULL;
}


static void error_info_init_lexical(error_info_t *error_info, error_id_t id,
                                    gchar *position) {
    error_info->id = id;
    error_info->is_lexical = TRUE;
    error_info->position.lexical = position;
}

static void error_info_init_syntactic(error_info_t *error_info, error_id_t id,
                                      gsize position) {
    error_info->id = id;
    error_info->is_lexical = FALSE;
    error_info->position.syntactic = position;
}

error_info_t *error_info_new_lexical(error_id_t id, gchar *position) {
    error_info_t *error_info = g_new_1(error_info_t);
    error_info_init_lexical(error_info, id, position);
    return error_info;
}

error_info_t *error_info_new_syntactic(error_id_t id, gsize position) {
    error_info_t *error_info = g_new_1(error_info_t);
    error_info_init_syntactic(error_info, id, position);
    return error_info;
}

static void error_info_final(error_info_t *error_info) {
    if (error_info->is_lexical) {
        error_info->position.lexical = NULL;
    }
}

DEFINE_FREE_FUNCS(error_info)


static void error_info_free_func(gpointer error_info) {
    error_info_free_no_nullify(error_info);
}

static token_t *error_new(error_info_t *info) {
    return token_new(TOKEN_ERROR, info, NULL, error_info_free_func);
}

token_t *error_new_lexical(error_id_t id, gchar *position) {
    return error_new(error_info_new_lexical(id, position));
}

token_t *error_new_syntactic(error_id_t id, gsize position) {
    return error_new(error_info_new_syntactic(id, position));
}

DEFINE_TOKEN_IS_TOKEN_FUNC(error, TOKEN_ERROR)

DEFINE_TOKEN_GET_ID_FUNC(error)

DEFINE_TOKEN_IS_TOKEN_WITH_ID_FUNC(error)

static char *ERROR_ID_NAMES[] = {
    "ERROR_LEXICAL_LEXICAL_TOKEN",
    "ERROR_LEXICAL_WHITE_SPACE",
    "ERROR_LEXICAL_LINE_TERMINATOR",
    "ERROR_LEXICAL_MULTI_LINE_COMMENT",
    "ERROR_LEXICAL_SINGLE_LINE_COMMENT",
    "ERROR_LEXICAL_KEYWORD",
    "ERROR_LEXICAL_FUTURE_RESERVED_WORD",
    "ERROR_LEXICAL_IDENTIFIER",
    "ERROR_LEXICAL_PUNCTUATOR",
    "ERROR_LEXICAL_NULL_LITERAL",
    "ERROR_LEXICAL_BOOLEAN_LITERAL",
    "ERROR_LEXICAL_NUMERIC_LITERAL_DECIMAL",
    "ERROR_LEXICAL_NUMERIC_LITERAL_BINARY",
    "ERROR_LEXICAL_NUMERIC_LITERAL_OCTAL",
    "ERROR_LEXICAL_NUMERIC_LITERAL_HEX",
    "ERROR_LEXICAL_NUMERIC_LITERAL_FOLLOWED_BY_IDENTIFIER",
    "ERROR_LEXICAL_STRING_LITERAL",
    "ERROR_EXPRESSION_PRIMARY_EXPRESSION_PARENTHESIS_LEFT",
    "ERROR_EXPRESSION_PRIMARY_EXPRESSION",
    "ERROR_EXPRESSION_ARRAY_LITERAL_SQUARE_BRACKET_LEFT",
    "ERROR_EXPRESSION_ARRAY_LITERAL_COMMA_OR_SQUARE_BRACKET_RIGHT",
    "ERROR_EXPRESSION_OBJECT_LITERAL_CURLY_BRACE_LEFT",
    "ERROR_EXPRESSION_OBJECT_LITERAL_COMMA_OR_CURLY_BRACE_RIGHT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_PLAIN_COLON",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_GET",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_PARENTHESIS_LEFT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_PARENTHESIS_RIGHT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_CURLY_BRACE_LEFT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_CURLY_BRACE_RIGHT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_SET",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_PARENTHESIS_LEFT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_PARENTHESIS_RIGHT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_IDENTIFIER",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_CURLY_BRACE_LEFT",
    "ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_CURLY_BRACE_RIGHT",
    "ERROR_EXPRESSION_PROPERTY_NAME",
    "ERROR_EXPRESSION_PROPERTY_NAME_SQUARE_BRACKET_RIGHT",
    "ERROR_EXPRESSION_CALLABLE_EXPRESSION_CALLABLE_EXPRESSION",
    "ERROR_EXPRESSION_PROPERTY_ACCESSOR_IDENTIFIER",
    "ERROR_EXPRESSION_PROPERTY_ACCESSOR_SQUARE_BRACKET_RIGHT",
    "ERROR_EXPRESSION_ARGUMENT_LIST_PARENTHESIS_LEFT",
    "ERROR_EXPRESSION_ARGUMENT_LIST_COMMA_OR_PARENTHESIS_RIGHT",
    "ERROR_EXPRESSION_NEW_EXPRESSION_NEW",
    "ERROR_EXPRESSION_NEW_EXPRESSION_CALL_EXPRESSION",
    "ERROR_EXPRESSION_CONDITIONAL_EXPRESSION_COLON",
    "ERROR_STATEMENT_BLOCK_CURLY_BRACE_LEFT",
    "ERROR_STATEMENT_VARIABLE_STATEMENT_VAR",
    "ERROR_STATEMENT_VARIABLE_STATEMENT_SEMICOLON",
    "ERROR_STATEMENT_VARIABLE_DECLARATION_IDENTIFIER",
    "ERROR_STATEMENT_INITIALIZER_EQUALS_SIGN",
    "ERROR_STATEMENT_EMPTY_STATEMENT_SEMICOLON",
    "ERROR_STATEMENT_IF_STATEMENT_IF",
    "ERROR_STATEMENT_IF_STATEMENT_PARENTHESIS_LEFT",
    "ERROR_STATEMENT_IF_STATEMENT_PARENTHESIS_RIGHT",
    "ERROR_STATEMENT_DO_WHILE_STATEMENT_DO",
    "ERROR_STATEMENT_DO_WHILE_STATEMENT_WHILE",
    "ERROR_STATEMENT_DO_WHILE_STATEMENT_PARENTHESIS_LEFT",
    "ERROR_STATEMENT_DO_WHILE_STATEMENT_PARENTHESIS_RIGHT",
    "ERROR_STATEMENT_WHILE_STATEMENT_WHILE",
    "ERROR_STATEMENT_WHILE_STATEMENT_PARENTHESIS_LEFT",
    "ERROR_STATEMENT_WHILE_STATEMENT_PARENTHESIS_RIGHT",
    "ERROR_STATEMENT_FOR_STATEMENT_FOR",
    "ERROR_STATEMENT_FOR_STATEMENT_PARENTHESIS_LEFT",
    "ERROR_STATEMENT_FOR_EACH_STATEMENT_PARENTHESIS_RIGHT",
    "ERROR_STATEMENT_FOR_STATEMENT_FIRST_SEMICOLON",
    "ERROR_STATEMENT_FOR_STATEMENT_SECOND_SEMICOLON",
    "ERROR_STATEMENT_FOR_STATEMENT_PARENTHESIS_RIGHT",
    "ERROR_STATEMENT_CONTINUE_STATEMENT_CONTINUE",
    "ERROR_STATEMENT_CONTINUE_STATEMENT_IDENTIFIER",
    "ERROR_STATEMENT_CONTINUE_STATEMENT_SEMICOLON",
    "ERROR_STATEMENT_BREAK_STATEMENT_BREAK",
    "ERROR_STATEMENT_BREAK_STATEMENT_IDENTIFIER",
    "ERROR_STATEMENT_BREAK_STATEMENT_SEMICOLON",
    "ERROR_STATEMENT_RETURN_STATEMENT_RETURN",
    "ERROR_STATEMENT_RETURN_STATEMENT_IDENTIFIER",
    "ERROR_STATEMENT_RETURN_STATEMENT_SEMICOLON",
    "ERROR_STATEMENT_LABELED_STATEMENT_IDENTIFIER",
    "ERROR_STATEMENT_LABELED_STATEMENT_COLON",
    "ERROR_STATEMENT_SWITCH_STATEMENT_SWITCH",
    "ERROR_STATEMENT_SWITCH_STATEMENT_PARENTHESIS_LEFT",
    "ERROR_STATEMENT_SWITCH_STATEMENT_PARENTHESIS_RIGHT",
    "ERROR_STATEMENT_SWITCH_STATEMENT_CURLY_BRACE_LEFT",
    "ERROR_STATEMENT_SWITCH_STATEMENT_CURLY_BRACE_RIGHT",
    "ERROR_STATEMENT_CASE_CLAUSE_CASE",
    "ERROR_STATEMENT_CASE_CLAUSE_COLON",
    "ERROR_STATEMENT_DEFAULT_CLAUSE_CASE",
    "ERROR_STATEMENT_DEFAULT_CLAUSE_COLON",
    "ERROR_STATEMENT_THROW_STATEMENT_THROW",
    "ERROR_STATEMENT_THROW_STATEMENT_IDENTIFIER",
    "ERROR_STATEMENT_THROW_STATEMENT_SEMICOLON",
    "ERROR_STATEMENT_TRY_STATEMENT_TRY",
    "ERROR_STATEMENT_TRY_STATEMENT_CATCH_PARENTHESIS_LEFT",
    "ERROR_STATEMENT_TRY_STATEMENT_CATCH_IDENTIFIER",
    "ERROR_STATEMENT_TRY_STATEMENT_CATCH_PARENTHESIS_RIGHT",
    "ERROR_STATEMENT_TRY_STATEMENT_CATCH_OR_FINALLY",
    "ERROR_STATEMENT_DEBUGGER_STATEMENT_DEBUGGER",
    "ERROR_STATEMENT_DEBUGGER_STATEMENT_SEMICOLON",
"ERROR_STATEMENT_EXPRESSION_STATEMENT_STARTS_WITH_CURLY_BRACE_LEFT_OR_FUNCTION",
    "ERROR_STATEMENT_EXPRESSION_STATEMENT_SEMICOLON",
    "ERROR_STATEMENT_STATEMENT_STATEMENT",
    "ERROR_FUNCTION_FUNCTION_DECLARATION_FUNCTION",
    "ERROR_FUNCTION_FUNCTION_DECLARATION_IDENTIFIER",
    "ERROR_FUNCTION_FUNCTION_EXPRESSION_FUNCTION",
    "ERROR_FUNCTION_FUNCTION_EXPRESSION_IDENTIFIER",
    "ERROR_FUNCTION_FORMAL_PARAMETER_LIST_PARENTHESIS_LEFT",
    "ERROR_FUNCTION_FORMAL_PARAMETER_LIST_IDENTIFIER",
    "ERROR_FUNCTION_FORMAL_PARAMETER_LIST_PARENTHESIS_RIGHT",
    "ERROR_FUNCTION_FUNCTION_BODY_CURLY_BRACE_LEFT",
    "ERROR_FUNCTION_FUNCTION_BODY_CURLY_BRACE_RIGHT",
    "ERROR_FUNCTION_SOURCE_ELEMENT"
};

char *error_get_id_name(token_t *error) {
    g_assert(error_is_error(error));
    return ERROR_ID_NAMES[*error_get_id(error)];
}
