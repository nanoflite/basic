#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <math.h>
#include <parser.h>

/* A test case that does nothing and succeeds. */
static void null_test_success(void **state) {
  (void) state; /* unused */

  evaluate_print("INT(205.5)");
  evaluate_print("INT(205.5+RND(1))");
  evaluate_print("INT(COS(RND(1))*10)");

  evaluate_print( "+1*(-1)" );
  evaluate_print( "100 * ABS(-2)" );
  for(int d=0; d<=90; d++) {
    float r = (d * M_PI) / 180.0;
    evaluate_print_func_param( "SIN", r );
    evaluate_print_func_param( "COS", r );
  }
  
  for(size_t i=0; i<100; i++) {
    evaluate_print_func_param( "RND", 0);
    evaluate_print_func_param( "RND", 1);
    evaluate_print_func_param( "RND", -1);
  }

  for(size_t i=0; i<100; i++) {
    evaluate_print("INT(205.5+RND(1))");
  }

}

int main(void) {
    const UnitTest tests[] = {
        unit_test(null_test_success),
    };
    return run_tests(tests);
}
