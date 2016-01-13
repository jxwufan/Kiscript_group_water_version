//
// Created by fan wu on 1/5/16.
//

#ifndef KISCRIPT_VARIABLE_H
#define KISCRIPT_VARIABLE_H

#include <glib.h>
#include "activation_record.h"
#include "parser.h"

#define TO_STRING_LEN 255
#define INTEGER_THRESHOLD 0.001
#define ZERO_THRESHOLD 0.00000001

typedef enum {
    VARIABLE_UNDEFINED,
    VARIABLE_NULL,
    VARIABLE_BOOL,
    VARIABLE_NUMERICAL,
    VARIABLE_STRING,
    VARIABLE_OBJECT,
    VARIABLE_EXCEPTION,
    VARIABLE_FUNC
} variable_type_t;

typedef struct {
    variable_type_t         variable_type;
    gpointer                variable_data;
    token_t                 *function_token;
    activation_record_t     *AR;
    gboolean                new_flag;
} variable_t;

variable_t  *variable_new           (variable_type_t variable_type, gpointer variable_data, activation_record_t *AR);
variable_t  *variable_null_new      ();
variable_t  *variable_undefined_new ();
variable_t  *variable_bool_new      (gpointer bool_data);
variable_t  *variable_numerical_new (gpointer numerical_data);
variable_t  *variable_string_new    (gpointer string_data);
variable_t  *variable_object_new    ();
variable_t  *variable_exception_new (gpointer exception_data); // TODO: exception
variable_t  *variable_function_new  (gpointer function_data, activation_record_t *AR);
variable_t  *variable_clone         (variable_t *variable);

void        variable_on_destory     (variable_t *variable);
void        variable_free           (variable_t *variable);

gchar*      variable_to_string      (variable_t *variable);
gdouble     variable_to_numerical   (variable_t *variable);
gboolean    variable_to_bool        (variable_t *variable);

gboolean    variable_object_insert  (variable_t *object_variable, token_t *key, variable_t *value);
variable_t  *variable_object_lookup (variable_t *object_variable, token_t *key, activation_record_t *AR_parent);

variable_t  *prototype_chain_lookup (variable_t *object_variable, gchar *attribute_identifier);


void variable_out_of_scope(variable_t *variable);


#endif //KISCRIPT_VARIABLE_H
