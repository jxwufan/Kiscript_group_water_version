//
// Created by fan wu on 1/4/16.
//

#include "activation_record.h"

activation_record_t *activation_record_new(activation_record_t *link) {
    activation_record_t *new_AR;
    new_AR = GC_malloc(sizeof(activation_record_t));
    new_AR->link = link;
    new_AR->AR_hash_table = g_hash_table_new(g_str_hash, g_str_equal);

    return new_AR;
};

activation_record_t *activation_record_clone(activation_record_t *origin) {
    activation_record_t *new_AR;
    new_AR = GC_malloc(sizeof(activation_record_t));
    new_AR->link = origin->link;
    new_AR->AR_hash_table = origin->AR_hash_table;

    g_hash_table_ref(origin->AR_hash_table);

    return NULL;
}

void activation_record_reach_end_of_scope(activation_record_t *AR) {
    g_hash_table_unref(AR->AR_hash_table);

    return;
}
