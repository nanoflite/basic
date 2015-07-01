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
#include "line.h"

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
    | FOR numeric_variable '=' numeric_expression TO numeric_expression [ STEP number ] 
    | CLEAR
    | LIST
    | RUN
    | END
    | DIM variable "(" expression ")"

  expression-list = ( string | expression ) [, expression-list]

  variable-list = variable [, variable-list]

  expression = string_expression | numeric_expression
  
  numeric_expression = ["+"|"-"] term {("+"|"-"|"OR") term} .

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

/*
#define MAX_NR_LINES 1000
char *__LINES[MAX_NR_LINES];
int __LINE_P = 0;
*/

char __memory[4096];
size_t __memory_p = 0;
size_t __memory_size = sizeof(__memory);

char __stack[1024];
size_t __stack_p = sizeof(__stack);
size_t __stack_size = sizeof(__stack);

bool __RUNNING = false;

typedef union
{
  float numeric;
  char *string;
} expression_value;

typedef enum
{
  expression_type_numeric,
  expression_type_string
} expression_type;

typedef struct
{
  expression_type type;
  expression_value value;    
} expression_result;

typedef enum
{
  stack_frame_type_for,
  stack_frame_type_gosub
} stack_frame_type;

typedef struct
{
  stack_frame_type type;
  char *variable_name; 
  float end_value;
  float step;
  size_t line;
  size_t cursor; 
} stack_frame_for;

typedef struct
{
  stack_frame_type type;
  size_t line;
} stack_frame_gosub;

token sym;
static void
get_sym(void)
{
  sym = tokenizer_get_next_token();
  // printf("token: %s\n", tokenizer_token_name( sym ) );
}

static float numeric_expression(void);
static char* string_expression(void);

void
expression(expression_result *result)
{

  char *string = string_expression();
  if ( NULL != string )
  {
    result->type = expression_type_string;
    result->value.string = string;
  }
  else
  {
    result->type = expression_type_numeric;
    result->value.numeric = numeric_expression();
  }
}

const char *last_error;

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
  {
    printf ("  %s\n", strings[i]);
  }

  free (strings);

  exit(1);
}


static void
move_to_next_line(void)
{
  /*
  __LINE_P++;
  while(__LINES[__LINE_P] == NULL && __LINE_P < MAX_NR_LINES) {
    __LINE_P++;
  }
  */
  /*
  while (__memory_p < __memory_size)
  {
    __memory_p++;
    
    if (__memory[__memory_p] && __memory[__memory_p] == '\n')
    {
      return;
    }
  }
  if (__memory_p == __memory_size)
  {
    error("No more memory.");
  }
  */
  lines_next();
}

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

