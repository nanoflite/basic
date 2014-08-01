#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "tokenizer.h"

/*
  expression = ["+"|"-"] term {("+"|"-"|"OR") term} .

  term = factor {( "*" | "/" | "AND" ) factor} .

  factor = 
    func "(" expression ")" 
    | number
    | "(" expression ")" .

  func =
    ABS
    | AND
    | ATN
    | COS
    | EXP
    | INT
    | LOG
    | NOT
    | OR
    | RND
    | SGN
    | SIN
    | SQR
    | TAN
*/


static float
_abs(float n)
{
  return abs(n);
}

static float
_rnd(float n)
{
  if (n > 0) {
    int random = rand();
    float randomf = (random * 1.0) / RAND_MAX;
    return randomf; 
  }

  if (n < 0) {
    srand(n);
    return _rnd(1);
  }

  time_t now;
  struct tm *tm;
  now = time(NULL);    
  tm = localtime(&now);
  float randomf = (tm->tm_sec * 1.0) / 60;
  return randomf;
}

static float
_int(float n)
{
  int i = (int) n;  
  return 1.0 * i;
}

static float
_sqr(float n)
{
  return (float) sqrt( (double) n );
}

static float
_sgn(float n)
{
  if (n < 0) {
    return -1.0;
  } else if (n > 0) {
    return 1.0;
  } else {
    return 0.0;
  }
}

static float
_or(float a, float b)
{
  return (float) ( (int) a | (int) b );
}

static float
_and(float a, float b)
{
  return (float) ( (int) a & (int) b );
}

static float
_not(float number)
{
  return (float) ( ~ (int) number );
}

typedef float (*function)(float number);

typedef struct
{
  token _token;
  function _function;
} token_to_function;

token_to_function token_to_functions[] = 
{
  { T_FUNC_ABS, _abs },
  { T_FUNC_SIN, sinf },
  { T_FUNC_COS, cosf },
  { T_FUNC_RND, _rnd },
  { T_FUNC_INT, _int },
  { T_FUNC_TAN, tanf },
  { T_FUNC_SQR, _sqr },
  { T_FUNC_SGN, _sgn },
  { T_FUNC_LOG, logf },
  { T_FUNC_EXP, expf },
  { T_FUNC_ATN, atanf },
  { T_FUNC_NOT, _not },
  { T_EOF, NULL }
};

token sym;
const char *last_error;

static float expression(void);

static void
get_sym(void)
{
  sym = tokenizer_get_next_token();
  // printf("t: %d\n", sym);
}

static void
error(const char *error_msg)
{
  last_error = error_msg;
}

static bool
accept(token t)
{
  if (t == sym) {
    get_sym();
    return true;
  }
  return false;
}

static bool
expect(token t)
{
  if (accept(t)) {
    return true;
  }
  error("Expect: unexpected symbol");
  return false;
}

static bool
is_function_token(token sym)
{
  for(size_t i=0; token_to_functions[i]._token != T_EOF;i++) {
    if (sym == token_to_functions[i]._token) {
      return true;
    }
  }

  return false;
}

static function
get_function(token sym)
{
  token_to_function ttf;
  for(size_t i = 0;; i++) {
    ttf = token_to_functions[i];
    if (ttf._token == T_EOF) {
      break;
    }
    if (ttf._token == sym) {
      return ttf._function;
    }
  }   
  return NULL;
}

static float
factor(void)
{
  float number;
  if (is_function_token(sym)) {
    token function_sym = sym;
    accept(sym);
    expect(T_LEFT_BANANA);
    function func = get_function(function_sym);
    number = func(expression());
    expect(T_RIGHT_BANANA);
  } else if (sym == T_NUMBER) {
    number = tokenizer_get_number();
    accept(T_NUMBER);
  } else if (accept(T_LEFT_BANANA)) {
    number = expression();
    expect(T_RIGHT_BANANA);
  } else {
    error("Factor: syntax error");
    get_sym();
  }

  return number; 
}

static float
term(void)
{
  float f1 = factor();
  while (sym == T_MULTIPLY || sym == T_DIVIDE || sym == T_OP_AND) {
    token operator = sym;
    get_sym();
    float f2 = factor();
    switch(operator) {
      case T_MULTIPLY:
        f1 = f1 * f2;
        break;
      case T_DIVIDE:
        f1 = f1 / f2;
        break;
      case T_OP_AND:
        f1 = _and( f1, f2 );
        break;
      default:
        error("term: oops");    
    }
  }
  return f1;
}

static float
expression(void)
{
  token operator = T_PLUS;
  if (sym == T_PLUS || sym == T_MINUS) {
    operator = sym;
    get_sym();
  }
  float t1 = term();
  if (operator == T_MINUS) {
    t1 = -1 * t1;
  }
  while ( sym == T_PLUS || sym == T_MINUS || sym == T_OP_OR ) {
    operator = sym; 
    get_sym();
    float t2 = term();
    switch(operator) {
      case T_PLUS:
        t1 = t1 + t2;
        break;
      case T_MINUS:
        t1 = t1 - t2;
        break;
      case T_OP_OR:
        t1 = _or( t1, t2 );
        break;
      default:
        error("expression: oops");
    }
  }
  return t1;
}

float evaluate(char *expression_string)
{
  last_error = NULL;
  tokenizer_init( expression_string );
  get_sym();
  float result =  expression();
  expect(T_EOF);
  return result;
}

void evaluate_print(char *line)
{
  float result = evaluate(line); 
  printf("%s = %f\n", line, result);
}

void evaluate_print_func_param( char *func, float param)
{
    char *e;
    asprintf(&e, "%s(%f)", func, param);
    evaluate_print(e);
    free(e);
}

const char *evaluate_last_error(void)
{
  return last_error;
}
