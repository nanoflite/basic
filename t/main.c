#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <math.h>
#include <parser.h>
#include <stdbool.h>
#include <stdio.h>

static void test_BASIC(void **state)
{
  assert_true( false );
} 

int main(void) {
    const UnitTest tests[] = {
        unit_test(test_BASIC),
    };
    return run_tests(tests);
}
