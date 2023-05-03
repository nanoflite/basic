/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Handle the tokenizing stuff.
 *
 * Version:	@(#)token.c	1.1.0	2023/05/01
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
#include "arch.h"
#include "basic.h"
#include "private.h"


static array	*__tokens = NULL;
static token	token_id = TOKEN_TYPE_END + 1000;
static char 	*tokenizer_p = NULL;
static float	actual_number;
static char	actual_string[BASIC_STR_LEN+1];
static char	actual_variable[BASIC_VAR_LEN+1];


#define add_token(t, k) \
static const token_entry _##t = { t, k };

add_token(T_ERROR, NULL);
add_token(T_EOL, NULL);
add_token(T_NUMBER, NULL);
add_token(T_STRING, NULL);
add_token(T_VARIABLE_STRING, NULL);
add_token(T_VARIABLE_NUMBER, NULL);
add_token(T_PLUS, "+");
add_token(T_MINUS, "-");
add_token(T_MULTIPLY, "*");
add_token(T_DIVIDE, "/");
add_token(T_POWER, "^");
add_token(T_LEFT_BANANA, "(");
add_token(T_RIGHT_BANANA, ")");
add_token(T_COLON, ":");
add_token(T_SEMICOLON, ";");
add_token(T_COMMA, ",");
add_token(T_EQUALS, "=");
add_token(T_LESS, "<");
add_token(T_GREATER, ">");


static bool
isvarchar(char c)
{
    if (c >= 'A' && c <= 'Z')
	return true;

    if (c == '$')
	return true;

    if (c >= '0' && c <= '9')
	return true;

    return false;
}


static token
_find_registered(void)
{
    size_t i;

    for (i = 0; i < array_size(__tokens); i++) {
	token_entry *entry = (token_entry *)array_get(__tokens, i);

	if (entry->name == NULL)
		continue;

	if (! strncmp(tokenizer_p, entry->name, strlen(entry->name))) {
		tokenizer_p += strlen(entry->name);

		return entry->token;
	}
    }

    return T_NFOUND;
}


void
tokenizer_setup(void)
{
    __tokens = array_new(sizeof(token_entry));

    tokenizer_register_token(&_T_ERROR);
    tokenizer_register_token(&_T_EOL);
    tokenizer_register_token(&_T_NUMBER);
    tokenizer_register_token(&_T_STRING);
    tokenizer_register_token(&_T_VARIABLE_STRING);
    tokenizer_register_token(&_T_VARIABLE_NUMBER);
    tokenizer_register_token(&_T_PLUS);			// +
    tokenizer_register_token(&_T_MINUS);		// -
    tokenizer_register_token(&_T_MULTIPLY);		// *
    tokenizer_register_token(&_T_DIVIDE);		// /
    tokenizer_register_token(&_T_POWER);		// ^
    tokenizer_register_token(&_T_LEFT_BANANA);		// (
    tokenizer_register_token(&_T_RIGHT_BANANA);		// )
    tokenizer_register_token(&_T_COLON);		// :
    tokenizer_register_token(&_T_SEMICOLON);		// ;
    tokenizer_register_token(&_T_COMMA);		// ,
    tokenizer_register_token(&_T_EQUALS);		// =
    tokenizer_register_token(&_T_LESS);			// <
    tokenizer_register_token(&_T_GREATER);		// >
}


token
tokenizer_register_token(const token_entry *entry)
{
    token_entry ent;
    token tok;

    tok = entry->token;

    if (tok == T_AUTO) {
	memcpy(&ent, entry, sizeof(token_entry));
	tok = ent.token = token_id++;

	array_push(__tokens, (void *)&ent);
    } else
	array_push(__tokens, (void *)entry);

    return tok;
}


void
tokenizer_free_registered_tokens(void)
{
    array_destroy(__tokens);
}


void
tokenizer_init(char *input)
{
    tokenizer_p = input;
}


char *
tokenizer_char_pointer(char *set)
{
    if (set != NULL) {
	tokenizer_p = set; 
	return NULL;
    }

    /* Skip white space. */
    while (*tokenizer_p && isspace(*tokenizer_p))
	tokenizer_p++;

    return tokenizer_p;
}


token
tokenizer_get_next_token(void)
{
    char temp[BASIC_STR_LEN+1];
    char *next_p;
    size_t l = 0;
    float f;

    if (*tokenizer_p == '\0')
	return T_EOL;

    // Skip white space
    while (*tokenizer_p && isspace(*tokenizer_p))
	tokenizer_p++;

    /* Check for number. */
    if (isdigit(*tokenizer_p) || *tokenizer_p == '.') {
	next_p = tokenizer_p;
	while (*next_p && (isdigit(*next_p) || *next_p == '.')) {
		l++;
		next_p++;
	}

	if (l > BASIC_STR_LEN)
		return T_ERROR;

	memset(temp, 0, l+1);
	strncpy(temp, tokenizer_p, sizeof(temp)-1);
	temp[l] = '\0';
	sscanf(temp, "%f", &f);
	actual_number = f;

	tokenizer_p = next_p;

	return T_NUMBER;
    }

    /* Check for string. */
    if (*tokenizer_p == '"') {
	next_p = ++tokenizer_p;
	while (*next_p && '"' != *next_p) {
		l++;
		next_p++;
	}

	/* Skip trailing " if we have one. */
	if (*next_p)
		next_p++;

	if (l > BASIC_STR_LEN)
		return T_ERROR;

	memcpy(actual_string, tokenizer_p, l);
	actual_string[l] = '\0';
   
	tokenizer_p = next_p;

	return T_STRING; 
    }

    token t = _find_registered();
    if (t != T_NFOUND)
	return t;

    /* Check for variable. */
    next_p = tokenizer_p;
    while (*next_p && isvarchar(*next_p)) {
	l++;
	next_p++;
    }

    if (l > BASIC_VAR_LEN)
	return T_ERROR;

    if (l > 0) {
	strncpy(actual_variable, tokenizer_p, l);
	actual_variable[l] = '\0';

	tokenizer_p = next_p;

	if (actual_variable[l - 1] == '$')
		return T_VARIABLE_STRING;

	return T_VARIABLE_NUMBER;
    }

    return T_ERROR; 
}


float
tokenizer_get_number(void)
{
    return actual_number;
}


char *
tokenizer_get_string(void)
{
    return actual_string;
}


void
tokenizer_get_variable_name(char *name)
{
    int i = sizeof(actual_variable);

    //NOTE: this is just to silence the warning!
    strncpy(name, actual_variable, i);
}
