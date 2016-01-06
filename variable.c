//
// Created by fan wu on 1/5/16.
//

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "variable.h"

void variable_out_of_scope(variable_t *variable) {
    g_assert(variable != NULL);
    if (variable->variable_type == VARIABLE_FUNC) {
        g_hash_table_unref(variable->AR->AR_hash_table);
    }
}

variable_t *variable_new(variable_type_t variable_type, gpointer variable_data,
                              activation_record_t *AR) {
    variable_t *variable = (variable_t*) GC_malloc(sizeof(variable_t));
    variable->variable_type = variable_type;
    variable->variable_data = variable_data;
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
    return new_variable;
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

void variable_free(variable_t *variable) {
    if (variable->variable_data != NULL) {
        if (variable->variable_type != VARIABLE_OBJECT) {
            GC_FREE(variable->variable_data);
        } else {
            g_hash_table_unref((GHashTable*) variable->variable_data);
        }
    }
    if (variable->AR != NULL) {
        g_hash_table_unref(variable->AR->AR_hash_table);
    }
}

variable_t *variable_null_new() {
    return variable_new(VARIABLE_NULL, NULL, NULL);
}

variable_t *variable_bool_new(gpointer bool_data) {
    return variable_new(VARIABLE_BOOL, GC_malloc(sizeof(gboolean)), NULL);
}

variable_t *variable_numerical_new(gpointer numerical_data) {
    gdouble *tmp_double = (gdouble*) GC_malloc(sizeof(gdouble));
    *tmp_double = *((gdouble*) numerical_data);
    return variable_new(VARIABLE_NUMERICAL, tmp_double, NULL);
}

variable_t *variable_string_new(gpointer string_data) {
    guint str_len = strlen((gchar*) string_data);
    gchar *str = GC_malloc(str_len * sizeof(gchar));
    memcpy(str, string_data, str_len);
    return variable_new(VARIABLE_STRING, str, NULL);
}

variable_t *variable_object_new() {
    GHashTable *attribute_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) variable_on_destory);

    return variable_new(VARIABLE_OBJECT, attribute_table, NULL);
}

variable_t *variable_function_new(gpointer function_data, activation_record_t *AR) {
    return variable_new(VARIABLE_FUNC, function_data, AR);
}
