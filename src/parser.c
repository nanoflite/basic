#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <execinfo.h>

#include "tokenizer.h"
#include "variables.h"

/*
  line = [number] statement [ : statement ] CR

  statement =
    PRINT expression-list [ ; ]
    | IF expression relation-operator expression THEN statement
    | GOTO expression
    | INPUT variable-list
    | LET variable = expression
    | GOSUB expression
    | RETURN
    | CLEAR
    | LIST
    | RUN
    | END
    | DIM variable "(" expression ")"

  expression-list = ( string | expression ) [, expression-list]

  variable-list = variable [, variable-list]

  expression = ["+"|"-"] term {("+"|"-"|"OR") term} .

  term = factor {( "*" | "/" | "AND" ) factor} .

  factor = 
    func "(" expression ")" 
    | number
    | "(" expression ")"
    | variable

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

  string = literal_string | string_func "(" string_expression ")"

  literal_string = '"' ... '"'
  
  string_func =
    CHR$

  string_expression = literal_string | string_variable

  variable = ( numeric_variable | string_variable | indexed_variable )

  numeric_variable = A | B | C ... | X | Y | Z

  string_variable = A$ | B$ | C$ ... | X$ | Y$ | Z$

  indexed_variable = ( numeric_variable | string_variable ) "(" expression ")"

  relation-operator = ( "<" | "<=" | "=" | ">=" | ">" )

*/

typedef struct
{
  int line_number;
  int next_line_number;
  char *line;
} line_entry;

#define MAX_NR_LINES 1000
char *__LINES[MAX_NR_LINES];
int __LINE_P = 0;
bool __RUNNING = false;

static float
_abs(float n)
{
  return fabs(n);
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
  printf("token: %s\n", tokenizer_token_name( sym ) );
}

static void
error(const char *error_msg)
{
       void *array[10];
       size_t size;
       char **strings;
       size_t i;
 
  last_error = error_msg;

  printf("--- ERROR: %s\n", error_msg);
    
       size = backtrace (array, 10);
       strings = backtrace_symbols (array, size);
     
       printf ("Showing %zd stack frames:\n", size);
     
       for (i = 0; i < size; i++)
          printf ("  %s\n", strings[i]);
     
       free (strings);
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
  } else if (sym == T_VARIABLE_NUMBER) {
    printf("NUMBER VAR\n");
    char* var_name = tokenizer_get_variable_name();
    printf("Var NAME: '%s'\n", var_name);
    number = variable_get_numeric(var_name);
    printf("number: %f\n", number);
    accept(T_VARIABLE_NUMBER);
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

  printf("expression?\n");

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
  printf("expression: %f\n", t1);
  return t1;
}

static bool line_check_boundaries(int line_number)
{
  if (line_number < 0) {
    return false;
  }

  if (line_number >= MAX_NR_LINES ) {
    return false;
  }

  return true;
}

static bool
line_number_exists(int line_number)
{
  if (!line_check_boundaries(line_number)) {
    error("invalid line number");
    return false;
  }
  return __LINES[line_number] != NULL;
}

static void
line_insert(int line_number, char *line)
{

  if (!line_check_boundaries(line_number)) {
    error("invalid line number");
    return;
  }

  __LINES[line_number] = strdup(line);
}

static void
line_delete(int line_number)
{

  if (!line_check_boundaries(line_number)) {
    error("invalid line number");
    return;
  }

  free(__LINES[line_number]);
  __LINES[line_number] = NULL;
}

static void
line_replace(int line_number, char *line)
{

  if (!line_check_boundaries(line_number)) {
    error("invalid line number");
    return;
  }

  if (__LINES[line_number] == NULL) {
    error("expected a line");
  }

  line_delete(line_number);
  line_insert(line_number, line);
}

static void
store_line( int line_number, char *line )
{
  if (line_number_exists(line_number)) {
    line_replace(line_number, line);
  } else {
    line_insert(line_number, line);
  }
}

static void
ready(void)
{
  puts("READY.");
}

static void
do_list(void)
{
  for(int i=0; i<MAX_NR_LINES; i++) {
    if (__LINES[i] != NULL) {
      puts(__LINES[i]);
    }
  }

  ready();

}

static void increment_line(void)
{
  __LINE_P++;

  if (__LINE_P >= MAX_NR_LINES) {
    __LINE_P = 0;
    __RUNNING = false;
  }

}

static char
chr(void)
{
  get_sym();

  int i =  (int) expression();

  if ( i == 205 ) {
    return '/';
  }

  if ( i == 206 ) {
    return '\\';
  }

  return i;
}

static char*
string_expression(void)
{
  puts("string_expression\n");

  char *string = NULL;
  char *var_name;

  switch (sym)
  {
    case T_STRING:
      string = tokenizer_get_string();
      printf("Found string: '%s'\n", string);
      accept(T_STRING);
      break;

    case T_STRING_FUNC_CHR:
      printf("%c", chr());
      break;

    case T_VARIABLE_STRING:
      var_name = tokenizer_get_variable_name();
      string = variable_get_string(var_name);
      accept(T_VARIABLE_STRING);
      break;

    case T_STRING_FUNC_MID$:
      // 0. expect a left banana
      accept(sym);
      expect(T_LEFT_BANANA);
      // 1. expect a string expression as first parameter (recurse into string_expression)
      char *source = string_expression();
      // 2. expect a comma
      expect(T_COMMA);
      // 3. expect an expression as second param
      int from = (int) expression();
      // 4. expect a comma
      expect(T_COMMA);
      // 5. expect an expression as third param
      int to = (int) expression();
      // 6. expect a right banana
      expect(T_RIGHT_BANANA);

      //TODO: Better MID$ implementation... optional third parameter
      string = strdup(&source[from]);
      string[to] = '\0'; 
    default:
      break;
  }

  return string;
}

