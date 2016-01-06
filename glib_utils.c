//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include "glib_utils.h"

GString *g_string_append_g_string(GString *string, GString *string2) {
    return g_string_append_len(string, string2->str, string2->len);
}

GString *g_string_clone(GString *string) {
    return g_string_new_len(string->str, string->len);
}

gboolean g_ptr_array_is_empty(GPtrArray *array) {
    return array->len == 0;
}

gboolean g_ptr_array_in_range(GPtrArray *array, gsize position) {
    return position < array->len;
}

#undef g_free

void g_free_no_nullify(gpointer pointer) {
    g_free(pointer);
}
