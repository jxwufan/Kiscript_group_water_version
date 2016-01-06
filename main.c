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
        fprintf(stderr, "lexical_parse_normalize_input: error");
        g_free(input);
        return EXIT_FAILURE;
    }

    token_t *lexical_error = NULL;
    GPtrArray *token_list = lexical_parse(input, &lexical_error);
    g_free(input);
    if (lexical_error) {
        GString *error_string = token_to_string(lexical_error);
        fprintf(stderr, "lexical_parse: %s", error_string->str);
        g_string_free(error_string, TRUE);
        token_free(&lexical_error);
        return EXIT_FAILURE;
    }

    token_t *program_or_error = syntactic_parse(token_list);
    token_list_free(&token_list);
    if (error_is_error(program_or_error)) {
        GString *error_string = token_to_string(program_or_error);
        fprintf(stderr, "syntactic_parse: %s", error_string->str);
        g_string_free(error_string, TRUE);
        token_free(&program_or_error);
        return EXIT_FAILURE;
    }

    GString *program_string = token_to_string(program_or_error);
    printf("%s", program_string->str);
    g_string_free(program_string, TRUE);
    token_free(&program_or_error);

    printf("\n");
    printf("Hello, KiScript!\n");

    return EXIT_SUCCESS;
}
