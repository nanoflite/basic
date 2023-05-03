/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Main module of the BASIC interpreter.
 *
 *---------------------------------------------------------------------------
 * line = [number] statement [ : statement ] CR
 *
 *  statement =
 *    CLEAR | CLR
 *    CONT
 *    DEF FN(X) = expression
 *    DELETE literal_string
 *    DIR
 *    DIM variable "(" expression ")"
 *    END
 *    FOR numeric_variable '=' numeric_expression TO numeric_expression [ STEP number ] 
 *    GOTO expression
 *    GOSUB expression
 *    IF relation-expression THEN statement
 *    INPUT variable-list
 *    LET variable = expression
 *    LIST
 *    LOAD literal_string
 *    NEW
 *    PRINT expression-list [ ; ]
 *    RETURN
 *    RUN
 *    SAVE literal_string
 *    STOP
 *
 *  expression-list = ( string | expression ) [, expression-list]
 *
 *  variable-list = variable [, variable-list]
 *
 *  expression = string_expression | numeric_expression
 *  
 *  numeric_expression = ["+"|"-"] term {("+"|"-"|"OR"|"EOR"|"XOR") term} .
 *
 *  term = factor {( "*" | "/" | "AND" ) factor} .
 *
 *  factor = 
 *    NOT factor
 *    func "(" expression ")" 
 *    number
 *    "(" expression ")"
 *    variable
 *    relation-expression
 *
 *  relation-expression =
 *    expression relation-operator expression
 *
 *  func =
 *    ABS
 *    ATN
 *    COS
 *    EXP
 *    INT
 *    LOG
 *    RND
 *    SGN
 *    SIN
 *    SQR
 *    TAN
 *
 *  string = literal_string | string_func "(" string_expression ")"
 *
 *  literal_string = '"' ... '"'
 *  
 *  string_func =
 *    CHR$
 *    ...
 *
 *  string_expression = literal_string | string_variable
 *
 *  variable = ( numeric_variable | string_variable | indexed_variable )
 *
 *  numeric_variable = A | B | C ... | X | Y | Z
 *
 *  string_variable = A$ | B$ | C$ ... | X$ | Y$ | Z$
 *
 *  indexed_variable = ( numeric_variable | string_variable ) "(" expression ")"
 *
 *  relation-operator = ( "<" | "<=" | "=" | ">=" | ">" )
 *---------------------------------------------------------------------------
 *
 * Version:	@(#)basic.c	1.1.0	2023/05/01
 *
 * Authors:	Fred N. van Kempen, <waltje@varcem.com>
 *		Johan Van den Brande <johan@vandenbrande.com>
 *
 *		Copyright 2023 Fred N. van Kempen.
 *		Copyright 2015,2016 Johan Van den Brande.
 *
 *		Redistribution and  use  in source  and binary forms, with
 *		or  without modification, are permitted  provided that the
 *		following conditions are met:
 *
 *		1. Redistributions of  source  code must retain the entire
 *		   above notice, this list of conditions and the following
 *		   disclaimer.
 *
 *		2. Redistributions in binary form must reproduce the above
 *		   copyright  notice,  this list  of  conditions  and  the
 *		   following disclaimer in  the documentation and/or other
 *		   materials provided with the distribution.
 *
 *		3. Neither the  name of the copyright holder nor the names
 *		   of  its  contributors may be used to endorse or promote
 *		   products  derived from  this  software without specific
 *		   prior written permission.
 *
 * THIS SOFTWARE  IS  PROVIDED BY THE  COPYRIGHT  HOLDERS AND CONTRIBUTORS
 * "AS IS" AND  ANY EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING, BUT  NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE  ARE  DISCLAIMED. IN  NO  EVENT  SHALL THE COPYRIGHT
 * HOLDER OR  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES;  LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON  ANY
 * THEORY OF  LIABILITY, WHETHER IN  CONTRACT, STRICT  LIABILITY, OR  TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING  IN ANY  WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "arch.h"
#include "basic.h"
#include "private.h"


typedef union {
    function_0 function;
    function_1 function_1;
    function_2 function_2;
    function_3 function_3;
    function_4 function_4;
    function_5 function_5;
} func_u;

typedef struct {
    token	token;
    func_type	type;
    int		nr_arguments : 3;		// max 8 arguments to function
    var_kind	kind_1 : 1;			// type of argument 1
    var_kind	kind_2 : 1;			// type of argument 2
    var_kind	kind_3 : 1;			// type of argument 3
    var_kind	kind_4 : 1;			// type of argument 4
    var_kind	kind_5 : 1;			// type of argument 5
    func_u	function; 			// pointer to handler
} function_t;


static array	*_tokens = NULL;
static array	*_functions = NULL;


error_t		__error;
ready_t		__ready;
putchar_t	__putch;
getchar_t	__getch;
getchar_t	__kbhit;

#ifdef _DEBUG
volatile bool	__DEBUG;
#endif
volatile bool	__RUNNING;
volatile bool	__EVALUATING;
volatile bool	__REPL;
volatile bool	__STOPPED;
volatile bool	__ERROR;

size_t		__program_size;
size_t		__memory_size;
char		*__memory;
size_t		__stack_size;
char		*__stack;			// current stack pointer
size_t		__stack_p;
uint16_t	__line,				// current program line
		__errline;			// line with the current error
char		*__cursor,			// current position on line
		*__cursor_saved;
token		sym;				// current input token
char		__dummy = 0;			// TBR


void
get_sym(void)
{
    sym = tokenizer_get_next_token();
//  printf("sym: %i\n", sym);
}


bool
accept(token t)
{
//  printf("accept %i\n", t);
    if (t == sym) {
	get_sym();
	return true;
    }
//  printf("accept got %i, expected %i\n", sym, t);

    return false;
}


bool
expect(token t)
{
//  printf("\nexpect %i, have %i\n", t, sym);
    if (accept(t))
	 return true;

    basic_error("UNEXPECTED SYMBOL");

    return false;
}


relop
get_relop(void)
{
    if (sym == T_LESS) {
	accept(T_LESS);
	if (sym == T_EQUALS) {
		accept(T_EQUALS);
		return OP_LE;
	} else if (sym == T_GREATER) {
		accept(T_GREATER);
		return OP_NE;
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


int
get_vector(int *vector, int size)
{
    int i, dims = 0;
    float n;

    for (i = 0; i < size; i++)
	vector[i] = 0;

    while (sym != T_RIGHT_BANANA) {
	n = numeric_expression();
	vector[dims] = (int)n;
	if (++dims > size) {
		basic_error("MAX DIM");
		return dims;
	}

	if (sym == T_COMMA)
		accept(T_COMMA);
    }

    return dims;
}


void
set_line(uint16_t line_number)
{
    char *cursor;

    __line = line_number;
    cursor = lines_get_contents(__line);

    tokenizer_char_pointer(cursor);
}


/* Skip our focus to the next statement on the current line. */
void
move_to_next_statement(void)
{
    while (sym != T_EOL && sym != T_COLON)
	get_sym();
}


/* Skip to the next line in the program. */
void
move_to_next_line(void)
{
    set_line(lines_next(__line));

    get_sym();
}


static void
get_parameter(var_kind k, basic_var *v)
{
    v->empty = false;
    v->mallocd = false;

    if (k == KIND_STRING) {
	v->kind = KIND_STRING;
	v->value.string = string_expression();
    } else {
	v->kind = KIND_NUMERIC;
	v->value.number = numeric_expression();
    }
}


static var_kind
function_kind_i(function_t *f, int i)
{
    switch (i) {
	case 0:
		return f->kind_1;

	case 1:
		return f->kind_2;

	case 2:
		return f->kind_3;

	case 3:
		return f->kind_4;

	case 4:
		return f->kind_5;

	default:
		return KIND_NUMERIC;
    }
}


/* Find and execute the function for the current token. */
int
dispatch_function(token tok, func_type type, basic_var *rv)
{
    basic_var v[6];	//FIXME: was 5, warning on overflow in get_param below
    function_t *f;
    size_t i;

    /* Find the function we need. */
    f = NULL;
    for (i = 0; i < array_size(_functions); i++) {
	f = (function_t *)array_get(_functions, i);
	if (f->type == type && f->token == tok)
		break;
	f = NULL;
    }

    /* If not found, give up. */
    if (f == NULL)
	return 1;

    if (f->nr_arguments > MAX_ARGUMENT) {
	basic_error("MAX ARGUMENTS");
	return -1;
    }

    /* Accept statement/function/command token. */
    accept(tok);

    /* Is this a statement without (declared) arguments? */
    if (f->nr_arguments == 0 /*&& f->type == FUNC_TYPE_KEYWORD*/) {
	/* Yep, just execute it. */
	f->function.function(rv);

	return 0;
    }

    /* Looks like a function, go bananas. */
    expect(T_LEFT_BANANA);

    for (i = 0; i < f->nr_arguments; i++){
	if (sym == T_RIGHT_BANANA)
		break;

	get_parameter(function_kind_i(f, i), &v[i]);
	if (i < f->nr_arguments-1) {
		if (sym != T_COMMA) {
			/* Probably just default parameters... */
			break;
		}  

		expect(T_COMMA);
	}
    }

    for (; i < f->nr_arguments; i++) {
	v[i].empty = true;
	v[i].mallocd = false;
	v[i].kind = KIND_NUMERIC;
    }

    /* All done with the fruit. */
    expect(T_RIGHT_BANANA);

    rv->mallocd = false;
    switch (f->nr_arguments) {
	case 0:
		f->function.function(rv);
		break;

	case 1:
		f->function.function_1(rv, &v[0]);
		break;

	case 2:
		f->function.function_2(rv, &v[0], &v[1]);
		break;

	case 3:
		f->function.function_3(rv, &v[0], &v[1], &v[2]);
		break;

	case 4:
		f->function.function_4(rv, &v[0], &v[1], &v[2], &v[3]);
		break;

	case 5:
		f->function.function_5(rv, &v[0], &v[1], &v[2], &v[3], &v[4]);
		break;

	default:
		return -1;
    }

    for (i = 0; i < f->nr_arguments; i++) {
	if (v[i].kind == KIND_STRING)
		free(v[i].value.string);
    }

    return 0;
}


/* Process the current line. */
void
parse_line(void)
{
    while (sym != T_EOL) {
	/* Handle a single statement. */
	if (! do_stmt())
		break;
    }
}


/*
 * Start executing lines of statements, beginning with the current
 * line number (in __line) and ending with the last line in memory.
 */
void
basic_run(void)
{
    basic_error(NULL);

    if (__STOPPED) {
	__cursor = __cursor_saved;
	__cursor_saved = NULL;
    } else
	__cursor = lines_get_contents(__line);

    __STOPPED = false;
    __RUNNING = false;
    __EVALUATING = false;

    if (__cursor != NULL) {
	/* Point the tokenizer at the current cursor. */
	tokenizer_init(__cursor);

	__RUNNING = true;

	while (__cursor && __RUNNING /* && !__ERROR */) {
		/* Get the next token to process. */
		get_sym();

		/* End of line, see if we can move to the next one. */
		if (sym == T_EOL) {
			__line = lines_next(__line);
			__cursor = lines_get_contents(__line);
			if (__cursor == NULL) {
				__RUNNING = false;
				break;
			}
			tokenizer_init(__cursor);
		}

		parse_line();
	}
    }
}


/* Something bad happened. */
void
basic_error(const char *msg)
{
    if (msg != NULL) {
	__ERROR = true;
	__errline = __line;

	__error(msg);
    } else {
	/* Nah, just clearing error status! */
	__ERROR = false;
	__errline = 0;
    }
}


/* Interpret a single ASCII input line. */
void
basic_eval(const char *line)
{
    char temp[BASIC_STR_LEN];
    uint16_t line_number;
    char *str;

    basic_error(NULL);

    /* Skip any leading whitespace. */
    while (isspace(*line))
	line++;

    if (strlen(line) > 0 && (strlen(line)-1) > BASIC_STR_LEN) {
	basic_error("LINE TOO LONG");
	return;
    }

    if (is_empty(line) || is_comment(line))
	return;

    strncpy(temp, line, sizeof(temp) - 1);
    _trim(temp);

    tokenizer_init(temp);

    get_sym();
    if (sym == T_NUMBER) {
	line_number = (uint16_t)tokenizer_get_number();
	str = tokenizer_char_pointer(NULL);

	get_sym();
	if (sym == T_EOL)
		lines_delete(line_number);
	else
		lines_store(line_number, str);
    } else {
	__RUNNING = false;
	__EVALUATING = true;

	while (__EVALUATING) {
		parse_line();

		accept(sym);
		if (sym == T_EOL || sym == T_ERROR)
			__EVALUATING = false;
	}

	__EVALUATING = false;
    }
}


void
basic_ready(const char *msg)
{
    if (__REPL)
	__ready(msg);
}


void
basic_destroy(void)
{
    vars_destroy();
    tokenizer_free_registered_tokens();
    array_destroy(_tokens);
    array_destroy(_functions);

    free(__stack);
    free(__memory);
}


void
basic_init(size_t memory_size, size_t stack_size)
{
    __memory_size = memory_size;
    __memory = malloc(__memory_size);
    if (__memory == NULL) {
	basic_error("CANNOT ALLOCATE PROGRAM SPACE");
	return;
    }
    memset(__memory, 0x00, memory_size);
    __program_size = __memory_size;

    __stack_size = stack_size;
    __stack = malloc(__stack_size);
    if (__stack == NULL) {
	basic_error("CANNOT ALLOCATE STACK SPACE");
	return;
    }
    memset(__stack, 0x00, __stack_size);
    __stack_p = __stack_size;

    lines_init();

    vars_init();

    tokenizer_setup();

    _tokens = array_new(sizeof(token_entry));
    _functions = array_new(sizeof(function_t));

    stmt_init();

    func_init();

    arch_init();
}


void
basic_register(error_t err, ready_t rdy,
	       putchar_t pc, getchar_t gc, getchar_t kbh)
{
    __error = err;
    __ready = rdy;
    __putch = pc;
    __getch = gc;
    __kbhit = kbh;

    __RUNNING = false;
    __EVALUATING = false;
    __REPL = true;
    __STOPPED = false;
    __ERROR = false;

    __line = __errline = 0;
}


void
basic_stop(void)
{
    char temp[64];

    if (__RUNNING || __EVALUATING) {
	__RUNNING = false;
	__EVALUATING = false;
	__STOPPED = true;

	__cursor_saved = tokenizer_char_pointer(NULL);
    }

    sprintf(temp, "\n*** BREAK on line %i", __line);
    basic_ready(temp);
}


void
basic_start(void)
{
    __REPL = false;

    basic_eval("RUN");
}


//**************************************************************************
// - Register functions

token
register_token(token t, const char *name)
{
    token_entry tok;

    tok.name = name;
    tok.token = t;
    t = tokenizer_register_token(&tok);

    return t;
}


token
register_function(func_type type, const char *keyword, function_0 f)
{
    token t = register_token(T_AUTO, keyword);
    function_t bf = {
	.token = t,
	.type = type,
	.nr_arguments = 0,
	.function.function = f
    };

    array_push(_functions, &bf);

    return t;
}


token
register_function_1(func_type type, const char *keyword,
		    function_1 f, var_kind v1)
{
    token t = register_token(T_AUTO, keyword);
    function_t bf = {
	.token = t,
	.type = type,
	.nr_arguments = 1,
	.kind_1 = v1,
	.function.function_1 = f
    };

    array_push(_functions, &bf);

    return t;
}


token
register_function_2(func_type type, const char *keyword,
		    function_2 f, var_kind v1, var_kind v2)
{
    token t = register_token(T_AUTO, keyword);
    function_t bf = {
	.token = t,
	.type = type,
	.nr_arguments = 2,
	.kind_1 = v1,
	.kind_2 = v2,
	.function.function_2 = f
    };

    array_push(_functions, &bf);

    return t;
}


token
register_function_3(func_type type, const char *keyword,
		    function_3 f, var_kind v1, var_kind v2, var_kind v3)
{
    token t = register_token(T_AUTO, keyword);
    function_t bf = {
	.token = t,
	.type = type,
	.nr_arguments = 3,
	.kind_1 = v1,
	.kind_2 = v2,
	.kind_3 = v3,
	.function.function_3 = f
    };

    array_push(_functions, &bf);

    return t;
}


token
register_function_4(func_type type, const char *keyword,
		    function_4 f, var_kind v1, var_kind v2, var_kind v3, var_kind v4)
{
    token t = register_token(T_AUTO, keyword);
    function_t bf = {
	.token = t,
	.type = type,
	.nr_arguments = 4,
	.kind_1 = v1,
	.kind_2 = v2,
	.kind_3 = v3,
	.kind_4 = v4,
	.function.function_4 = f
    };

    array_push(_functions, &bf);

    return t;
}


token
register_function_5(func_type type, const char *keyword,
		    function_5 f, var_kind v1, var_kind v2, var_kind v3, var_kind v4, var_kind v5)
{
    token t = register_token(T_AUTO, keyword);
    function_t bf = {
	.token = t,
	.type = type,
	.nr_arguments = 5,
	.kind_1 = v1,
	.kind_2 = v2,
	.kind_3 = v3,
	.kind_4 = v4,
	.kind_5 = v5,
	.function.function_5 = f
    };

    array_push(_functions, &bf);

    return t;
}
