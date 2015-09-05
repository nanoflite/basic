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
#include "lines.h"
#include "array.h"

/*
  line = [number] statement [ : statement ] CR

  statement =
    PRINT expression-list [ ; ]
    | IF relation-expression THEN statement
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
    | relation-expression

  relation-expression =
    expression relation-operator expression

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

typedef float (*function)(float number);

typedef struct
{
  token _token;
  function _function;
} token_to_function;

typedef enum {
  basic_function_type_keyword,
  basic_function_type_op,
  basic_function_type_numeric,
  basic_function_type_string
} basic_function_type;

typedef enum {
  kind_numeric,
  kind_string
} kind;

typedef union {
  float number;
  char* string;
} value;

typedef struct {
  kind kind;
  value value;
} basic_type;

typedef int (*function_0)(basic_type* rv);
typedef int (*function_1)(basic_type* v1, basic_type* rv);
typedef int (*function_2)(basic_type* v1, basic_type* v2, basic_type* rv);
typedef int (*function_3)(basic_type* v1, basic_type* v2, basic_type* v3, basic_type* rv);
typedef int (*function_4)(basic_type* v1, basic_type* v2, basic_type* v3, basic_type* v4, basic_type* rv);
typedef int (*function_5)(basic_type* v1, basic_type* v2, basic_type* v3, basic_type* v4, basic_type* v5, basic_type* rv);

typedef union
{
  function_0 function_0;
  function_1 function_1;
  function_2 function_2;
  function_3 function_3;
  function_4 function_4;
  function_5 function_5;
} basic_function_union;

typedef struct {
  token token;
  basic_function_type type;
  size_t nr_arguments;
  kind kind_1;
  kind kind_2;
  kind kind_3;
  kind kind_4;
  kind kind_5;
  basic_function_union function; 
} basic_function;

static array* basic_tokens = NULL;
static array* basic_functions = NULL;
static token t_keyword_print;
static token t_keyword_goto;
static token t_keyword_if;
static token t_keyword_then;
static token t_keyword_let;
// static token t_keyword_input;
static token t_keyword_gosub;
static token t_keyword_return;
// static token t_keyword_clear;
static token t_keyword_list;
static token t_keyword_run;
static token t_keyword_end;
static token t_keyword_for;
static token t_keyword_to;
static token t_keyword_step;
static token t_keyword_next;
static token t_op_or;
static token t_op_and;

typedef enum {
  // T_FUNC_ABS = TOKEN_TYPE_END,
  // T_FUNC_SIN,
  // T_FUNC_COS,
  // T_FUNC_RND,
  // T_FUNC_INT,
  // T_FUNC_TAN,
  // T_FUNC_SQR,
  // T_FUNC_SGN,
  // T_FUNC_LOG,
  // T_FUNC_EXP,
  // T_FUNC_ATN,
  // T_FUNC_NOT,
  T_STRING_FUNC_CHR,
  T_STRING_FUNC_MID$,
  //T_OP_OR,
  //T_OP_AND,
  // T_KEYWORD_PRINT,
  // T_KEYWORD_GOTO,
  // T_KEYWORD_IF,
  // T_KEYWORD_THEN,
  // T_KEYWORD_LET,
  // T_KEYWORD_INPUT,
  // T_KEYWORD_GOSUB,
  // T_KEYWORD_RETURN,
  // T_KEYWORD_CLEAR,
  // T_KEYWORD_LIST,
  // T_KEYWORD_RUN,
  // T_KEYWORD_END,
  // T_KEYWORD_FOR,
  // T_KEYWORD_TO,
  // T_KEYWORD_STEP,
  // T_KEYWORD_NEXT,
  // T_STRING_FUNC_LEN
} token_type_basic;

//add_token( T_FUNC_ABS, "ABS" );
//add_token( T_FUNC_SIN, "SIN" );
//add_token( T_FUNC_COS, "COS" );
//add_token( T_FUNC_RND, "RND" );
//add_token( T_FUNC_INT, "INT" );
//add_token( T_FUNC_TAN, "TAN" );
//add_token( T_FUNC_SQR, "SQR" );
//add_token( T_FUNC_SGN, "SGN" );
//add_token( T_FUNC_LOG, "LOG" );
//add_token( T_FUNC_EXP, "EXP" );
//add_token( T_FUNC_ATN, "ATN" );
//add_token( T_FUNC_NOT, "NOT" );
// add_token( T_OP_OR, "OR" );
// add_token( T_OP_AND, "AND" );
// add_token( T_KEYWORD_PRINT, "PRINT" );
// add_token( T_KEYWORD_GOTO, "GOTO" );
// add_token( T_KEYWORD_IF, "IF" );
// add_token( T_KEYWORD_THEN, "THEN" );
// add_token( T_KEYWORD_LET, "LET" );
// add_token( T_KEYWORD_INPUT, "INPUT" );
// add_token( T_KEYWORD_GOSUB, "GOSUB" );
// add_token( T_KEYWORD_RETURN, "RETURN" );
// add_token( T_KEYWORD_FOR, "FOR" );
// add_token( T_KEYWORD_TO, "TO" );
// add_token( T_KEYWORD_STEP, "STEP" );
// add_token( T_KEYWORD_NEXT, "NEXT" );
// add_token( T_KEYWORD_CLEAR, "CLEAR" );
// add_token( T_KEYWORD_LIST, "LIST" );
// add_token( T_KEYWORD_RUN, "RUN" );
// add_token( T_KEYWORD_END, "END" );
add_token( T_STRING_FUNC_CHR, "CHR$" );
add_token( T_STRING_FUNC_MID$, "MID$" );

// add_token( T_STRING_FUNC_LEN, "LEN" );

static uint16_t __line;
static char* __cursor;
static char* __memory;
static char* __stack;
static size_t __memory_size;
static size_t __stack_size;
static size_t __program_size;
static size_t __stack_p;

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
  char* cursor; 
} stack_frame_for;

typedef struct
{
  stack_frame_type type;
  size_t line;
  char* cursor;
} stack_frame_gosub;

//static bool is_basic_function_token(token sym);
//static basic_function* get_basic_function(token sym);
static int basic_dispatch_function(basic_function* function, basic_type* rv);
static basic_function* find_basic_function_by_type(token sym, basic_function_type type);

token register_token(char* name , char* keyword);
token register_function_0(basic_function_type type, char* keyword, function_0 function);
token register_function_1(basic_function_type type, char* keyword, function_1 function, kind v1);
token register_function_2(basic_function_type type, char* keyword, function_2 function, kind v1, kind v2);
token register_function_3(basic_function_type type, char* keyword, function_3 function, kind v1, kind v2, kind v3);
token register_function_4(basic_function_type type, char* keyword, function_4 function, kind v1, kind v2, kind v3, kind v4);
token register_function_5(basic_function_type type, char* keyword, function_5 function, kind v1, kind v2, kind v3, kind v4, kind v5);

int str_len(basic_type* str, basic_type* rv);

typedef enum {
  OP_NOP,
  OP_LT,
  OP_LE,
  OP_EQ,
  OP_GE,
  OP_GT
} relop;

static bool numeric_condition(float left, float right, relop op);
static relop get_relop(void);

// size_t __TOKEN_I = TOKEN_TYPE_END + 1000;

// #define __TOKEN_I (TOKEN_TYPE_END + 1000)

/*
#define token_id 0

#define register_function_1(t, name, f, v1) \
  add_token( token_id, name ); \
  basic_function bf_##f = { \
    .token = token_id, \
    .type = basic_function_type_##t, \
    .nr_arguments = 1, \
    .kind_1 = kind_##v1, \
    .function.function_1 = f \
  }; \
*/
/*
basic_function bf_len = {
  .token = T_STRING_FUNC_LEN,
  .type = basic_function_type_numeric,
  .nr_arguments = 1,
  .kind_1 = kind_string,
  .function.function_1 = str_len
};
*/
// register_function_1(numeric, "LEN", str_len, string);

