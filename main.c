//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexical_parser.h"
#include "syntactic_parser.h"
#include "parser.h"
#include "id_string.h"

void print_program_tree(token_t *current_token, gint deepth) {
    if (current_token == NULL)
        return;
    for (gint i = 0; i < deepth; ++i) {
        printf("\t");
    }
    printf("%s ", token_type_str[current_token->id]);
    if (current_token->id == TOKEN_LEXICAL_IDENTIFIER) {
        printf("%s\n", identifier_get_value(current_token)->str);
    } else if (current_token->id == TOKEN_LEXICAL_PUNCTUATOR) {
        printf("%s\n", punctuator_type_str[*punctuator_get_id(current_token)]);
    } else {
        printf("\n");
    }

    for (gint i = 0; i < current_token->children->len; ++i) {
        print_program_tree((token_t*) (current_token->children->pdata[i]), deepth + 1);
    }
}

int main() {

    char *input = NULL;
    size_t input_length = 0;
    getline(&input, &input_length, stdin);
    if (errno) {
        perror("getline");
        g_free(input);
        return EXIT_FAILURE;
    } else {
        char *input_malloc = input;
        input = g_strdup(input);
        free(input_malloc);
        input_malloc = NULL;
    }

    if (!lexical_parse_normalize_input(&input)) {
        g_error("lexical_parse_normalize_input: error");
        g_free(input);
        return EXIT_FAILURE;
    }

    token_t *error = NULL;
    GPtrArray *token_list = lexical_parse(input, &error);
    g_free(input);
    if (error) {
        g_error("lexical_parse: %s", error_get_id_name(error));
        token_free(&error);
        return EXIT_FAILURE;
    }

    token_t *program_or_error = syntactic_parse(token_list);
    token_list_free(&token_list);
    if (error_is_error(program_or_error)) {
        g_error("syntactic_parse: %s", error_get_id_name(program_or_error));
        token_free(&program_or_error);
        return EXIT_FAILURE;
    }

    print_program_tree(program_or_error, 0);

    token_free(&program_or_error);

    printf("Hello, KiScript!\n");
    return EXIT_SUCCESS;
}
