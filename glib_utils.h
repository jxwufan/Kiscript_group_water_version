//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#ifndef KISCRIPT_GLIB_UTILS_H
#define KISCRIPT_GLIB_UTILS_H

#include <glib.h>

#define g_new_1(type_name) g_new(type_name, 1)

void g_free_no_nullify(gpointer pointer);

#define g_free(pointer) \
    G_STMT_START { \
        g_free_no_nullify(pointer); \
        pointer = NULL; \
    } G_STMT_END

GString *g_string_append_g_string(GString *string, GString *string2);

GString *g_string_clone(GString *string);

gboolean g_ptr_array_is_empty(GPtrArray *array);

gboolean g_ptr_array_in_range(GPtrArray *array, gsize position);

#endif //KISCRIPT_GLIB_UTILS_H
