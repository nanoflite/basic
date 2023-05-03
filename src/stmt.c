/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Implement (most of) the BASIC statements.
 *
 * Version:	@(#)stmt.c	1.1.0	2023/05/01
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
#include <time.h>
#include "arch.h"
#include "basic.h"
#include "private.h"


#define MAX_LINE	BASIC_STR_LEN


typedef enum {
    DATA_STATE_INIT,
    DATA_STATE_FIND,
    DATA_STATE_READ
} data_state;

typedef struct {
    uint16_t	line;
    char	*cursor;
    char	*char_pointer;
    data_state	state;
} data_ptr;

typedef enum {
    FRAME_TYPE_FOR,
    FRAME_TYPE_GOSUB
} frame_type;

typedef struct {
    frame_type	type;
    size_t	line;
    char	*cursor; 
    char	var_name[BASIC_VAR_LEN]; 
    float	end_value;
    float	step;
} stack_frame_for;

typedef struct {
    frame_type	type;
    size_t	line;
    char	*cursor;
} stack_frame_gosub;


static token	t_kw_data;
static token	t_kw_for;
static token	t_kw_gosub;
static token	t_kw_goto;
static token	t_kw_if;
static token	t_kw_next;
static token	t_kw_on;
static token	t_kw_rem;
static token	t_kw_step;
static token	t_kw_then;
static token	t_kw_to;
#if 0
static token	t_kw_def;
static token	t_kw_fn;
#endif

static data_ptr	__data;


static void	do_print_expr(const expr_result *);


/* Perform the "CLEAR" command. */
static int
do_clear(basic_var *rv)
{
    vars_destroy();
    vars_init();

    basic_ready(NULL);

    return 0;
}


/* Perform the "CLS" statement. */
static int
do_cls(basic_var *rv)
{
#if PLATFORM == PLATFORM_XMEGA
    basic_io_print("\x1b");
    basic_io_print("E");
#endif

#ifdef _WIN32
    basic_io_print("\014");
#else
    basic_io_print("\033[2J");
    basic_io_print("\033[0;0H");
#endif

    rv->kind = KIND_NUMERIC;
    rv->value.number = 0;

    return 0;
}


/* Perform the "CONT" command. */
static int
do_cont(basic_var* rv)
{
    if (__STOPPED)
	basic_run();
    else
	basic_error("PROGRAM WAS NOT STOPPED");

    basic_ready(NULL);

    return 0; 
}


/* Perform the "DATA val[,val ...]" statement. */
static int
do_data(basic_var *rv)
{
    move_to_next_statement();

    return 0;
}


#if 0
static int
do_def_fn(basic_var *rv)
{
    if (sym != t_kw_fn) {
	basic_error("EXPECTED FN");
	return 0;
    }

    // Find 'X' between '(', ')'.
  
    // Associate 'X' with the location of the expression (string pointer).
    // When 'evalled', use the string pointer to run the expression valuator.

    return 0;
}  
#endif


/* Perform the "DELETE "filename"" command. */
static int
do_delete(basic_var *rv)
{
    char *filename;

    if (sym != T_STRING) {
	basic_error("EXPECTED LITERAL STRING");
	return 0;
    }

    filename = tokenizer_get_string();
    accept(T_STRING);
    arch_delete(filename);

    basic_ready(NULL);

    return 0;
}


/* Perform the "DIM var(dim, dim, ..)" statement. */
static int
do_dim(basic_var *rv)
{
    char name[BASIC_VAR_LEN+1];
    int vector[MAX_VECTOR];
    int l, dims;

    while (sym != T_EOL && sym != T_COLON) {
	if (sym == T_VARIABLE_NUMBER || sym == T_VARIABLE_STRING) {
		var_type type = (sym == T_VARIABLE_STRING) ? VAR_TYPE_STRING : VAR_TYPE_NUMERIC ;
		tokenizer_get_variable_name(name);

		l = (int)strlen(name);
		name[l] = '(';
		name[l+1] = '\0';
		accept(sym);
		expect(T_LEFT_BANANA); 
		dims = get_vector(vector, MAX_VECTOR);
		expect(T_RIGHT_BANANA);

		vars_array_init(name, type, dims, vector);
	}

	if (sym == T_COMMA)
		accept(T_COMMA);
    }

    return 0;
}


static void
do_dir_cb(const char *name, size_t size, bool label, void *context)
{
    if (label) {
	printf("-- %-13s --\n", name);
    } else {
#if PLATFORM == PLATFORM_XMEGA
	printf("> %-8s : %6d\n", name, size);
#else
	printf("> %-8s : %6lu\n", name, (unsigned long)size);
#endif
    }
}  


/* Perform the "DIR" command. */
static int
do_dir(basic_var *rv)
{
    arch_dir(do_dir_cb, NULL);

    basic_ready(NULL);

    return 0;
}


#ifdef _DEBUG
static void
do_dump_cb(const variable *var, void *context)
{
    vars_dump(var);

    basic_ready(NULL);
}


/* Perform the "DUMP" debug command. */
static int
do_dump(basic_var *rv)
{
    vars_each(do_dump_cb, NULL);

    return 0;
}
#endif


/* Perform the "END" statement. */
static int
do_end(basic_var *rv)
{
    __RUNNING = false;

    return 0;
}


/* Perform the "FOR var = {val} TO {val} [STEP {val}" statement. */
static int
do_for(basic_var *rv)
{
    char name[BASIC_VAR_LEN];
    float value, end_value, step;
    stack_frame_for *f;

    if (sym != T_VARIABLE_NUMBER) {
	basic_error("EXPECTED VAR");
	return 0;
    }
    tokenizer_get_variable_name(name);

    get_sym();
    expect(T_EQUALS);
    value = numeric_expression();
    vars_set_numeric(name, value);

    expect(t_kw_to);
    end_value = numeric_expression();

    step = 1.0;
    if (sym != T_EOL && sym != T_COLON) {
	expect(t_kw_step);
	step = numeric_expression();
    }  

    f = (stack_frame_for *)&(__stack[__stack_p]);
    if (f->type == FRAME_TYPE_FOR && !strcmp(name, f->var_name)) {
	// overwrite the current loop for the same variable
    } else {
	if (__stack_p < sizeof(stack_frame_for)) {
		basic_error("STACK FULL");
		return 0;
	}  

	__stack_p -= sizeof(stack_frame_for);
	f = (stack_frame_for *) &(__stack[__stack_p]);
	f->type = FRAME_TYPE_FOR;
	strncpy(f->var_name, name, BASIC_VAR_LEN);
    }

//  printf("FOR %s %i newp=%i\n", name, (int)__line, (int)__stack_p);
    f->end_value = end_value;
    f->step = step;
    f->line = __line;
    f->cursor = tokenizer_char_pointer(NULL); 

    return 0;
}


/* Perform the "GET strvar" statement. */
static int
do_get(basic_var *rv)
{
    char name[BASIC_VAR_LEN+1];
    char c[4] = "";
    int ch;

    if (sym != T_VARIABLE_STRING) {
	basic_error("EXPECTED STRING VAR");
	return 0;
    }
    tokenizer_get_variable_name(name);

    accept(T_VARIABLE_STRING);

    if (__kbhit && __kbhit()) {
	ch = __getch();
	if (ch == 10)
		ch = 13;
	snprintf(c, sizeof(c), "%c", ch);
    }

    vars_set_string(name, c);

    return 0;
}


/* Perform the "GOSUB {line}" statement. */
static int
do_gosub(basic_var *rv)
{
    stack_frame_gosub *g;
    int line_number;

    if (sym != T_NUMBER) {
	basic_error("EXPECTED NUMBER");
	return 0;
    }

    line_number = (int)tokenizer_get_number();
    accept(T_NUMBER);

    if (__stack_p < sizeof(stack_frame_gosub)) {
	basic_error("STACK FULL");
	return 0;
    }

    __stack_p -= sizeof(stack_frame_gosub);
    g = (stack_frame_gosub *) &(__stack[__stack_p]);
//  printf("GOSUB stack line=%i _p=%d\n", (int)__line, (int)__stack_p);
    g->type = FRAME_TYPE_GOSUB;
    g->line = __line;
    g->cursor = tokenizer_char_pointer(NULL); 

    set_line(line_number);

    return 0;
}


/* Perform the "GOTO {line}" statement. */
static int
do_goto(basic_var *rv)
{
    int line_number;

    if (sym != T_NUMBER) {
	basic_error("GOTO EXPECTED NUMBER");
	return 0;
    }

    line_number = (int)tokenizer_get_number();
    accept(T_NUMBER);

    if (lines_get_contents(line_number) == NULL) {
	basic_error("GOTO LINE NOT FOUND");
	return 0;
    }

    set_line(line_number);

    return 0;
}


static bool
if_expression(void)
{
    expr_result left, right;
    bool result;
    relop op;

    expression(&left);

    if (left.type == EXPR_TYPE_STRING) {
//FIXME: why only on strings?
	op = get_relop();
	expression(&right);
	result = condition(&left, &right, op);
    } else
	result = left.value.numeric == 1.0;

    return result;
}


static int
do_if(basic_var *rv)
{
    bool result, next;

    result = if_expression();

    // No priorities, just parse the operators
    while (sym == T_AND || sym == T_OR) {
	get_sym();
	next = if_expression();
	if (sym == T_AND)
		result = result && next;
	else
		result = result || next;
    }

    if (sym != t_kw_then) {
	basic_error("IF WITHOUT THEN");
	return 0;
    } 
  
    //FIXME: should we not accept(t_kw_then) here?

    if (result) {
	get_sym();	//FIXME: see above, this can then go

	if (sym == T_NUMBER) {
		float line_number = tokenizer_get_number();
		accept(T_NUMBER);
		set_line( (uint16_t)line_number );
	} else 
		parse_line();
    } else {
//FIXME: set expect for ELSE here?
	move_to_next_line();
    }

    return 0;
}


/* Perform the "INPUT [{promprstr},] varstr" statement. */
static int
do_input(basic_var *rv)
{
    char name[BASIC_VAR_LEN+1];
    char line[MAX_LINE];
    bool prompt = false;
    expr_result expr;
    char *t, *p;
    float value;

    if (sym != T_VARIABLE_NUMBER && sym != T_VARIABLE_STRING) {
	expression(&expr);
	if (sym == T_COMMA || sym == T_SEMICOLON)
		get_sym();
	else
		basic_error("UNEXPECTED TOKEN");

	prompt = true;
    }

    if (sym != T_VARIABLE_NUMBER && sym != T_VARIABLE_STRING) {
	basic_error("EXPECTED VAR");
	return 0;
    }

    token type = sym;
    if (type == T_VARIABLE_NUMBER) {
	tokenizer_get_variable_name(name);
	accept(T_VARIABLE_NUMBER);
    }

    if (type == T_VARIABLE_STRING) {
	tokenizer_get_variable_name(name);
	accept(T_VARIABLE_STRING);
    }

    if (prompt) {
	do_print_expr(&expr);
	if (expr.type == EXPR_TYPE_STRING) {
		free(expr.value.string);
	}
    }

//FIXME: "prompt" is the optional first arg of the INPUT statement!?!?
    basic_io_readline((prompt ? " " : "? "), line, sizeof(line)); 

    if (type == T_VARIABLE_NUMBER) {
	p = line;
	value = strtof(line, &t);
	vars_set_numeric(name, value);

	while (sym == T_COMMA) {
		// find ',' in input
		while (*p && *p != ',' && *p != ' ')
			++p;
		if (!*p)
			basic_error("EXPECTED COMMA IN INPUT");
		++p;

		get_sym();
		if (type == T_VARIABLE_NUMBER) {
			tokenizer_get_variable_name(name);
			accept(T_VARIABLE_NUMBER);
		} else
			basic_error("EXPECTED NUMERIC VARIABLE");
		value = strtof(p, &t);
		vars_set_numeric(name, value);
	}
    }

    if (type == T_VARIABLE_STRING)
	vars_set_string(name, line);

    return 0;
}


/* Perform the "[LET] var = {value}" statement. */
static int
do_let(basic_var *rv)
{
    char name[BASIC_VAR_LEN+1];
    int vector[MAX_VECTOR];
    bool is_array = false;
    token type;
    int l;

    if (sym != T_VARIABLE_NUMBER && sym != T_VARIABLE_STRING) {
	basic_error("EXPECTED VAR");
	return 0;
    }

    tokenizer_get_variable_name(name);
    type = sym;
    get_sym();
    if (sym == T_LEFT_BANANA) {
	is_array = true;
	l = (int)strlen(name);
	name[l] = '(';
	name[l+1] = '\0';
	accept(T_LEFT_BANANA);
	get_vector(vector, MAX_VECTOR);
	expect(T_RIGHT_BANANA);
    }

    expect(T_EQUALS);
  
    if (type == T_VARIABLE_NUMBER) {
	float value = numeric_expression();

	if (is_array)
		vars_array_set_numeric(name, value, vector);
	else
		vars_set_numeric(name, value);
    }

    if (type == T_VARIABLE_STRING) {
	char *value = string_expression();

	if (is_array)
		vars_array_set_string(name, value, vector);
	else
		vars_set_string(name, value);

	free(value);
    }

    return 0;
}


int
is_comment(const char *s)
{
    return *s == '#';
}


int
is_empty(const char *s)
{
    while (*s != '\0') {
	if (! isspace(*s))
		return 0;
	s++;
    }

    return 1;
}


// Insert '\0' behind first non space character, starting from right.
void
_trim(char *s)
{
    char *p = s + strlen(s) - 1;	// without the '\0'

    while (p >= s && isspace(*p))
	--p;

    *(p+1) = '\0'; 
}  


static void
do_load_cb(const char *line, void *context)
{
    char temp[BASIC_STR_LEN];
    uint16_t line_number;
    char *p = temp;
    int i;

    /* Skip leading whitespace. */
    while (isspace(*line))
	line++;

    /* Skip these. */
    if (is_empty(line) || is_comment(line))
        return;

    if (strlen(line) > 0 && (strlen(line)-1) > BASIC_STR_LEN) {
        basic_error("LINE TOO LONG");
        return;
    }

    strncpy(p, line, sizeof(temp) - 1);

    i = 0;
    while (isdigit(*p))
	p++;  
    sscanf(temp, "%i", &i);
    line_number = (uint16_t)i;

    while (isspace(*p))
	p++;

    /* Trim off any trailing whitespace. */
    _trim(p);

    lines_store(line_number, p);
}  


/* Perform the "LOAD "filename"" command. */
static int
do_load(basic_var *rv)
{
    char *filename;

    if (sym != T_STRING) {
	basic_error("EXPECTED LITERAL STRING");
	return 0;
    }

    filename = tokenizer_get_string();
    accept(T_STRING);

    lines_clear();

    arch_load(filename, do_load_cb, NULL);

    basic_ready(NULL);

    return 0;
}


/* Perform the "NEW" command. */
static int
do_new(basic_var *rv)
{
    lines_clear();

    vars_destroy();
    vars_init();

    basic_ready(NULL);

    return 0;
}


/* Perform the "NEXT [var]" statement. */
static int
do_next(basic_var *rv)
{
    char var_name[BASIC_VAR_LEN];
    stack_frame_for *f;
    float value;

    f = (stack_frame_for*) &(__stack[__stack_p]);
    if (f->type != FRAME_TYPE_FOR) {
	basic_error("EXPECTED FOR STACK FRAME");
	return 0;
    }

    if (sym == T_VARIABLE_NUMBER) {
	tokenizer_get_variable_name(var_name);
	accept(T_VARIABLE_NUMBER);
	//printf("NEXT var=%s stack=%s p=%d\n", var_name, f->var_name, (int)__stack_p);
	if (strcmp(var_name, f->var_name) != 0) {
		char _error[40];
		snprintf(_error, sizeof(_error), "EXPECTED NEXT WITH %s, GOT %s", var_name, f->var_name);
		basic_error(_error);
		return 0;
	}
    }

    // check end condition 
    value = vars_get_numeric(f->var_name) + f->step;
    if ((f->step > 0 && value > f->end_value) || (f->step < 0 && value < f->end_value)) {
	__stack_p += sizeof(stack_frame_for);
	return 0;
    }

    vars_set_numeric(f->var_name, value); 

    __line = (uint16_t)f->line;
    tokenizer_char_pointer(f->cursor);

    return 0;
}


static size_t
get_list(size_t *list, size_t max_size)
{
    size_t size = 0;
    float n;

    do {  
	if (sym == T_COMMA)
		accept(T_COMMA);

	n = numeric_expression();
	list[size++] = (size_t)n;
	if (size > max_size) {
		basic_error("LIST MAX SIZE");
		return size;
	}
    } while (sym == T_COMMA);

    return size;
}


/* Perform the "ON {index} [GOSUB | GOTO] {line}" statement. */
static int
do_on_goto(basic_var *rv)
{
    size_t list[20], size;
    uint16_t line_number;
    stack_frame_gosub *g;
    int index;

    index = (int)numeric_expression();

    token what = T_EOL;
    if (sym == t_kw_goto){
	what = t_kw_goto;
    } else if (sym == t_kw_gosub){
	what = t_kw_gosub;
    } else {
	basic_error("ON WITHOUT GOTO OR GOSUB");
	return 0;
    }
    accept(what);

    size = get_list(list, 20);
    if (index > (int)size){
	basic_error("ON OUT OF BOUNDS");
	return 0;
    }

    line_number = (uint16_t)list[index-1];  
    if (what == t_kw_goto){
	//TODO: refactor to helper and use in goto as well
	if (lines_get_contents(line_number) == NULL)
		basic_error("LINE NOT FOUND");
    } else {
	//TODO: refactor to helper and use in gosub as well
	if (__stack_p < sizeof(stack_frame_gosub)) {
		basic_error("STACK FULL");
		return 0;
	}

	__stack_p -= sizeof(stack_frame_gosub);
	g = (stack_frame_gosub *) &(__stack[__stack_p]);

	g->type = FRAME_TYPE_GOSUB;
	g->line = __line;
	g->cursor = tokenizer_char_pointer(NULL); 
    }

    set_line(line_number);

    return 0;
}


static void
do_print_expr(const expr_result *expr)
{
    char buffer[16];
    float val;
    long ival;

    if (expr->type == EXPR_TYPE_STRING) {
        basic_io_print(expr->value.string);
    } else if (expr->type == EXPR_TYPE_NUMERIC) {
        val = expr->value.numeric;
        ival = (long)val;
        if (ival == val) {
                snprintf(buffer, sizeof(buffer), "%li", ival);
                basic_io_print(buffer);
        } else {
                snprintf(buffer, sizeof(buffer), "%f", val);
                basic_io_print(buffer);
        }
    } else
        basic_error("UNKNOWN EXPRESSION");
}


/* Perform the "PRINT" or "?" statements. */
static int
do_print(basic_var *rv)
{
    expr_result expr;

    if (sym == T_EOL || sym == T_COLON) {
	/* Just an empty print statement. */
	__putch('\n');
    } else {
	while (sym != T_EOL && sym != T_COLON) {
		if (! dispatch_function(sym, FUNC_PRINT, rv)) {
			/* Already done. */
		} else if (sym != T_COMMA && sym != T_SEMICOLON) {
			expression(&expr);

			do_print_expr(&expr);

			if (expr.type == EXPR_TYPE_STRING) {
				if (expr.value.string != &__dummy)
					free(expr.value.string);
			}
		}

		if (sym == T_COMMA) {
			accept(T_COMMA);
			__putch('\t');
		} else if (sym == T_SEMICOLON) {
			accept(T_SEMICOLON);
		} else
			__putch('\n');
	}
    }

    fflush(stdout);

    return 0;
}


static void
do_list_cb(uint16_t number, const char *contents)
{
    char buffer[BASIC_STR_LEN];

    snprintf(buffer, sizeof(buffer), "%i %s\n", number, contents);
    basic_io_print(buffer);
}


/* Perform the LIST command. */
static int
do_list(basic_var *rv)
{
    uint16_t start = 0;
    uint16_t end = 0;

    if (sym == T_NUMBER) {
	start = (uint16_t) tokenizer_get_number();
	accept(T_NUMBER);
	if (sym == T_MINUS) {
		accept(T_MINUS);
		if (sym == T_NUMBER) {
			end = (uint16_t) tokenizer_get_number();
			accept(T_NUMBER);
		}
	}
    }

    lines_list(start, end, do_list_cb);

    basic_ready(NULL);

    return 0;
}


static float
_data_number(void)
{
    if (sym == T_MINUS) {
        get_sym();

        return -tokenizer_get_number();
    }

    return tokenizer_get_number();
}


static bool
_data_find(var_type type, value *value)
{
//  printf("data find\n");
    tokenizer_init(__data.cursor);
    tokenizer_char_pointer(__data.char_pointer);

    while (__data.cursor) {
	get_sym();
	while (sym != T_EOL) {
		if (sym == t_kw_rem) {
			//FIXME: why? Cant we just skip over it??
			break;
		}

		if (sym == t_kw_data) {
			accept(sym);
			if (type == VAR_TYPE_STRING)
				value->string = tokenizer_get_string();
			else
				value->number = _data_number();

			__data.state = DATA_STATE_READ;
			__data.char_pointer = tokenizer_char_pointer(NULL);

			return true;
		}

		get_sym();
	}

	__data.line = lines_next(__data.line);
	__data.cursor = lines_get_contents(__data.line);
	tokenizer_init(__data.cursor);
    }

    return false;
}  


static bool
_data_read(var_type type, value *value)
{
    bool rv = false;
//  printf("data read\n");

    tokenizer_init(__data.cursor);
    tokenizer_char_pointer(__data.char_pointer);

    get_sym();
    if (sym != T_EOL) {
	accept(T_COMMA);	// separated by commas
	if (type == VAR_TYPE_STRING) {
		value->string = tokenizer_get_string();
	} else {
		value->number = _data_number();
	}

	__data.char_pointer = tokenizer_char_pointer(NULL);
	rv = true; 
    } else
	__data.cursor = lines_get_contents(__data.line);

    return rv;
}  


static bool
_do_data_read(var_type type, value *value)
{
    token save_sym = sym;
    char *save_pointer = tokenizer_char_pointer(NULL);
    bool found, rv = false;

    switch (__data.state) {
	case DATA_STATE_INIT:
		__data.line = lines_first();
		__data.cursor = lines_get_contents(__data.line);
		__data.char_pointer = tokenizer_char_pointer(NULL);
		__data.state = DATA_STATE_FIND;
		/*FALLTHROUGH*/

	case DATA_STATE_FIND:  
		rv = _data_find(type, value);
		break;

	case DATA_STATE_READ:
		found = _data_read(type, value);
		if (found)
			rv = true;
		else
			rv = _data_find(type, value);
    }

    tokenizer_init(__cursor);
    sym = save_sym;
    tokenizer_char_pointer(save_pointer);

    return rv;
}


/* Perform the "READ var" statement. */
static int
do_read(basic_var *rv)
{
    char name[BASIC_VAR_LEN+1];
    bool is_array = false;
    int vector[MAX_VECTOR];
    value v;
    int l;

    // if not initialized data_ptr, find first data statement
    // while not end of variable list
    //  read data, put in variable
    //  proceed to next data statement
    while (sym != T_EOL && sym != T_COLON) {
	if (sym == T_VARIABLE_NUMBER || sym == T_VARIABLE_STRING) {
		var_type type = (sym == T_VARIABLE_STRING) ? VAR_TYPE_STRING : VAR_TYPE_NUMERIC;
		tokenizer_get_variable_name(name);

		accept(sym);
		if (sym == T_LEFT_BANANA) {
			is_array = true;
			l = (int)strlen(name);
			name[l] = '(';
			name[l+1] = '\0';
			accept(T_LEFT_BANANA);
			get_vector(vector, MAX_VECTOR);
			expect(T_RIGHT_BANANA);
		}
//		printf("do_read variable name: %s\n", name);

		bool read_ok = _do_data_read(type, &v);
//		printf("READ %f\n", v.number);
		if (! read_ok) {
			basic_error("READ WITHOUT DATA");
			return 0;
		}
		if (type == VAR_TYPE_STRING) {
			if (is_array)
				vars_array_set_string(name, v.string, vector);
			else  
				vars_set_string(name, v.string);
		} else {
			if (is_array)
				vars_array_set_numeric(name, v.number, vector);
			else
				vars_set_numeric(name, v.number);
		}
	}

	//get_sym();
	accept(T_COMMA);
    }

    return 0;
}


/* Perform the "REM" statement. */
static int
do_rem(basic_var *rv)
{
    set_line(lines_next(__line));

    get_sym();

    return 0;
}


/* Perform the "RETURN" statement. */
static int
do_return(basic_var *rv)
{
    stack_frame_gosub *g;
    frame_type t;

    // skip unfinished fors
    t = *(frame_type *)&(__stack[__stack_p]);
    while (t == FRAME_TYPE_FOR) {
	__stack_p += sizeof(stack_frame_for);
	t = *(frame_type *)&(__stack[__stack_p]);
    }

    g = (stack_frame_gosub *)&(__stack[__stack_p]);

//  printf("RETURN stack line=%i _p=%i\n", (int)__line, (int)__stack_p);
    if (g->type != FRAME_TYPE_GOSUB) {
	basic_error("EXPECTED GOSUB STACK FRAME");
	return 0;
    }

    __line = (uint16_t)g->line;
    tokenizer_char_pointer(g->cursor);

    __stack_p += sizeof(stack_frame_gosub);

    return 0;
}


/* Perform the "RESTORE" statement. */
static int
do_restore(basic_var *rv)
{
    __data.line = 0;
    __data.cursor = 0;
    __data.state = DATA_STATE_INIT;

    return 0;
}


/* Perform the "RUN [line]" command. */
static int
do_run(basic_var* rv)
{
    if (sym == T_NUMBER) {
	__line = (uint16_t)tokenizer_get_number();
	accept(T_NUMBER);
    } else
	__line = lines_first();

    /* Clear all variables. */
    vars_destroy();
    vars_init();

    /* Run the program. */
    basic_run();
 
    basic_ready(NULL); 

    return 0; 
}


typedef struct {
    uint16_t number;
} _save_cb_ctx;


static uint16_t 
do_save_cb(char **line, void *context)
{
    _save_cb_ctx *ctx = (_save_cb_ctx *)context;
    uint16_t number = ctx->number;
    ctx->number = lines_next(number);
   
    *line = lines_get_contents(number);

    return number;
}  


/* Perform the "SAVE "filename"" command. */
static int
do_save(basic_var *rv)
{
    char *filename;

    if (sym != T_STRING) {
	basic_error("EXPECTED LITERAL STRING");
	return 0;
    }

    filename = tokenizer_get_string();
    accept(T_STRING);

    _save_cb_ctx ctx;
    ctx.number = lines_first();

    arch_save(filename, do_save_cb, &ctx);

    basic_ready(NULL);

    return 0;
}


/* Perform the "SLEEP {msec}" statement. */
static int
do_sleep(basic_var *rv, basic_var *delay)
{
    int milliseconds = (int)delay->value.number;
 
    arch_sleep(milliseconds);

    rv->kind = KIND_NUMERIC;
    rv->value.number = 0;

    return 0;
}


/* Perform the SPC() statement. */
static int
do_spc(basic_var *rv, basic_var *n)
{
    for (size_t i = 0; i < n->value.number; i++)
	__putch(' ');
 
    rv->kind = KIND_NUMERIC;
    rv->value.number = 0;

    return 0;
}


/* Perform the "STOP" statement. */
static int
do_stop(basic_var *rv)
{
    /* Skip past the STOP statement. */
    move_to_next_statement();

    /* Signal the stop (so we can CONTinue later.) */
    basic_stop();

    basic_ready(NULL);

    return 0;
}


/* Perform the TAB() statement. */
static int
do_tab(basic_var *rv, basic_var *n)
{
    for (size_t i = 0;  i <n->value.number; i++)
	__putch(' ');

    rv->kind = KIND_NUMERIC;
    rv->value.number = 0;

    return 0;
}


/* Register the various BASIC statements. */
void
stmt_init(void)
{
    __data.line = 0;
    __data.cursor = 0;
    __data.state = DATA_STATE_INIT;

    /* BASIC operators. */
    register_token(T_AND, "AND");
    register_token(T_EOR, "EOR");
    register_token(T_XOR, "XOR");
    register_token(T_NOT, "NOT");
    register_token(T_OR, "OR");

    /* BASIC commands. */
    register_function(FUNC_KEYWORD, "CLEAR", do_clear);
    register_function(FUNC_KEYWORD, "CLR", do_clear);
    register_function(FUNC_KEYWORD, "CONT", do_cont);
    register_function(FUNC_KEYWORD, "DELETE", do_delete);
    register_function(FUNC_KEYWORD, "DIR", do_dir);
    register_function(FUNC_KEYWORD, "LIST", do_list);
    register_function(FUNC_KEYWORD, "LOAD", do_load);
    register_function(FUNC_KEYWORD, "NEW", do_clear);
    register_function(FUNC_KEYWORD, "RUN", do_run);
    register_function(FUNC_KEYWORD, "SAVE", do_save);

    /* BASIC statements. */
    register_function(FUNC_KEYWORD, "CLS", do_cls);
    t_kw_data = register_function(FUNC_KEYWORD, "DATA", do_data);
#if 0
    t_kw_def = register_function(FUNC_KEYWORD, "DEF", do_def_fn);
#endif
    register_function(FUNC_KEYWORD, "DIM", do_dim);
#ifdef _DEBUG
    register_function(FUNC_KEYWORD, "DUMP", do_dump);
#endif
    register_function(FUNC_KEYWORD, "END", do_end);
#if 0
    t_kw_else = register_token(T_ELSE, "ELSE");
    t_kw_fn = register_token(T_FN, "FN");
#endif
    t_kw_for = register_function(FUNC_KEYWORD, "FOR", do_for);
    register_function(FUNC_KEYWORD, "GET", do_get);
    t_kw_gosub = register_function(FUNC_KEYWORD, "GOSUB", do_gosub); 
    t_kw_goto = register_function(FUNC_KEYWORD, "GOTO", do_goto);
    t_kw_if = register_function(FUNC_KEYWORD, "IF", do_if);
    register_function(FUNC_KEYWORD, "INPUT", do_input);
    register_function(FUNC_KEYWORD, "LET", do_let);
    register_function(FUNC_KEYWORD, "NEW", do_new);
    t_kw_next = register_function(FUNC_KEYWORD, "NEXT", do_next);
    t_kw_on = register_function(FUNC_KEYWORD, "ON", do_on_goto);
#if 0
    register_function_1(FUNC_KEYWORD, "ONERR", do_onerr);
#endif
    register_function(FUNC_KEYWORD, "PRINT", do_print);
    register_function(FUNC_KEYWORD, "?", do_print);
    register_function(FUNC_KEYWORD, "READ", do_read);
    t_kw_rem = register_function(FUNC_KEYWORD, "REM", do_rem);
    register_function(FUNC_KEYWORD, "RETURN", do_return);
    register_function(FUNC_KEYWORD, "RESTORE", do_restore); 
    register_function_1(FUNC_KEYWORD, "SLEEP", do_sleep, KIND_NUMERIC);
    register_function_1(FUNC_PRINT, "SPC", do_spc, KIND_NUMERIC);
    t_kw_step = register_token(T_AUTO, "STEP");		// FIXME: T_STEP
    register_function(FUNC_KEYWORD, "STOP", do_stop);
    register_function_1(FUNC_PRINT, "TAB", do_tab, KIND_NUMERIC);
    t_kw_then = register_token(T_AUTO, "THEN");		// FIXME: T_THEN
    t_kw_to = register_token(T_AUTO, "TO");		// FIXME: T_TO

#if 0
    /* These are for the "files I/O" support module. */
    register_function(FUNC_KEYWORD, "CMD", files_cmd);
    register_function(FUNC_KEYWORD, "CLOSE", files_close);
    register_function(FUNC_STRING, "GET#", files_get);
    register_function(FUNC_STRING, "INPUT#", files_input);
    register_function(FUNC_KEYWORD, "OPEN", files_open);
    register_function(FUNC_KEYWORD, "PRINT#", files_print);
    register_function(FUNC_NUMERIC, "STATUS", files_status);
#endif
}


bool
do_stmt(void)
{
    basic_var rv;

    switch (sym) {
	case T_ERROR:
		basic_error("STATEMENT ERROR");
		break;

	case T_COLON:
		accept(sym);
		break;

	default:
		if (dispatch_function(sym, FUNC_KEYWORD, &rv))
			do_let(NULL);
		break;
    }

    return !__ERROR;
}
