//
// Created by fan wu on 1/5/16.
//

#include <stdio.h>
#include <math.h>
#include "variable.h"

void variable_out_of_scope(variable_t *variable) {
    g_assert(variable != NULL);
    if (variable->variable_type == VARIABLE_FUNC) {
        g_hash_table_unref(variable->AR->AR_hash_table);
    }
}

variable_t *variable_new(variable_type_t variable_type, gpointer *variable_data, GHashTable *attribute_hash_table,
                              activation_record_t *AR) {
    variable_t *variable = (variable_t*) GC_malloc(sizeof(variable_t));
    variable->variable_type = variable_type;
    variable->variable_data = variable_data;
    variable->attribute_hash_table = attribute_hash_table;
    variable->AR = AR;
    return variable;
}

variable_t *variable_clone(variable_t *variable) {
    variable_t *new_variable = (variable_t*) GC_malloc(sizeof(variable_t));
    new_variable->variable_type = variable->variable_type;
    new_variable->variable_data = variable->variable_data;
    if (variable->variable_type == VARIABLE_FUNC) {
        new_variable->AR = variable->AR;
        g_hash_table_ref(variable->AR->AR_hash_table);
    }
    return NULL;
}

void variable_on_destory(variable_t *variable) {
    if (variable->variable_type == VARIABLE_FUNC) {
        g_hash_table_unref(variable->AR->AR_hash_table);
    } else if (variable->variable_type == VARIABLE_OBJECT){
        // TODO: recollect attriube table memory
    }
}

gchar* variable_to_string(variable_t *variable) {
    gchar* str = GC_malloc(TO_STRING_LEN);
    if (variable->variable_type == VARIABLE_NUMERICAL) {
        gdouble tmp_double = *((gdouble*) variable->variable_data);
        if (fabs(tmp_double - (gint) tmp_double) < INTEGER_THRESHOLD) {
            sprintf(str, "%d", (gint) tmp_double);
        } else {
            sprintf(str, "%.5f", tmp_double);
        }
    }

    return str;
}