//
// Created by fan wu on 1/5/16.
//

#ifndef KISCRIPT_VARIABLE_H
#define KISCRIPT_VARIABLE_H

#include <glib.h>
#include "activation_record.h"

typedef enum {
    VARIABLE_UNDEFINED,
    VARIABLE_NULL,
    VARIABLE_BOOL,
    VARIABLE_NUMBER,
    VARIABLE_STRING,
    VARIABLE_OBJECT,
    VARIABLE_EXCEPTION,
    VARIABLE_FUNC
} variable_type_t;

typedef struct {
    variable_type_t variable_type;
    gpointer        variable_data;
    GHashTable      *attribute_hash_table;
    activation_record_t *AR;
} variable_t;

void variable_out_of_scope(variable_t *variable);


#endif //KISCRIPT_VARIABLE_H
