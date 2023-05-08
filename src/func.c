/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Implement the various BASIC functions.
 *
 * Version:	@(#)func.c	1.1.1	2023/05/05
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
#ifdef _MSC_VER
# ifndef _USE_MATH_DEFINES
#  define _USE_MATH_DEFINES
# endif
#endif
#include <math.h>
#include <time.h>
#include "arch.h"
#include "basic.h"
#include "private.h"
#include "version.h"


static int
f_abs(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = (float)fabs(n->value.number);

    return 0;
}


static int
str_asc(basic_var *rv, const basic_var *str)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = (float)str->value.string[0];

    return 0;
}


static int
f_atn(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = atanf(n->value.number);

    return 0;
}


static int
str_chr(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_STRING;
    char *chr = (char *)malloc(16);

    sprintf(chr, "%c", (int)n->value.number);
    rv->value.string = chr;
    rv->mallocd = true;

    return 0;
}


static int
f_cos(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = cosf(n->value.number);

    return 0;
}


/* Return the current date as MMDDYYYY. */
static int
v_datestr(basic_var *rv)
{
    struct tm *tm;
    time_t now;
    char *str;

    (void)time(&now);
    tm = localtime(&now);

    str = (char *)malloc(13);
    if (str == NULL)
	return 1;
    sprintf(str, "%02u/%02u/%04u",
	(tm->tm_mday & 255), (tm->tm_mon & 255)+1, (tm->tm_year & 255)+1900);

    rv->kind = KIND_STRING;
    rv->value.string = str;
    rv->mallocd = true;

    return 0;
}


/* Return the line number of the current error. */
static int
v_errlin(basic_var *rv)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = 12345;	// FIXME: not usable until we have ONERR

    return 0;
}


static int
f_exp(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = expf(n->value.number);

    return 0;
}


static int
f_free(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    if (n->value.number)
	rv->value.number = (float)__stack_p;
    else
	rv->value.number = (float)lines_memory_available();

    return 0;
}


static int
f_int(basic_var *rv, const basic_var *n)
{
    int i = (int)n->value.number;  

    rv->kind = KIND_NUMERIC;
    rv->value.number = (float)(1.0 * i);

    return 0;
}


static int
str_left(basic_var *rv, const basic_var *str, const basic_var *length)
{
    rv->kind = KIND_STRING;
    rv->value.string = C_STRDUP(str->value.string);
    rv->value.string[(int) length->value.number] = '\0';
    rv->mallocd = true;

    return 0;
}


static int
str_len(basic_var *rv, const basic_var *str)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = (float)strlen(str->value.string);

    return 0;
}


/* Return the current line number. */
static int
v_line(basic_var *rv)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = __line;

    return 0;
}


static int
f_log(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = logf(n->value.number);

    return 0;
}


static int
str_mid(basic_var *rv, const basic_var *str, const basic_var *start, const basic_var *length)
{
    rv->kind = KIND_STRING;
    char *source = str->value.string;
    int _start = (int)start->value.number - 1;
    int _len = (int)strlen(source);
    int _length;

    if (_start>_len)
	_start = _len;

    if (length->empty) {
	_length = _len - _start;
    } else {
	_length = (int)length->value.number;
	if (_length + _start>_len)
		_length = _len - _start;
    }

    char *string = C_STRDUP(&source[_start]);
    string[_length] = '\0'; 
    rv->value.string = string;
    rv->mallocd = true;

    return 0;
}


/* Return the value of Pi. */
static int
v_pi(basic_var *rv)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = (float)M_PI;

    return 0;
}


static int
f_pow(basic_var *rv, const basic_var *x, const basic_var *y)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = powf(x->value.number, y->value.number);

    return 0;
}


static int
str_right(basic_var *rv, const basic_var *str, const basic_var *length)
{
    char *source = str->value.string;

    rv->kind = KIND_STRING;

    rv->value.string = C_STRDUP(&source[strlen(source) - (int) length->value.number]);
    rv->mallocd = true;

    return 0;
}


