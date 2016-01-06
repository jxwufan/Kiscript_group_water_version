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
    }
}

gchar* variable_to_string(variable_t *variable) {
    if (variable->variable_type == VARIABLE_NUMBER) {
        int neg=1;
        variable_t *tmp_variable = variable_clone(variable);
        gchar *expected_string = ".";
        double *tmp_number = (double*)tmp_variable->variable_data;
        if (tmp_number<0) {
            *tmp_number = - (*tmp_number);
            neg = -1;
        }
        int tmp_int = (int)(*tmp_number);
        double tmp_double = *tmp_number - tmp_int;
        if (tmp_int==0) {
            expected_string = g_strconcat("0", expected_string);
        }
        while (tmp_int>0) {
            expected_string = g_strconcat((gchar)(tmp_int%10+'0'), expected_string);
            tmp_int/=10;
        }
        for (int i=0; i<12; i++) {
            tmp_double *= 10;
            tmp_int = (int)(tmp_double);
            tmp_double -= tmp_int;
            expected_string = g_strconcat(expected_string, (gchar)(tmp_int+'0'));
        }
        if (neg == -1) {
            expected_string = g_strconcat("-", expected_string);
        }
        return expected_string;
    }
}