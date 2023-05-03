/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Handle the expressions.
 *
 * Version:	@(#)expr.c	1.1.0	2023/05/01
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


static float
op_and(float a, float b)
{
    return (float) ((int) a & (int) b);
}


static float
op_eor(float a, float b)
{
    return (float) ((int) a ^ (int) b);
}


static float
op_or(float a, float b)
{
    return (float) ((int) a | (int) b);
}


static char *
string_term(void)
{
    char var_name[BASIC_VAR_LEN+1];
    int vector[MAX_VECTOR];
    char *string = NULL;
    basic_var rv;
    int l;

    switch (sym) {
	case T_STRING:
		string = C_STRDUP(tokenizer_get_string());
		accept(T_STRING);
		break;

	 case T_VARIABLE_STRING:
		tokenizer_get_variable_name(var_name);
		get_sym();
		if (sym == T_LEFT_BANANA) {
			l = (int)strlen(var_name);
			var_name[l] = '(';
			var_name[l+1] = '\0';
			accept(T_LEFT_BANANA);
			get_vector(vector, MAX_VECTOR);
			string = C_STRDUP(vars_array_get_string(var_name, vector));
			if (string == NULL)
				string = &__dummy;

			expect(T_RIGHT_BANANA);
		} else {
			string = C_STRDUP(vars_get_string(var_name));
			accept(T_VARIABLE_STRING);
		}
		break;

	default:
		if (! dispatch_function(sym, FUNC_STRING, &rv)) {
			if (rv.kind != KIND_STRING)
				basic_error("EXPECTED STRING TERM");

			string = C_STRDUP(rv.value.string);
			if (rv.mallocd == true)
				free(rv.value.string);
		}
		break;
    }

    return string;
}


char *
string_expression(void)
{
    char *s1 = string_term();
    char *s2;
    int l;

    while (sym == T_PLUS) {
	accept(T_PLUS);

	s2 = string_term();
	l = (int)strlen(s1) + (int)strlen(s2) + 1;
	s1 = realloc(s1, l);
	s1 = strcat(s1, s2);
	free(s2);
    }
 
    return s1; 
}


static float
numeric_factor(void)
{
    char var_name[BASIC_VAR_LEN+1];
    int vector[MAX_VECTOR];
    float number = 0;
    basic_var rv;
    int l;

//FIXME: try functions last, saves execution time
    if (! dispatch_function(sym, FUNC_NUMERIC, &rv)) {
	if (rv.kind != KIND_NUMERIC)
		basic_error("EXPECTED NUMERIC FACTOR");
	number = rv.value.number;
    } else if (sym == T_NUMBER) {
	number = tokenizer_get_number();
	accept(T_NUMBER);
    } else if (sym == T_VARIABLE_NUMBER) {
	tokenizer_get_variable_name(var_name);
	get_sym();
	if (sym == T_LEFT_BANANA) {
		l = (int)strlen(var_name);
		var_name[l] = '(';
		var_name[l+1] = '\0';
		accept(T_LEFT_BANANA);
		get_vector(vector, MAX_VECTOR);
		number = vars_array_get_numeric(var_name, vector);
		expect(T_RIGHT_BANANA);
	} else {
		number = vars_get_numeric(var_name);
		accept(T_VARIABLE_NUMBER);
	}
    } else if (accept(T_LEFT_BANANA)) {
	number = numeric_expression();
	expect(T_RIGHT_BANANA);
    } else {
	basic_error("FACTOR SYNTAX ERROR");
	get_sym();
    }

    return number; 
}


static bool
string_condition(const char *left, const char *right, relop op)
{
    int comparison = strcmp(left, right);

//  printf("String condition('%s','%s'): %d\n", left, right, comparison);

    switch (op) {
	case OP_NOP:
		basic_error("EXPECTED RELOP");
		break;

	case OP_LT:
		return comparison < 0;

	case OP_LE:
		return comparison <= 0;

	case OP_EQ:
		return comparison == 0;

	case OP_NE:
		return comparison != 0;

	case OP_GE:
		return comparison >= 0;

	case OP_GT:
		return comparison > 0;  
    }

    return false;
}


static float
factor(void)
{
    char *s1, *s2;
    relop op;
    float r;

    if (sym == T_NOT) {
	get_sym();

	if (sym == T_NUMBER || sym == T_VARIABLE_NUMBER) {
		r = numeric_factor();
		r = (float) (~ (int)r);

		return r;
	} else
		basic_error("NOT on STRING");
    }

    if (sym == T_STRING || sym == T_VARIABLE_STRING) {
	s1 = string_term(); 
	op = get_relop();
	if (op == OP_NOP) {
		free(s1);
		basic_error("EXPECTED RELOP");
		return 0;
	}

	s2 = string_term();
	r = string_condition(s1, s2, op);
	free(s2);
	free(s1);

	return r;
    }

    return numeric_factor();
}


static float
power_term(void)
{
    float f1 = factor();
    float f2;

    while (sym == T_POWER) {
	get_sym();
	f2 = factor();
	f1 = (float)pow(f1, f2);
    }

    return f1;
}


static float
term(void)
{
    float f1 = power_term();
    float f2;
    token op;

    while (sym == T_MULTIPLY || sym == T_DIVIDE || sym == T_AND) {
	op = sym;
	get_sym();
	f2 = power_term();

	switch (op) {
		case T_MULTIPLY:
			f1 = f1 * f2;
			break;

		case T_DIVIDE:
			f1 = f1 / f2;
			break;

		case T_AND:
			f1 = op_and(f1, f2);
			break;

		default:
			basic_error("TERM SYNTAX ERROR");    
	}
    }

    return f1;
}


float
numeric_expression(void)
{
    token op = T_PLUS;
    float t1, t2;

    if (sym == T_PLUS || sym == T_MINUS) {
	op = sym;
	get_sym();
    }

    t1 = term();
    if (op == T_MINUS)
	t1 = -1 * t1;

    while (sym == T_PLUS || sym == T_MINUS ||
	   sym == T_OR || sym == T_EOR || sym == T_XOR) {
	op = sym;
	get_sym();
	t2 = term();
	switch (op) {
		case T_PLUS:
			t1 = t1 + t2;
			break;

		case T_MINUS:
			t1 = t1 - t2;
			break;

		case T_OR:
			t1 = op_or(t1, t2);
			break;

		case T_EOR:
		case T_XOR:
			t1 = op_eor(t1, t2);
			break;

		default:
			basic_error("EXPRESSION SYNTAX ERROR");
	 }
    }

    return t1;
}


static bool
numeric_condition(float left, float right, relop op)
{
//  printf("numeric condition %f, %f, %d\n", left, right, op);

    switch (op) {
	case OP_NOP:
		basic_error("EXPECTED RELOP");
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

	case OP_NE:
		return left != right;
    }

    return false;
}


void
expression(expr_result *result)
{
    char *string, *s2;
    float number, n2;
    relop op;

    string = string_expression();
    if (string != NULL) {
	op = get_relop();
	if (op == OP_NOP) {
		result->type = EXPR_TYPE_STRING;
		result->value.string = string;
	} else {
		s2 = string_expression();

		result->type = EXPR_TYPE_NUMERIC;
		result->value.numeric = string_condition(string, s2, op);
		free(s2);
		free(string);
	}
    } else {
	number = numeric_expression();
	op = get_relop();
	if (op != OP_NOP) {
		n2 = numeric_expression();
		number = numeric_condition(number, n2, op);
	}

	result->type = EXPR_TYPE_NUMERIC;
	result->value.numeric = number;
    }
}


bool
condition(expr_result *left, expr_result *right, relop op)
{
    if (left->type == EXPR_TYPE_NUMERIC) {
	if (right->type != EXPR_TYPE_NUMERIC)
		basic_error("EXPECTED NUMERIC RIGHT HAND TYPE");  

	return numeric_condition(left->value.numeric, right->value.numeric, op);
    } else {
	if (right->type != EXPR_TYPE_STRING)
		basic_error("EXPECTED STRING RIGHT HAND TYPE");
	return string_condition(left->value.string, right->value.string, op);;
    }

    return false;
}
