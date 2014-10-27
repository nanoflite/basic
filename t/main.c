#include "test.h"

#include <math.h>
#include <parser.h>
#include <stdbool.h>
#include <stdio.h>

extern void test_dictionary(void **state);

static void test_BASIC(void **state)
{
  assert_true( true );
} 

int main(void) {
    const UnitTest tests[] = {
        unit_test(test_BASIC),
        unit_test(test_dictionary),
    };
    return run_tests(tests);
}
