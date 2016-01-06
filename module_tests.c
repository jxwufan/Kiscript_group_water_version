//
// Created by fan wu on 1/6/16.
//

#include <stdio.h>
#include "module_tests.h"
#include "variable.h"

void test_to_string() {
    variable_t *test_variable = GC_malloc(sizeof(variable_t));
    test_variable->variable_type = VARIABLE_NUMERICAL;
    gdouble tmp = -10.1;
    test_variable->variable_data = &tmp;
    printf("%s\n", variable_to_string(test_variable));
}
