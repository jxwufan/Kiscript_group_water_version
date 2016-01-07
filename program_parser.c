//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include "program_parser.h"

#include "function_parser.h"
#include "syntactic_parser_utils.h"

/*
 * AST:
 * Program - SourceElement*
 *
 * GRAMMAR:
 * Program :
 *     SourceElement*
 */

token_t *program(GPtrArray *input, gsize *position_p) {

    token_t *program_token = token_new_no_data(TOKEN_PROGRAM_PROGRAM);

    while (g_ptr_array_in_range(input, *position_p)) {
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, source_element, program_token)
    }


    return program_token;
}
