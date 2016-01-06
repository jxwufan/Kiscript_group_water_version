//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#ifndef KISCRIPT_UTILS_H_H
#define KISCRIPT_UTILS_H_H

#include <glib.h>
#include <stdio.h>

typedef gpointer (*clone_func_t)(gpointer);
typedef void (*free_func_t)(gpointer);
typedef GString *(*to_string_func_t)(gpointer);

#define DECLARE_ID_GET_NAME_FUNC(id_name) \
    char *id_name##_get_name(id_name##_t id_name);

#define DEFINE_ID_GET_NAME_FUNC(id_name, name_array) \
    char *id_name##_get_name(id_name##_t id_name) { \
        return (name_array)[id_name]; \
    }

#define DEFINE_ID_TO_STRING_FUNC_FUNC(id_name) \
    GString *id_name##_to_string_func(gpointer id_name) { \
        return g_string_new(id_name##_get_name(*(id_name##_t*) id_name)); \
    }

#define DECLARE_FREE_FUNCS_WITH_TYPE(name, type_name) \
    void name##_free_no_nullify(type_name *name); \
 \
    void name##_free(type_name **name##_p); \

#define DEFINE_FREE_FUNCS_WITH_TYPE(name, type_name) \
    void name##_free_no_nullify(type_name *name) { \
        name##_final((name)); \
        g_free((name)); \
    } \
 \
    void name##_free(type_name **name##_p) { \
        name##_free_no_nullify(*name##_p); \
        *name##_p = NULL; \
    }

#define DECLARE_FREE_FUNCS(name) DECLARE_FREE_FUNCS_WITH_TYPE(name, name##_t)

#define DEFINE_FREE_FUNCS(name) DEFINE_FREE_FUNCS_WITH_TYPE(name, name##_t)

#define DEFINE_PRIMITIVE_NEW_FUNC_WITH_TYPE(name, type_name) \
    type_name *name##_new(type_name value) { \
        type_name *value_p = g_new_1(type_name); \
        *value_p = value; \
        return value_p; \
    }

#define DEFINE_PRIMITIVE_CLONE_FUNC_FUNC_WITH_TYPE(name, type_name) \
    gpointer name##_clone_func(gpointer value) { \
        return name##_new(*(type_name *) value); \
    }

#define DEFINE_PRIMITIVE_NEW_FUNC(name) \
        DEFINE_PRIMITIVE_NEW_FUNC_WITH_TYPE(name, name##_t)

#define DEFINE_PRIMITIVE_CLONE_FUNC_FUNC(name) \
        DEFINE_PRIMITIVE_CLONE_FUNC_FUNC_WITH_TYPE(name, name##_t)

#endif //KISCRIPT_UTILS_H_H