token sym;
static void
get_sym(void)
{
  sym = tokenizer_get_next_token();
  // printf("token: %ld\n", sym);
  // printf("token: %s\n", tokenizer_token_name( sym ) );
}

static void
set_line( uint16_t line_number )
{
  __line = line_number;
  char *cursor = lines_get_contents( __line );
  tokenizer_char_pointer( cursor );
}

static float numeric_expression(void);
static char* string_expression(void);

void
expression(expression_result *result)
{
  // printf("expression\n");
  char *string = string_expression();
  if ( NULL != string )
  {
    // printf("string");
    result->type = expression_type_string;
    result->value.string = string;
  }
  else
  {
    // printf("numeric");
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

  printf("--- ERROR: %d %s\n", __line, error_msg);

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

static int
f_abs(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = fabs(n->value.number);
  return 0;
}

static int
f_rnd(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  if (n->value.number > 0)
  {
    int random = rand();
    rv->value.number = (random * 1.0) / RAND_MAX;
    return 0;
  }

  if (n->value.number < 0)
  {
    srand(n->value.number);
    int random = rand();
    rv->value.number = (random * 1.0) / RAND_MAX;
    return 0;
  }

  time_t now;
  struct tm *tm;
  now = time(NULL);    
  tm = localtime(&now);
  rv->value.number = (tm->tm_sec * 1.0) / 60;
  return 0;
}

static int
f_int(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  int i = (int) n->value.number;  
  rv->value.number = 1.0 * i;
  return 0;
}

static int
f_sqr(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = (float) sqrt( (double) n->value.number );
  return 0;
}

static int
f_sgn(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  if (n->value.number < 0)
  {
    rv->value.number = -1.0;
  }
  else
  if (n->value.number > 0)
  {
    rv->value.number = 1.0;
  }
  else
  {
    rv->value.number = 0.0;
  }
  return 0; 
}

static int
f_not(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = (float) ( ~ (int) n->value.number );
  return 0;
}

static int
f_sin(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = sinf(n->value.number);
  return 0;
}

static int
f_cos(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = cosf(n->value.number);
  return 0;
}

static int
f_tan(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = tanf(n->value.number);
  return 0;
}

static int
f_log(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = logf(n->value.number);
  return 0;
}

static int
f_exp(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = expf(n->value.number);
  return 0;
}

static int
f_atn(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = atanf(n->value.number);
  return 0;
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

//token_to_function token_to_functions[] = 
//{
//  { T_FUNC_ABS, _abs },
//  { T_FUNC_SIN, sinf },
//  { T_FUNC_COS, cosf },
//  { T_FUNC_RND, _rnd },
//  { T_FUNC_INT, _int },
//  { T_FUNC_TAN, tanf },
//  { T_FUNC_SQR, _sqr },
//  { T_FUNC_SGN, _sgn },
//  { T_FUNC_LOG, logf },
//  { T_FUNC_EXP, expf },
//  { T_FUNC_ATN, atanf },
//  { T_FUNC_NOT, _not },
//  { T_EOF, NULL }
//};

static bool
accept(token t)
{
  // printf("accept %ld\n", t);
  if (t == sym) {
    get_sym();
    return true;
  }
  // printf("accept got %ld, expected %ld\n", sym, t);
  return false;
}

static bool
expect(token t)
{
  // printf("expect %ld\n", t);
  if (accept(t)) {
    return true;
  }
  error("Expect: unexpected symbol");
  return false;
}

//static bool
//is_function_token(token sym)
//{
//  for(size_t i=0; token_to_functions[i]._token != T_EOF;i++) {
//    if (sym == token_to_functions[i]._token) {
//      return true;
//    }
//  }
//
//  return false;
//}
//
//static function
//get_function(token sym)
//{
//  token_to_function ttf;
//  for(size_t i = 0;; i++) {
//    ttf = token_to_functions[i];
//    if (ttf._token == T_EOF) {
//      break;
//    }
//    if (ttf._token == sym) {
//      return ttf._function;
//    }
//  }   
//  return NULL;
//}

static float
factor(void)
{
  // printf("  factor: %ld\n", sym);

  float number;
  basic_function* bf;
//  if (is_function_token(sym)) {
//    // printf("function token\n");
//    token function_sym = sym;
//    accept(sym);
//    expect(T_LEFT_BANANA);
//    function func = get_function(function_sym);
//    number = func(numeric_expression());
//    expect(T_RIGHT_BANANA);
//  } else if ( (bf = find_basic_function_by_type(sym, basic_function_type_numeric)) != NULL ) {
  if ( (bf = find_basic_function_by_type(sym, basic_function_type_numeric)) != NULL ) {
    // printf("basic function\n");
    basic_type rv;
    basic_dispatch_function( bf, &rv);
    if (rv.kind != kind_numeric)
    {
      error("Expected numeric.");
    }
    number = rv.value.number;
  } else if (sym == T_NUMBER) {
    // printf("number\n");
    number = tokenizer_get_number();
    accept(T_NUMBER);
  } else if (sym == T_VARIABLE_NUMBER) {
    // printf("variable number\n");
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
    // printf("sym: %ld\n", sym);
    error("Factor: syntax error");
    get_sym();
  }

  relop op = get_relop();
  if (op != OP_NOP)
  {
    float right_number = factor();    
    number = numeric_condition(number, right_number, op);
  }

  return number; 
}

static float
term(void)
{

  // printf("term\n");

  float f1 = factor();
  // while (sym == T_MULTIPLY || sym == T_DIVIDE || sym == t_op_and || sym == T_EQUALS || sym == T_LESS || sym == T_GREATER ) {
  while (sym == T_MULTIPLY || sym == T_DIVIDE || sym == t_op_and ) {
    token operator = sym;
    //relop op;
    //if ( sym == T_EQUALS || sym == T_LESS || sym == T_GREATER )
    //{
    //  op = get_relop();
    //}
    //else
    //{
      get_sym();
    //}
    float f2 = factor();
    switch(operator) {
      case T_MULTIPLY:
        f1 = f1 * f2;
        break;
      case T_DIVIDE:
        f1 = f1 / f2;
        break;
      //case T_EQUALS:
      //case T_LESS:
      //case T_GREATER:
      //{
      //  f1 = numeric_condition(f1, f2, op);
      //  break;
      //}
      default:
        if (operator == t_op_and)
        {
          f1 = _and( f1, f2 );
        }
        else
        {
        error("term: oops");    
        }
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
  // printf("get term 1\n");
  float t1 = term();
  if (operator == T_MINUS) {
    t1 = -1 * t1;
  }
  //while ( sym == T_PLUS || sym == T_MINUS || sym == t_op_or || sym == T_EQUALS || sym == T_LESS || sym == T_GREATER ) {
  while ( sym == T_PLUS || sym == T_MINUS || sym == t_op_or ) {
    operator = sym;
    //relop op;
    //if ( sym == T_EQUALS || sym == T_LESS || sym == T_GREATER )
    //{
    //  op = get_relop();
    //}
    //else
    //{
      get_sym();
    //}
    // printf("get term 2\n");
    float t2 = term();
    switch(operator) {
      case T_PLUS:
        t1 = t1 + t2;
        break;
      case T_MINUS:
        t1 = t1 - t2;
        break;
      //case T_EQUALS:
      //case T_LESS:
      //case T_GREATER:
      //{
      //  t1 = numeric_condition( t1, t2, op );
      //  break;
      //}
      default:
        if ( operator == t_op_or )
        {
          t1 = _or( t1, t2 );
        }
        else
        {
        error("expression: oops");
        }
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
list_out(uint16_t number, char* contents)
{
  printf("%d %s\n", number, contents);
}

//static void
//do_list(void)
static int
do_list(basic_type* rv)
{
  accept(t_keyword_list);
  lines_list(list_out);
  ready();
  return 0;
}

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

    // TODO: register functions -> look up functions that return strings here!
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
      //if (is_basic_function_token(sym))
      //{
      //  basic_function* bf = get_basic_function(sym);
      //  if (bf->type == basic_function_type_string)
      //  {
      //    // printf("string_expression function\n");
      //    basic_type rv;
      //    basic_dispatch_function( get_basic_function(sym), &rv);
      //    if (rv.kind != kind_string)
      //    {
      //      error("Expected string.");
      //    }
      //    string = rv.value.string;
      //  }
      //
      {
        basic_function* bf = find_basic_function_by_type(sym, basic_function_type_string);
        if ( bf != NULL )
        {
          basic_type rv;
          basic_dispatch_function( bf, &rv);
          if (rv.kind != kind_string)
          {
            error("Expected string.");
          }
          string = rv.value.string;
        }
      }
      break;
  }

  return string;
}

/*
static void
do_print(void)
*/
  static int
do_print(basic_type* rv)
{
  //accept(T_KEYWORD_PRINT);
  accept(t_keyword_print);

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

  return 0;
}

//static void
//do_goto(void)
static int
do_goto(basic_type* rv)
{
  accept(t_keyword_goto);
  
  if (sym != T_NUMBER) {
    error("Number expected");
    return 0;
  }

  int line_number = (int) tokenizer_get_number();
  accept(T_NUMBER);

  char* line = lines_get_contents(line_number);
  if (line == NULL) {
    error("Line not found.");
    return 0;
  }

  set_line( line_number );

  return 0;
}

//  static void
//do_gosub(void)
static int
do_gosub(basic_type* rv)
{
  // printf("do_gosub\n");
  accept(t_keyword_gosub);
  
  if (sym != T_NUMBER) {
    error("Number expected");
    return 0;
  }
  int line_number = (int) tokenizer_get_number();
  // printf("line number: %d\n", line_number);
  accept(T_NUMBER);

  stack_frame_gosub *g;
  if ( __stack_p < sizeof(stack_frame_gosub) )
  {
    error("Stack too small.");
    return 0;
  }

  __stack_p -= sizeof(stack_frame_gosub);
  g = (stack_frame_gosub*) &(__stack[__stack_p]);

  g->type = stack_frame_type_gosub;
  g->line = __line;
  g->cursor = tokenizer_char_pointer(NULL); 

  set_line( line_number );

  return 0;
}

//  static void
//do_return(void)
static int
do_return(basic_type* rv)
{
  // printf("do_return");
  accept(t_keyword_return);

  stack_frame_gosub *g;
  g = (stack_frame_gosub*) &(__stack[__stack_p]);

  if ( g->type != stack_frame_type_gosub )
  {
    error("Uncorrect stack frame, expected gosub");
    return 0;
  }

  __line = g->line;
  tokenizer_char_pointer( g->cursor );

  __stack_p += sizeof(stack_frame_gosub);

  return 0;
}

//  static void
//do_for(void)
static int
do_for(basic_type* rv)
{
  // printf("do_for\n");

  accept(t_keyword_for);

  if ( sym != T_VARIABLE_NUMBER ) {
    error("Variable expected");
    return 0;
  }

  char *name = tokenizer_get_variable_name();
  get_sym();
  expect(T_EQUALS);
  float value = numeric_expression();
  variable_set_numeric(name, value);

  expect(t_keyword_to);
  
  float end_value = numeric_expression();

  float step = 1.0;
  if (sym != T_EOF && sym != T_COLON)
  {
    expect(t_keyword_step);
    step = numeric_expression();
  }  

  stack_frame_for *f;
  if ( __stack_p <  sizeof(stack_frame_for) )
  {
    error("Stack too small.");
    return 0;
  }  

  __stack_p -= sizeof(stack_frame_for);
  f = (stack_frame_for*) &(__stack[__stack_p]);
  
  f->type = stack_frame_type_for;
  f->variable_name = name;
  f->end_value = end_value;
  f->step = step;
  f->line = __line;
  f->cursor = tokenizer_char_pointer(NULL); 

  return 0;
}

//  static void
//do_next(void)
static int
do_next(basic_type* rv)
{
  // printf("do_next\n");
  accept(t_keyword_next);

  stack_frame_for *f;
  f = (stack_frame_for*) &(__stack[__stack_p]);

  if ( f->type != stack_frame_type_for )
  {
    error("Uncorrect stack frame, expected for");
    return 0;
  }

  if (sym == T_VARIABLE_NUMBER)
  {
    char* var_name = tokenizer_get_variable_name();
    accept(T_VARIABLE_NUMBER);
    if ( strcmp(var_name, f->variable_name) != 0 )
    {
      error("Expected for with other var");
      return 0;
    }
  }

  float value = variable_get_numeric(f->variable_name) + f->step;
  if ( (f->step > 0 && value > f->end_value) || (f->step < 0 && value < f->end_value) )
  {
      // printf("for done\n");
      __stack_p += sizeof(stack_frame_for);
      return 0;
  }

  // printf("n: %s\n", f->variable_name);
  variable_set_numeric(f->variable_name, value); 

  __line = f->line;
  tokenizer_char_pointer( f->cursor );

  return 0;
}

static int
do_end(basic_type* rv)
{
  __RUNNING = false;
  return 0;
}

static void parse_line(void);
static void statement(void);

//static void
//do_run(void)
static int
do_run(basic_type* rv)
{
  __line = lines_first();
  __cursor = lines_get_contents(__line);
  tokenizer_init( __cursor );
  __RUNNING = true;
  while (__cursor && __RUNNING)
  {
    get_sym();
    if ( sym == T_EOF ) {
      __line = lines_next(__line);
      __cursor = lines_get_contents(__line);
      if ( __cursor == NULL )
      {
        __RUNNING = false;
        break;
      }
      tokenizer_init( __cursor );
    }
    parse_line();
  }
 
  ready(); 

  return 0; 
}

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

  // printf("numeric condition %f, %f, %d\n", left, right, op);

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
move_to_next_statement(void)
{
  while (sym != T_EOF && sym != T_COLON) {
    get_sym();
  }
}

//static void
//do_if(void)
static int
do_if(basic_type* rv)
{
  // printf("do if\n");
  // accept(t_keyword_if);

  expression_result left_side, right_side;
  bool result;

  // get_sym();

  // float left_side = numeric_expression();
  expression(&left_side);

  if ( left_side.type == expression_type_string )
  {
    relop op = get_relop();
  // float right_side = numeric_expression();
  // get_sym();
    expression(&right_side);
    result = condition(&left_side, &right_side, op);
  }
  else
  {
    result = left_side.value.numeric == 1.0;
  }

  //printf("left: %f, op: %d, right: %f\n", left_side.value.numeric, op, right_side.value.numeric);

  // printf("sym: %ld\n", sym);

  if (sym != t_keyword_then) {
    error("IF without THEN.");
    return 0;
  } 
  
  // printf("check condition\n");

  if (result) {
    // printf("condition reached");
    get_sym();
    statement();
  } else {
    // printf("condition not reached");
    // move to next line or statement
    move_to_next_statement();
  }

  return 0;
}

//static void
//do_let(void)
static int
do_let(basic_type* rv)
{
  if (sym != T_VARIABLE_NUMBER && sym != T_VARIABLE_STRING) {
    error("Expected a variable");
    return 0;
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

  return 0;
}

static void
parse_line(void)
{
  while (sym != T_EOF && sym != T_COLON) {
    statement();
  }
}

static void
statement(void)
{
  // puts("statement");
  // printf("\tdiag: symbol: %ld\n", sym);
  switch(sym) {
    //case T_KEYWORD_LIST:
    //  do_list();
    //  break;
    // case T_KEYWORD_PRINT:
    //  do_print(NULL);
    //  break;
    // case T_KEYWORD_GOTO:
    //  do_goto();
    //  break;
    //case T_KEYWORD_GOSUB:
    //  do_gosub();
    //  break;
    // case T_KEYWORD_RETURN:
    //  do_return();
    //  break;
    //case T_KEYWORD_RUN:
    //  do_run();
    //  break;
    //case T_KEYWORD_IF:
    //  do_if();
    //  break;
    //case T_KEYWORD_FOR:
    //  do_for();
    //  break;
    // case T_KEYWORD_NEXT:
    //  do_next();
    //  break;
    //case T_KEYWORD_END:
    //  __RUNNING = false;
    //  break;
    case T_ERROR:
      error("Oh no... T_ERROR");
      break;
    // case T_KEYWORD_LET:
    //  get_sym();
    default:
//      if (sym == t_keyword_print)
//      {
//        do_print(NULL);
//      }
      {
        basic_function* bf = find_basic_function_by_type(sym, basic_function_type_keyword);
        if ( bf != NULL )
        {
          basic_type rv;
          basic_dispatch_function( bf, &rv);
        }
        else
        {
          do_let(NULL);
        }
      }
      break;
  }
  /*
  // TODO:
  statement_func func = statement_get_func(sym);
  func();
  */
}


void basic_init(char* memory, size_t memory_size, size_t stack_size)
{
  __memory = memory;
  __memory_size = memory_size;
  __stack_size = stack_size;

  __line = 0;
  
  __stack = __memory + __memory_size - __stack_size;
  __stack_p = __stack_size;
  __program_size = __memory_size - __stack_size;

  basic_tokens = array_new(sizeof(token_entry));
  basic_functions = array_new(sizeof(basic_function));

  tokenizer_setup();

  // tokenizer_register_token( &_T_FUNC_ABS );
  // tokenizer_register_token( &_T_FUNC_SIN );
  // tokenizer_register_token( &_T_FUNC_COS );
  // tokenizer_register_token( &_T_FUNC_RND );
  // tokenizer_register_token( &_T_FUNC_INT );
  // tokenizer_register_token( &_T_FUNC_TAN );
  // tokenizer_register_token( &_T_FUNC_SQR );
  // tokenizer_register_token( &_T_FUNC_SGN );
  // tokenizer_register_token( &_T_FUNC_LOG );
  // tokenizer_register_token( &_T_FUNC_EXP );
  // tokenizer_register_token( &_T_FUNC_ATN );
  // tokenizer_register_token( &_T_FUNC_NOT );
  // tokenizer_register_token( &_T_OP_OR );
  // tokenizer_register_token( &_T_OP_AND );
  // tokenizer_register_token( &_T_KEYWORD_PRINT );
  // tokenizer_register_token( &_T_KEYWORD_GOTO );
  // tokenizer_register_token( &_T_KEYWORD_IF );
  // tokenizer_register_token( &_T_KEYWORD_THEN );
  // tokenizer_register_token( &_T_KEYWORD_LET );
  // tokenizer_register_token( &_T_KEYWORD_INPUT );
  // tokenizer_register_token( &_T_KEYWORD_GOSUB );
  // tokenizer_register_token( &_T_KEYWORD_RETURN );
  // tokenizer_register_token( &_T_KEYWORD_FOR );
  // tokenizer_register_token( &_T_KEYWORD_TO );
  // tokenizer_register_token( &_T_KEYWORD_STEP );
  // tokenizer_register_token( &_T_KEYWORD_NEXT );
  // tokenizer_register_token( &_T_KEYWORD_CLEAR );
  // tokenizer_register_token( &_T_KEYWORD_LIST );
  // tokenizer_register_token( &_T_KEYWORD_RUN );
  // tokenizer_register_token( &_T_KEYWORD_END );
  tokenizer_register_token( &_T_STRING_FUNC_CHR );
  tokenizer_register_token( &_T_STRING_FUNC_MID$ );
  // tokenizer_register_token( &_T_STRING_FUNC_LEN );


  // BASIC keywords
  t_keyword_print = register_function_0(basic_function_type_keyword, "PRINT", do_print);
  t_keyword_list = register_function_0(basic_function_type_keyword, "LIST", do_list);
  t_keyword_goto = register_function_0(basic_function_type_keyword, "GOTO", do_goto);
  t_keyword_gosub = register_function_0(basic_function_type_keyword, "GOSUB", do_gosub); 
  t_keyword_return = register_function_0(basic_function_type_keyword, "RETURN", do_return);
  t_keyword_run = register_function_0(basic_function_type_keyword, "RUN", do_run);
  t_keyword_if = register_function_0(basic_function_type_keyword, "IF", do_if);
  t_keyword_then = register_token("THEN", "THEN");
  t_keyword_for = register_function_0(basic_function_type_keyword, "FOR", do_for);
  t_keyword_to = register_token("TO", "TO");
  t_keyword_step = register_token("STEP", "STEP");
  t_keyword_next = register_function_0(basic_function_type_keyword, "NEXT", do_next);
  t_keyword_end = register_function_0(basic_function_type_keyword, "END", do_end);
  t_keyword_let = register_function_0(basic_function_type_keyword, "LET", do_let);

  // LOGICAL and BINARY operators
  t_op_or = register_token("OR", "OR");
  t_op_and = register_token("AND", "AND");
 
  // BASIC functions 
  register_function_1(basic_function_type_numeric, "ABS", f_abs, kind_numeric);
  register_function_1(basic_function_type_numeric, "SIN", f_sin, kind_numeric);
  register_function_1(basic_function_type_numeric, "COS", f_cos, kind_numeric);
  register_function_1(basic_function_type_numeric, "RND", f_rnd, kind_numeric);
  register_function_1(basic_function_type_numeric, "INT", f_int, kind_numeric);
  register_function_1(basic_function_type_numeric, "TAN", f_tan, kind_numeric);
  register_function_1(basic_function_type_numeric, "SQR", f_sqr, kind_numeric);
  register_function_1(basic_function_type_numeric, "SGN", f_sgn, kind_numeric);
  register_function_1(basic_function_type_numeric, "LOG", f_log, kind_numeric);
  register_function_1(basic_function_type_numeric, "EXP", f_exp, kind_numeric);
  register_function_1(basic_function_type_numeric, "ATN", f_atn, kind_numeric);
  register_function_1(basic_function_type_numeric, "NOT", f_not, kind_numeric);

  register_function_1(basic_function_type_numeric, "LEN", str_len, kind_string);

  lines_init(__memory, __program_size);
  variables_init();
}

void
basic_eval(char *line_string)
{
  tokenizer_init( line_string );
  get_sym();
  // printf("diag: symbol: %ld\n", sym);
  if (sym == T_NUMBER ) {
    float line_number = tokenizer_get_number();
    char* line = tokenizer_char_pointer(NULL);
    get_sym();
    if (sym == T_EOF) {
      lines_delete( line_number );
    } else {
      lines_store( line_number, line);
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

// - Register functions

static size_t basic_token_id = TOKEN_TYPE_END + 1000; 

token
register_token(char* name , char* keyword)
{
  token_entry token;

  token.token = basic_token_id++;
  token.name = name;
  token.keyword = keyword;

  // printf("token '%s' = %ld\n", keyword, token.token);

  tokenizer_register_token(&token);

  return token.token;
}

token
register_function_0(basic_function_type type, char* keyword, function_0 function)
{
  token t = register_token(keyword, keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 0,
    .function.function_0 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_1(basic_function_type type, char* keyword, function_1 function, kind v1)
{
  token t = register_token(keyword, keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 1,
    .kind_1 = v1,
    .function.function_1 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_2(basic_function_type type, char* keyword, function_2 function, kind v1, kind v2)
{
  token t = register_token(keyword, keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 2,
    .kind_1 = v1,
    .kind_2 = v2,
    .function.function_2 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_3(basic_function_type type, char* keyword, function_3 function, kind v1, kind v2, kind v3)
{
  token t = register_token(keyword, keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 3,
    .kind_1 = v1,
    .kind_2 = v2,
    .kind_3 = v3,
    .function.function_3 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_4(basic_function_type type, char* keyword, function_4 function, kind v1, kind v2, kind v3, kind v4)
{
  token t = register_token(keyword, keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 4,
    .kind_1 = v1,
    .kind_2 = v2,
    .kind_3 = v3,
    .kind_4 = v4,
    .function.function_4 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_5(basic_function_type type, char* keyword, function_5 function, kind v1, kind v2, kind v3, kind v4, kind v5)
{
  token t = register_token(keyword, keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 5,
    .kind_1 = v1,
    .kind_2 = v2,
    .kind_3 = v3,
    .kind_4 = v4,
    .kind_5 = v5,
    .function.function_5 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

//static bool
//is_basic_function_token(token sym)
//{
//  return sym >= TOKEN_TYPE_END + 1000;
//}
//
//static basic_function*
//get_basic_function(token sym)
//{
//  for(size_t i=0; i<array_size(basic_functions); i++)
//  {
//    basic_function* bf = (basic_function*) array_get(basic_functions, i);
//    if (bf->token == sym)
//    {
//      return bf;
//    }
//  }
//  return NULL;
//}

static basic_function*
find_basic_function_by_type(token sym, basic_function_type type)
{
  for(size_t i=0; i<array_size(basic_functions); i++)
  {
    basic_function* bf = (basic_function*) array_get(basic_functions, i);
    if (bf->type == type && bf->token == sym)
    {
      return bf;
    }
  }
  return NULL;
}



  static void
get_parameter(kind k, basic_type* v)
{

  // printf("get_parameter %d\n", k);

  if (k == kind_string)
  {
    // printf("go get string\n");
    char *s = string_expression();
    v->kind = kind_string;
    v->value.string = s;
  }
  else
  {
    // printf("go get numeric\n");
    float n = numeric_expression();
    v->kind = kind_numeric;
    v->value.number = n;
  }
}

static int
basic_dispatch_function(basic_function* function, basic_type* rv)
{
  basic_type v1, v2, v3, v4, v5;
  // printf("nr arguments: %ld\n", function->nr_arguments);
  switch (function->nr_arguments)
  {
    case 0:
      accept(sym);
      if (function->type != basic_function_type_keyword)
      {
        expect(T_LEFT_BANANA);
        expect(T_RIGHT_BANANA);
      }
      function->function.function_0(rv);
      break; 
    case 1:
      accept(sym);
      expect(T_LEFT_BANANA);
      get_parameter(function->kind_1, &v1);
      expect(T_RIGHT_BANANA);
      function->function.function_1(&v1, rv);
      break;
    case 2:
      accept(sym);
      expect(T_LEFT_BANANA);
      get_parameter(function->kind_1, &v1);
      expect(T_COMMA);
      get_parameter(function->kind_2, &v2);
      expect(T_RIGHT_BANANA);
      function->function.function_2(&v1, &v2, rv);
      break;
    case 3:
      accept(sym);
      expect(T_LEFT_BANANA);
      get_parameter(function->kind_1, &v1);
      expect(T_COMMA);
      get_parameter(function->kind_2, &v2);
      expect(T_COMMA);
      get_parameter(function->kind_3, &v3);
      expect(T_RIGHT_BANANA);
      function->function.function_3(&v1, &v2, &v3, rv);
      break;
    case 4:
      accept(sym);
      expect(T_LEFT_BANANA);
      get_parameter(function->kind_1, &v1);
      expect(T_COMMA);
      get_parameter(function->kind_2, &v2);
      expect(T_COMMA);
      get_parameter(function->kind_3, &v3);
      expect(T_COMMA);
      get_parameter(function->kind_4, &v4);
      expect(T_RIGHT_BANANA);
      function->function.function_4(&v1, &v2, &v3, &v4, rv);
      break;
    case 5:
      accept(sym);
      expect(T_LEFT_BANANA);
      get_parameter(function->kind_1, &v1);
      expect(T_COMMA);
      get_parameter(function->kind_2, &v2);
      expect(T_COMMA);
      get_parameter(function->kind_3, &v3);
      expect(T_COMMA);
      get_parameter(function->kind_4, &v4);
      expect(T_COMMA);
      get_parameter(function->kind_5, &v5);
      expect(T_RIGHT_BANANA);
      function->function.function_5(&v1, &v2, &v3, &v4, &v5, rv);
      break;
    default:
      error("Max nr vars exceeded");
      return -1;
  }
  return 0;
}

int
str_len(basic_type* str, basic_type* rv)
{
  // printf("In str_len: t:%d, v:'%s'\n", str->kind, str->value.string);
  rv->kind = kind_numeric;
  rv->value.number = (int) strlen(str->value.string);
  return 0;
}
