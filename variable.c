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

    if (variable->variable_type == VARIABLE_OBJECT) {
        g_hash_table_ref((GHashTable*) variable->variable_data);
    }

    if (variable->variable_type == VARIABLE_FUNC) {
        new_variable->AR = variable->AR;
        g_hash_table_ref(variable->AR->AR_hash_table);
    }
    return new_variable;
}

void variable_on_destory(variable_t *variable) {
//    g_assert(variable != NULL);
    if (variable == NULL)
        return;
    if (variable->variable_type == VARIABLE_FUNC) {
        g_hash_table_unref(variable->AR->AR_hash_table);
    } else if (variable->variable_type == VARIABLE_OBJECT){
        // TODO: recollect attriube table memory
        g_hash_table_unref((GHashTable*) variable->variable_data);
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
    } else if (variable->variable_type == VARIABLE_STRING) {
        strcpy(str, (gchar*) variable->variable_data);
    } else if (variable->variable_type == VARIABLE_BOOL) {
        gboolean bool_value = *((gboolean*) variable->variable_data);
        if (bool_value) {
            sprintf(str, "true");
        } else {
            sprintf(str, "false");
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
    gboolean  *tmp_bool = (gboolean*) GC_malloc(sizeof(gboolean));
    *tmp_bool = *((gboolean *) bool_data);
    return variable_new(VARIABLE_BOOL, tmp_bool, NULL);
}

variable_t *variable_numerical_new(gpointer numerical_data) {
    gdouble *tmp_double = (gdouble*) GC_malloc(sizeof(gdouble));
    *tmp_double = *((gdouble*) numerical_data);
    return variable_new(VARIABLE_NUMERICAL, tmp_double, NULL);
}

variable_t *variable_string_new(gpointer string_data) {
    guint str_mem_len = strlen((gchar*) string_data) + 1;
    gchar *str = GC_malloc((str_mem_len) * sizeof(gchar));
    strcpy(str, (gchar*) string_data);
    return variable_new(VARIABLE_STRING, str, NULL);
}

variable_t *variable_object_new() {
    GHashTable *attribute_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) variable_on_destory);

    return variable_new(VARIABLE_OBJECT, attribute_table, NULL);
}

variable_t *variable_function_new(gpointer function_data, activation_record_t *AR) {
    return variable_new(VARIABLE_FUNC, function_data, AR);
}
