//
// Created by fan wu on 1/5/16.
//

#include "variable.h"

void variable_out_of_scope(variable_t *variable) {
    g_assert(variable != NULL);
    if (variable->variable_type == VARIABLE_FUNC) {
        g_hash_table_unref(variable->AR->AR_hash_table);
    }
}

variable_t *variable_new(variable_type_t variable_type, gpointer *variable_data, GHashTable *attribute_hash_table,
                              activation_record_t *AR) {
    variable_t *variable = (variable_type_t*) GC_malloc(sizeof(variable_type_t));
    variable->variable_type = variable_type;
    variable->variable_data = variable_data;
    variable->attribute_hash_table = attribute_hash_table;
    variable->AR = AR;
    return variable;
}
