//
// Created by fan wu on 1/5/16.
//

#include "interpreter.h"
#include "parser.h"

return_struct_t *evaluate_program(token_t *program_token, activation_record_t *AR_parent) {
    activation_record_t *AR = activation_record_new(AR_parent, NULL);
    for (guint i  = 0; i < program_token->children->len; ++i) {
//        return_struct_t *return_struct;
    }
    return NULL;
}

return_struct_t *return_struct_new() {
    return_struct_t *return_struct = GC_malloc(sizeof(return_struct_t));
    return return_struct;
}
