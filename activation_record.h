//
// Created by fan wu on 1/4/16.
//

#ifndef KISCRIPT_ACTIVE_RECORD_H
#define KISCRIPT_ACTIVE_RECORD_H

#include <glib.h>
#include <gc.h>

typedef struct _activation_record {
    struct _activation_record   *dynamic_link;
    struct _activation_record   *static_link;
    GHashTable                  *AR_hash_table;
} activation_record_t;


activation_record_t *activation_record_new(activation_record_t *dynamic_link, activation_record_t *static_link);
activation_record_t *activation_record_clone(activation_record_t *origin);

gboolean    activation_record_insert    (activation_record_t *AR, gchar *key, gpointer value);
gboolean    activation_record_declare   (activation_record_t *AR, gchar *key);
gpointer    activation_record_lookup    (activation_record_t *AR, gchar *key);

void activation_record_reach_end_of_scope(activation_record_t *AR);

#endif //KISCRIPT_ACTIVE_RECORD_H