static void
do_print(void)
{
  get_sym();

  switch (sym) {
    case T_STRING:
      printf("%s", tokenizer_get_string());
      accept(T_STRING);
      break;

    case T_STRING_FUNC_CHR:
      printf("%c", chr());
      break;

    case T_VARIABLE_STRING:
      printf("STRING VAR\n");
      char* var_name = tokenizer_get_variable_name();
      printf("string var name: '%s'\n", var_name);
      char *string = variable_get_string(var_name);
      printf("%s", string);
      accept(T_VARIABLE_STRING);
      break;

    case T_EOF:
      printf("\n");
      break;

    default:
      printf("%f", expression());
      break;
  }

  if (sym != T_SEMICOLON) {
    printf("\n");
  } else {
    accept(T_SEMICOLON);
  }

  increment_line(); 

}

static void
do_goto(void)
{
  get_sym();
  
  if (sym != T_NUMBER) {
    error("Number expected");
    return;
  }

  int line_number = (int) tokenizer_get_number();

  if (__LINES[line_number] == NULL) {
    error("Line not found.");
    return;
  }

  __LINE_P = line_number;

}

static char *
get_next_line(void)
{
  while(__LINES[__LINE_P] == NULL && __LINE_P < MAX_NR_LINES) {
    __LINE_P++;
  }

  if (__LINE_P < MAX_NR_LINES) {
    return __LINES[__LINE_P];
  }

  return NULL;
}

static void line(void);
static void statement(void);

static void
do_run(void)
{
  __LINE_P = 0;
  __RUNNING = true;
  while (__RUNNING) {
    char *code = get_next_line();
    if (code == NULL) {
      __RUNNING = false;
      break;
    }
    tokenizer_init( code );
    get_sym();
    // expect number, throw away
    if (sym != T_NUMBER) {
      error("Where's the number?");
      exit(-1);
    }
    get_sym();
    line();
  }
 
  ready();  
}

typedef enum {
  OP_NOP,
  OP_LT,
  OP_LE,
  OP_EQ,
  OP_GE,
  OP_GT
} relop;

static relop
get_relop(void)
{

  if (sym == T_LESS) {
    get_sym();
    if (sym == T_EQUALS) {
      return OP_LE;
    }
    return OP_LT;
  }

  if (sym == T_EQUALS) {
    return OP_EQ;
  }

  if (sym == T_GREATER) {
    get_sym();
    if (sym == T_EQUALS) {
      return OP_GE;
    }
    return OP_GT;
  }

  return OP_NOP;
}

static bool
condition(float left, float right, relop op)
{

  switch(op) {
    case OP_NOP:
      error("No valid relation operator found");
      break;
    case OP_LT:
      return left < right;
    case OP_LE:
      return left <= right;
    case OP_EQ:
      return left == right;
    case OP_GE:
      return left >= right;
    case OP_GT:
      return left > right;  
  }

  return false;
}

static void
do_if(void)
{

  get_sym();

  float left_side = expression();
  relop op = get_relop();
  float right_side = expression();

  if (sym != T_KEYWORD_THEN) {
    error("IF without THEN.");
    return;
  } 

  if (condition(left_side, right_side, op)) {
    get_sym();
    statement();
  }

}

static void
do_let(void)
{
  if (sym != T_VARIABLE_NUMBER && sym != T_VARIABLE_STRING) {
    error("Expected a variable");
    return;
  }

  if (sym == T_VARIABLE_NUMBER) {
    char *name = tokenizer_get_variable_name();
    printf("I got this name: %s\n", name);
    get_sym();
    expect(T_EQUALS);
    float value = expression();
    variable_set_numeric(name, value);
  }

  if (sym == T_VARIABLE_STRING) {
    char *name = tokenizer_get_variable_name();
    printf("I got this name: %s\n", name);
    get_sym();
    expect(T_EQUALS);
    char *value = string_expression();
    variable_set_string(name, value);
  }

}

static void
line(void)
{
  while (sym != T_EOF) {
    statement();
    get_sym();
    if (sym != T_COLON) {
      break;
    }
  }
}

static void
statement(void)
{
  puts("I do statement");
  printf("\tdiag: symbol: %d\n", sym);
  switch(sym) {
    case T_KEYWORD_LIST:
      do_list();
      break;
    case T_KEYWORD_PRINT:
      do_print();
      break;
    case T_KEYWORD_GOTO:
      do_goto();
      break;
    case T_KEYWORD_RUN:
      do_run();
      break;
    case T_KEYWORD_IF:
      do_if();
      break;
    case T_ERROR:
      error("Oh no... T_ERROR");
      break;
    case T_KEYWORD_LET:
      get_sym();
    default:
      do_let();
      break;
  }
}

void basic_init(void)
{
  for(int i=0; i<MAX_NR_LINES; i++) {
    __LINES[i] = NULL;
  }

  variables_init();
}

void
basic_eval(char *line_string)
{
  tokenizer_init( line_string );
  get_sym();
  // printf("diag: symbol: %d\n", sym);
  if (sym == T_NUMBER ) {
    float line_number = tokenizer_get_number();
    get_sym();
    if (sym == T_EOF) {
      line_delete( line_number );
    } else {
      store_line( line_number, line_string);
    }
  } else {
    line();
  }
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
