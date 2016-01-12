//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <gc.h>

#include "lexical_parser.h"
#include "syntactic_parser.h"
#include "parser.h"
#include "interpreter.h"
#include "variable.h"

int main() {
    GC_INIT();

    FILE *fi = fopen("./add.js", "r");

    char *input = (char *) g_malloc(sizeof(char) * 5000);
    size_t input_length = 0;
    int data;
    while((data = fgetc(fi)) != EOF) {
        input[input_length++] = data;
    }
    input[input_length] = '\0';

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

//    evaluate_program(program_or_error, NULL);

    GString *program_string = token_to_string(program_or_error);
    printf("%s", program_string->str);
    g_string_free(program_string, TRUE);
    token_free(&program_or_error);

//    printf("\n");
//    printf("Hello, KiScript!\n");

    return EXIT_SUCCESS;
}