static bool
accept(token t)
{
  if (t == sym) {
    get_sym();
    return true;
  }
  // printf("accept got %s, expected %s\n", tokenizer_token_name(sym), tokenizer_token_name(t));
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
    number = func(numeric_expression());
    expect(T_RIGHT_BANANA);
  } else if (sym == T_NUMBER) {
    number = tokenizer_get_number();
    accept(T_NUMBER);
  } else if (sym == T_VARIABLE_NUMBER) {
    // printf("NUMBER VAR\n");
    char* var_name = tokenizer_get_variable_name();
    // printf("Var NAME: '%s'\n", var_name);
    number = variable_get_numeric(var_name);
    // printf("number: %f\n", number);
    accept(T_VARIABLE_NUMBER);
  } else if (accept(T_LEFT_BANANA)) {
    number = numeric_expression();
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
  // printf("term\n");
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
numeric_expression(void)
{

  // printf("numeric expression?\n");

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
  // printf("expression: %f\n", t1);
  return t1;
}


static void
ready(void)
{
  puts("READY.");
}

static void
do_list(void)
{
  lines_reset();

  while (true)
  {
    line* l = lines_current();
    if ( l == NULL )
    {
      break;
    }
    printf("%ld %s\n", l->number, l->contents);
    lines_next();
  }
 
  ready();
}

// static void increment_line(void)
// {
//   /*
//    __LINE_P++;
// 
//   if (__LINE_P >= MAX_NR_LINES) {
//     __LINE_P = 0;
//     __RUNNING = false;
//   }
//   */
//   /*
//   while(__memory_p < __memory_size && __memory[__memory_p] && __memory[__memory_p] != '\n'){
//     __memory_p++;
//   } 
//   if (__memory[__memory_p] == '\n')
//   {
//     __memory_p++;
//   }
//   */
// }

static char
chr(void)
{
  get_sym();

  int i =  (int) numeric_expression();

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
  // puts("string_expression\n");

  char *string = NULL;
  char *var_name;

  switch (sym)
  {
    case T_STRING:
      string = strdup(tokenizer_get_string());
      // printf("Found string: '%s'\n", string);
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
      int from = (int) numeric_expression();
      // 4. expect a comma
      expect(T_COMMA);
      // 5. expect an expression as third param
      int to = (int) numeric_expression();
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

  /*
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
      printf("%f", numeric_expression());
      break;
  }
  */

  expression_result expr;
  expression(&expr);

  if (expr.type == expression_type_string)
  {
    printf("%s", expr.value.string);
  }
  else
  if (expr.type == expression_type_numeric)
  {
    printf("%f", expr.value.numeric);
  }
  else
  {
    error("unknown expression");
  }

  if (sym != T_SEMICOLON) {
    printf("\n");
  } else {
    accept(T_SEMICOLON);
  }

  // increment_line(); 

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

  line* l = lines_get_by_number(line_number);
  if (l == NULL) {
    error("Line not found.");
    return;
  }

  lines_set_by_number( line_number );
}

  static void
do_gosub(void)
{
  accept(T_KEYWORD_GOSUB);
  // int line = (int) numeric_expression();
  
  if (sym != T_NUMBER) {
    error("Number expected");
    return;
  }

  int line_number = (int) tokenizer_get_number();

  /*
  // Move to next line
  __LINE_P++;
  while(__LINES[__LINE_P] == NULL && __LINE_P < MAX_NR_LINES) {
    __LINE_P++;
  }
  */
  move_to_next_line();
  
  /*
  stack_push(__LINE_P);
  stack_push(T_KEYWORD_GOSUB);
  */

  stack_frame_gosub *g;
  if ( __stack_p < sizeof(stack_frame_gosub) )
  {
    error("Stack too small.");
    return;
  }

  __stack_p -= sizeof(stack_frame_gosub);
  g = (stack_frame_gosub*) &(__stack[__stack_p]);

  g->type = stack_frame_type_gosub;
  g->line = lines_current()->number;

  lines_set_by_number(line_number);
}

  static void
do_return(void)
{
  accept(T_KEYWORD_RETURN);

  /*
  int keyword = (int) stack_pop();
  if (keyword != T_KEYWORD_GOSUB) {
    error("RETURN without GOSUB");
    return;
  } 
  __LINE_P = stack_pop();
  */

  stack_frame_gosub *g;
  g = (stack_frame_gosub*) &(__stack[__stack_p]);

  if ( g->type != stack_frame_type_gosub )
  {
    error("Uncorrect stack frame, expected gosub");
    return;
  }

  lines_set_by_number( g->line );

  __stack_p += sizeof(stack_frame_gosub);
}

  static void
do_for(void)
{
  // printf("do_for\n");

  accept(T_KEYWORD_FOR);

  if ( sym != T_VARIABLE_NUMBER ) {
    error("Variable expected");
    return;
  }

  char *name = tokenizer_get_variable_name();
  get_sym();
  expect(T_EQUALS);
  float value = numeric_expression();
  variable_set_numeric(name, value);

  expect(T_KEYWORD_TO);
  
  float end_value = numeric_expression();
  
  float step = 1.0;
  get_sym();
  if (sym == T_KEYWORD_STEP) {
    accept(T_KEYWORD_STEP);
    step = numeric_expression();
  }
  
  move_to_next_line();
  /* 
  stack_push(__LINE_P);
  stack_push(step);
  stack_push(end_value);
  stack_push_string(name);
  stack_push(T_KEYWORD_FOR);
  */

  stack_frame_for *f;
  if ( __stack_p <  sizeof(stack_frame_for) )
  {
    error("Stack too small.");
    return;
  }  

  __stack_p -= sizeof(stack_frame_for);
  f = (stack_frame_for*) &(__stack[__stack_p]);
  
  f->type = stack_frame_type_for;
  f->variable_name = name;
  f->end_value = end_value;
  f->step = step;
  f->line = lines_current()->number;
  f->cursor = 0; 
}

  static void
do_next(void)
{
  // printf("do_next\n");

  accept(T_KEYWORD_NEXT);

  /*
  int keyword = (int) stack_pop();
  if (keyword != T_KEYWORD_FOR) {
    error("NEXT without FOR");
    return;
  }

  char *name = stack_string_pop();
  float end_value = stack_pop();
  float step = stack_pop();
  int line = stack_pop();

  float value = variable_get_numeric(name);
  if ( (step > 0 && value >= end_value) || (step < 0 && value <= end_value) )
  {
    // TODO: Move to next instruction
    move_to_next_line();  
  }
  else
  {
    __LINE_P = line;
    stack_push(__LINE_P);
    stack_push(step);
    stack_push(end_value);
    stack_push_string(name);
    stack_push(T_KEYWORD_FOR);
  } 
  */

  stack_frame_for *f;
  f = (stack_frame_for*) &(__stack[__stack_p]);

  if ( f->type != stack_frame_type_for )
  {
    error("Uncorrect stack frame, expected for");
    return;
  }

  float value = variable_get_numeric(f->variable_name);
  if ( (f->step > 0 && value >= f->end_value) || (f->step < 0 && value <= f->end_value) )
  {
      //printf("For has done\n");
      //printf("Move to line behind next");
      __stack_p += sizeof(stack_frame_for);
      move_to_next_line();
      return;
  }

  variable_set_numeric(f->variable_name, value + f->step); 
  lines_set_by_number( f->line );
}

static char *
get_next_line(void)
{
  line* l = lines_next();
  return l->contents;
}

static void parse_line(void);
static void statement(void);

static void
do_run(void)
{
  lines_reset();
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
    parse_line();
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
    accept(T_LESS);
    if (sym == T_EQUALS) {
      accept(T_EQUALS);
      return OP_LE;
    }
    return OP_LT;
  }

  if (sym == T_EQUALS) {
    accept(T_EQUALS);
    return OP_EQ;
  }

  if (sym == T_GREATER) {
    accept(T_GREATER);
    if (sym == T_EQUALS) {
      accept(T_EQUALS);
      return OP_GE;
    }
    return OP_GT;
  }

  return OP_NOP;
}

static bool
numeric_condition(float left, float right, relop op)
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

static bool
string_condition(char *left, char *right, relop op)
{


  int comparison = strcmp(left, right);

  // printf("String condition('%s','%s'): %d\n", left, right, comparison);

  switch(op) {
    case OP_NOP:
      error("No valid relation operator found");
      break;
    case OP_LT:
      return comparison < 0;
    case OP_LE:
      return comparison <= 0;
    case OP_EQ:
      return comparison == 0;
    case OP_GE:
      return comparison >= 0;
    case OP_GT:
      return comparison > 0;  
  }

  return false;
}

static bool
condition(expression_result *left_er, expression_result *right_er, relop op)
{

  if (left_er->type == expression_type_numeric)
  {
    if (right_er->type != expression_type_numeric)
    {
      error("Illegal right hand type, expected numeric.");  
    }
    return numeric_condition(left_er->value.numeric, right_er->value.numeric, op);
  }
  else
  {
    if (right_er->type != expression_type_string)
    {
      error("Illegal right hand type, expected string");
    }
    return string_condition(left_er->value.string, right_er->value.string, op);;
  }
}

static void
do_if(void)
{
  expression_result left_side, right_side;

  get_sym();

  // float left_side = numeric_expression();
  expression(&left_side);
  relop op = get_relop();
  // float right_side = numeric_expression();
  // get_sym();
  expression(&right_side);

  if (sym != T_KEYWORD_THEN) {
    error("IF without THEN.");
    return;
  } 

  if (condition(&left_side, &right_side, op)) {
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
    // printf("I got this name: %s\n", name);
    get_sym();
    expect(T_EQUALS);
    float value = numeric_expression();
    variable_set_numeric(name, value);
  }

  if (sym == T_VARIABLE_STRING) {
    char *name = tokenizer_get_variable_name();
    // printf("I got this name: %s\n", name);
    get_sym();
    expect(T_EQUALS);
    char *value = string_expression();
    variable_set_string(name, value);
  }

}

static void
parse_line(void)
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
  // puts("I do statement");
  // printf("\tdiag: symbol: %d\n", sym);
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
    case T_KEYWORD_GOSUB:
      do_gosub();
      break;
    case T_KEYWORD_RETURN:
      do_return();
      break;
    case T_KEYWORD_RUN:
      do_run();
      break;
    case T_KEYWORD_IF:
      do_if();
      break;
    case T_KEYWORD_FOR:
      do_for();
      break;
    case T_KEYWORD_NEXT:
      do_next();
      break;
    case T_KEYWORD_END:
      __RUNNING = false;
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
  /*
  // TODO:
  statement_func func = statement_get_func(sym);
  func();
  */
}

void basic_init(void)
{
  lines_init(__memory, __memory_size);
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
      lines_delete( line_number );
    } else {
      lines_store( line_number, line_string);
    }
  } else {
    parse_line();
  }
}

float evaluate(char *expression_string)
{
  last_error = NULL;
  tokenizer_init( expression_string );
  get_sym();
  float result =  numeric_expression();
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
