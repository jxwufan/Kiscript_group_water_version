//
// Created by fan wu on 1/4/16.
//

#include "activation_record.h"
#include "variable.h"

activation_record_t *activation_record_new(activation_record_t *dynamic_link, activation_record_t *static_link) {
    activation_record_t *new_AR;
    new_AR = GC_malloc(sizeof(activation_record_t));
    new_AR->dynamic_link = dynamic_link;
    new_AR->static_link = static_link;
    new_AR->AR_hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) variable_on_destory);

    return new_AR;
};

activation_record_t *activation_record_clone(activation_record_t *origin) {
    activation_record_t *new_AR;
    new_AR = GC_malloc(sizeof(activation_record_t));
    new_AR->dynamic_link = origin->dynamic_link;
    new_AR->static_link = origin->static_link;
    new_AR->AR_hash_table = origin->AR_hash_table;

    g_hash_table_ref(origin->AR_hash_table);

    return new_AR;
}

void activation_record_reach_end_of_scope(activation_record_t *AR) {
    g_hash_table_unref(AR->AR_hash_table);

    return;
}

gboolean activation_record_insert(activation_record_t *AR, gchar *key, gpointer value) {
    activation_record_t *current_AR = AR;
    if (((variable_t *)value)->variable_type == VARIABLE_FUNC || ((variable_t *)value)->variable_type == VARIABLE_OBJECT) {
        if (((variable_t *)value)->new_flag) {
            ((variable_t *)value)->new_flag = FALSE;
        } else {
            g_hash_table_ref(((variable_t *) value)->variable_data);
        }
    }

    while (current_AR != NULL) {
        if (g_hash_table_contains(current_AR->AR_hash_table, key) == TRUE) {
            return g_hash_table_insert(current_AR->AR_hash_table, key, value);
        } else {
            current_AR = current_AR->dynamic_link;
        }
    }

    if (current_AR == NULL) {
        activation_record_declare(AR->static_link, key);
        return activation_record_insert(AR->static_link, key, value);
    }

    return FALSE;
}

gboolean activation_record_declare(activation_record_t *AR, gchar *key) {
    return g_hash_table_insert(AR->AR_hash_table, key, NULL);
}

gpointer activation_record_lookup(activation_record_t *AR, gchar *key) {
    while (AR != NULL) {
        if (g_hash_table_contains(AR->AR_hash_table, key)) {
            return g_hash_table_lookup(AR->AR_hash_table, key);
        } else {
            AR = AR->dynamic_link;
        }
    }
    return NULL;
}
