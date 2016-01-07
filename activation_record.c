//
// Created by fan wu on 1/4/16.
//

#include "activation_record.h"
#include "variable.h"

activation_record_t *activation_record_new(activation_record_t *link) {
    activation_record_t *new_AR;
    new_AR = GC_malloc(sizeof(activation_record_t));
    new_AR->link = link;
    new_AR->AR_hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) variable_on_destory);

    return new_AR;
};

activation_record_t *activation_record_clone(activation_record_t *origin) {
    activation_record_t *new_AR;
    new_AR = GC_malloc(sizeof(activation_record_t));
    new_AR->link = origin->link;
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

    while (current_AR != NULL) {
        if (g_hash_table_contains(current_AR->AR_hash_table, key)) {
            return g_hash_table_insert(current_AR->AR_hash_table, key, value);
        } else {
            current_AR = current_AR->link;
        }
    }
    if (current_AR == NULL) {
        activation_record_declare(AR, key);
        return activation_record_insert(AR, key, value);
    }

    return FALSE;
}

gboolean activation_record_declare(activation_record_t *AR, gchar *key) {
    return g_hash_table_insert(AR->AR_hash_table, key, NULL);
}

variable_t * activation_record_lookup(activation_record_t *AR, gchar *key) {
    while (AR != NULL) {
        if (g_hash_table_contains(AR->AR_hash_table, key)) {
            return g_hash_table_lookup(AR->AR_hash_table, key);
        } else {
            AR = AR->link;
        }
    }
    return NULL;
}
