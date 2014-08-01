#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <math.h>
#include <parser.h>
#include <stdbool.h>
#include <stdio.h>

// relative comapre...
bool
compare_float_relative_epsilon(float f1, float f2, float max_relative_error)
{
  // printf("%f - %f\n", f1, f2);

  if ( f1 == f2 ) {
    return true;
  }

  float relative_error = fabs( (f1 - f2) / f2 );

  if (relative_error <= max_relative_error) {
        return true;
  }
  
  return false;
}

static void test_empty_expression(void **state)
{
  evaluate("");
  assert_true( NULL != evaluate_last_error() );
}

static void test_too_many_right_parentheses(void **state)
{
  evaluate("INT(1))))");
  assert_true( NULL != evaluate_last_error() );
}

static void test_INT(void **state)
{
  assert_true( 205.0 == evaluate("INT(205.5)") );
}

static void test_EXPRESSSION(void **state)
{
  assert_true( 10.0 == evaluate("INT(COS(RND(1))*10)") );
}

static void test_leading_plus(void **state)
{
  assert_true( -1.0 == evaluate( "+1*(-1)" ) );
}

static void test_ABS(void **state)
{
  assert_true( 2.0 == evaluate("ABS(-2)") );
}

static void test_SIN(void **state)
{
  char expression[256];
  for(int d=0; d<=90; d++) {
    float r = (d * M_PI) / 180.0;
    snprintf(expression, sizeof(expression), "SIN(%f)", r);
    assert_true( compare_float_relative_epsilon( sinf(r), evaluate( expression ), 0.0001 ) );
  }
} 

static void test_COS(void **state)
{
  char expression[256];
  for(int d=0; d<=89; d++) {
    float r = (d * M_PI) / 180.0;
    snprintf(expression, sizeof(expression), "COS(%f)", r);
    assert_true( compare_float_relative_epsilon( cosf(r), evaluate( expression ), 0.0001 ) );
  }
}

static void test_LOGIC(void **state)
{
  assert_true( evaluate("1AND0") == 0.0 );
  assert_true( evaluate("1OR0") == 1.0 );
  assert_true( evaluate("1OR2OR4") == 7.0 );
  assert_true( evaluate("255AND2") == 2.0 );
} 

int main(void) {
    const UnitTest tests[] = {
        unit_test(test_empty_expression),
        unit_test(test_too_many_right_parentheses),
        unit_test(test_EXPRESSSION),
        unit_test(test_leading_plus),
        unit_test(test_INT),
        unit_test(test_ABS),
        unit_test(test_SIN),
        unit_test(test_COS),
        unit_test(test_LOGIC),
    };
    return run_tests(tests);
}