static int
f_rnd(basic_var *rv, const basic_var *n)
{
    struct tm *tm;
    time_t now;
    int random;

    rv->kind = KIND_NUMERIC;

    if (n->value.number > 0) {
	random = rand();

	rv->value.number = (float)((random * 1.0) / RAND_MAX);

	return 0;
    }

    if (n->value.number < 0) {
	srand((unsigned int)n->value.number);

	random = rand();
	rv->value.number = (float)((random * 1.0) / RAND_MAX);

	return 0;
    }

    now = time(NULL);    
    tm = localtime(&now);
    rv->value.number = (float)((tm->tm_sec * 1.0) / 60);

    return 0;
}


static int
f_sgn(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    if (n->value.number < 0.0) {
	rv->value.number = (float)-1.0;
    } else if (n->value.number > 0.0) {
	rv->value.number = (float)1.0;
    } else {
	rv->value.number = 0.0;
    }

    return 0; 
}


static int
f_sin(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = sinf(n->value.number);

    return 0;
}


static int
f_sqr(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = (float)sqrt((double) n->value.number);

    return 0;
}


static int
str_str(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_STRING;
    rv->value.string = (char *)malloc(16);
    sprintf(rv->value.string, "%f", n->value.number);
    rv->mallocd = true;

    return 0;
}


static int
f_tan(basic_var *rv, const basic_var *n)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = tanf(n->value.number);

    return 0;
}


/* Return the current time of day. */
static int
v_timestr(basic_var *rv)
{
    time_t now;
    struct tm *tm;
    char *str;

    (void)time(&now);
    tm = localtime(&now);

    str = (char *)malloc(9);
    if (str == NULL)
	return 1;
    sprintf(str, "%02i:%02i:%02i", tm->tm_hour, tm->tm_min, tm->tm_sec);

    rv->kind = KIND_STRING;
    rv->value.string = str;
    rv->mallocd = true;

    return 0;
}


static int
str_val(basic_var *rv, const basic_var *str)
{
    rv->kind = KIND_NUMERIC;
    rv->value.number = (float)atof(str->value.string);

    return 0;
}


/* Return the version ID of the interpreter. */
static int
v_version(basic_var *rv)
{
    rv->kind = KIND_STRING;
    rv->value.string = APP_VERSION;

    return 0;
}


/* Register the various BASIC functions. */
void
func_init(void)
{
    /* Register system variables. */
    register_function(FUNC_NUMERIC, "LINE", v_line);
    register_function(FUNC_NUMERIC, "ERRLIN", v_errlin);
    register_function(FUNC_STRING, "DATE$", v_datestr);
    register_function(FUNC_NUMERIC, "PI", v_pi);
    register_function(FUNC_STRING, "TIME$", v_timestr);
    register_function(FUNC_STRING, "VERSION$", v_version);

    /* Register functions. */
    register_function_1(FUNC_NUMERIC, "ABS", f_abs, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "ASC", str_asc, KIND_STRING);
    register_function_1(FUNC_NUMERIC, "ATN", f_atn, KIND_NUMERIC);
    register_function_1(FUNC_STRING, "CHR$", str_chr, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "COS", f_cos, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "EXP", f_exp, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "FRE", f_free, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "INT", f_int, KIND_NUMERIC);
    register_function_2(FUNC_STRING, "LEFT$", str_left, KIND_STRING, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "LEN", str_len, KIND_STRING);
    register_function_1(FUNC_NUMERIC, "LOG", f_log, KIND_NUMERIC);
    register_function_3(FUNC_STRING, "MID$", str_mid, KIND_STRING, KIND_NUMERIC, KIND_NUMERIC);
#if 0
    register_function_1(FUNC_NUMERIC, "POS", f_pos, KIND_NUMERIC);
#endif
    register_function_2(FUNC_NUMERIC, "POW", f_pow, KIND_NUMERIC, KIND_NUMERIC);
    register_function_2(FUNC_STRING, "RIGHT$", str_right, KIND_STRING, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "RND", f_rnd, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "SGN", f_sgn, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "SIN", f_sin, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "SQR", f_sqr, KIND_NUMERIC);
    register_function_1(FUNC_STRING, "STR$", str_str, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "TAN", f_tan, KIND_NUMERIC);
    register_function_1(FUNC_NUMERIC, "VAL", str_val, KIND_STRING);
}
