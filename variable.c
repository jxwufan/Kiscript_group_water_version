//
// Created by fan wu on 1/5/16.
//

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "variable.h"
#include "parser.h"
#include "interpreter.h"
#include "lexical_parser.h"

// TODO: This function may meaningless
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
    variable->new_flag = TRUE;
    return variable;
}

variable_t *variable_clone(variable_t *variable) {
    variable_t *new_variable = (variable_t*) GC_malloc(sizeof(variable_t));
    new_variable->variable_type = variable->variable_type;
    new_variable->variable_data = variable->variable_data;

    if (variable->variable_type == VARIABLE_OBJECT || variable->variable_type == VARIABLE_FUNC) {
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
        g_hash_table_unref((GHashTable*) variable->variable_data);
    } else if (variable->variable_type == VARIABLE_OBJECT){
        // TODO: collect attribute table memory
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
    } else if (variable->variable_type == VARIABLE_OBJECT) {
        // TODO: implement with full description of object
        sprintf(str, "Object");
    } else if (variable->variable_type == VARIABLE_FUNC) {
        // TODO: implement with full description of function
        sprintf(str, "Func");
    }

    return str;
}

gdouble variable_to_numerical(variable_t *variable) {
    gdouble return_value = NAN;
    if (variable->variable_type == VARIABLE_STRING) {
        gchar* str = (gchar*) variable->variable_data;
        sscanf(str, "%lf", &return_value);

        gboolean is_legal_num = TRUE;
        gint cnt_dot = 0;

        for (gint i=0; i<strlen(str); i++) {
            if (str[i]>='0' && str[i]<='9') {
                continue;
            } else if (str[i]=='.') {
                cnt_dot++;
                if (cnt_dot>1) {
                    is_legal_num = FALSE;
                }
            } else if (str[i]=='-') {
                if (i>0) {
                    is_legal_num = FALSE;
                }
            } else {
                is_legal_num = FALSE;
            }
        }

        if (!is_legal_num) {
            return_value = NAN;
        }
    } else if (variable->variable_type == VARIABLE_NUMERICAL) {
        return_value = *((gdouble*)variable->variable_data);
    } else if (variable->variable_type == VARIABLE_BOOL) {
        gboolean tmp_bool = *((gboolean*)variable->variable_data);
        if (tmp_bool) {
            return_value = 1;
        } else {
            return_value = 0;
        }
    }

    return return_value;
}

void variable_free(variable_t *variable) {
    if (variable->variable_data != NULL) {
        if (variable->variable_type != VARIABLE_OBJECT && variable->variable_type != VARIABLE_FUNC) {
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
    GHashTable *function_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) variable_on_destory);

    variable_t *prototype_variable = variable_object_new();

    g_hash_table_insert(function_table, "", function_data);
    g_hash_table_insert(function_table, "prototype", prototype_variable);
    g_hash_table_insert(function_table, "__proto__", NULL);

    g_hash_table_ref(AR->AR_hash_table);

    variable_t *function_variable = variable_new(VARIABLE_FUNC, function_table, AR);

    g_hash_table_insert((GHashTable*) prototype_variable->variable_data, "constructor", function_variable);
    g_hash_table_insert((GHashTable*) prototype_variable->variable_data, "__proto__", NULL);

    g_hash_table_ref(prototype_variable->variable_data);
    g_hash_table_ref(function_table);

    return function_variable;
}

gboolean variable_object_insert(variable_t *object_variable, token_t *key, variable_t *value) {
    g_assert(object_variable->variable_type == VARIABLE_OBJECT || object_variable->variable_type == VARIABLE_FUNC);
    if (value->variable_type == VARIABLE_OBJECT || value->variable_type == VARIABLE_FUNC) {
        if (value->new_flag) {
            value->new_flag = FALSE;
        } else {
            g_hash_table_ref(value->variable_data);
        }
    }

    if ( key->id != TOKEN_LEXICAL_IDENTIFIER) {
        return_struct_t *return_struct_variable_key = evaluate_token(key, NULL);

        if (return_struct_variable_key->status == STAUS_NORMAL) {
            return g_hash_table_insert((GHashTable *) object_variable->variable_data, variable_to_string(return_struct_variable_key->mid_variable),
                                       value);
        } else  {
            return FALSE;
        }
    } else {
        return g_hash_table_insert((GHashTable *) object_variable->variable_data, identifier_get_value(key)->str, value);
    }

    return FALSE;
}

variable_t *variable_object_lookup(variable_t *object_variable, token_t *key) {
    g_assert(object_variable->variable_type == VARIABLE_OBJECT || object_variable->variable_type == VARIABLE_FUNC);

    if (key->id != TOKEN_LEXICAL_IDENTIFIER) {
        return_struct_t *return_struct_variable_key = evaluate_token(key, NULL);

        if (return_struct_variable_key->status == STAUS_NORMAL) {
            // TODO: to string may cause  exception
            return prototype_chain_lookup(object_variable, variable_to_string(return_struct_variable_key->mid_variable));
        } else {
            return NULL;
        }
    } else {
        return prototype_chain_lookup(object_variable, identifier_get_value(key)->str);
    }

    return NULL;
}

variable_t *prototype_chain_lookup(variable_t *object_variable, gchar *attribute_identifier) {
    variable_t *return_variable;

    while (object_variable != NULL) {
        return_variable = g_hash_table_lookup((GHashTable*) object_variable->variable_data, attribute_identifier);
        if (return_variable != NULL)
            return return_variable;
        object_variable = g_hash_table_lookup((GHashTable*) object_variable->variable_data, "__proto__");
    }
    return NULL;
}
