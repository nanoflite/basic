/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Handle variables.
 *
 * Version:	@(#)vars.c	1.1.0	2023/05/01
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
#include "arch.h"
#include "basic.h"
#include "private.h"


typedef union {
    float	num;
    char	*string;
} var_value;

typedef struct variable {
    char	*name;
    var_type	type;
    var_value	value;
    bool	is_array;
    int		nr_dimensions;
    int		dimensions[5];
    array	*array;
} variable;


static dictionary	*_dict = NULL;
static const char	*E_INDEX_OUT_OF_BOUNDS = "INDEX OUT OF BOUNDS";
static const char	*E_VAR_NOT_FOUND = "VAR NOT FOUND";


#if PLATFORM != PLATFORM_XMEGA
static void
calc_vector(const variable *var, int index, int *vector)
{
    int i, product = 1;

    for (i = 1; i < var->nr_dimensions; ++i)
	product *= var->dimensions[i];

    for (i = 0; i < var->nr_dimensions; ++i) {
	vector[i] = index / product;
	index %= product;
	if ((i+1) < var->nr_dimensions)
		product /= var->dimensions[i+1];
    }
}  


static void
vector_print(int *vector, int dimensions)
{
    int i;

    for (i = 0; i < dimensions; i++) {
	printf("%i", vector[i]);

	if (i < dimensions - 1)
		printf(",");
    }
}
#endif


bool
vars_init(void)
{
  _dict = dict_new();

  return _dict != NULL;
}


static void
destroy_cb(const char *name, void *value, void *context)
{
    variable *var = (variable *)value;

    if (var->type == VAR_TYPE_STRING) {
	if (var->value.string != NULL)
		free(var->value.string);
    }

    if (var->is_array)
	array_destroy(var->array); 

    if (var->name != NULL)
	free(var->name);

    free(var);
}


void
vars_destroy(void)
{
    dict_destroy(_dict, destroy_cb);
}


variable *
vars_get(const char *name)
{
    return dict_get(_dict, name);
}


char *
vars_get_string(const char *name)
{
//  printf("Var name: '%s'\n", name);
    variable *var = dict_get(_dict, name);

    if (var == NULL)
	var = vars_set_string(name, "");

    return var->value.string;
}


variable *
vars_set_string(const char *name, const char *value)
{
//  printf("set var '%s' to '%s'\n", name, value); 
    variable *var = dict_get(_dict, name);

    if (var == NULL) {
	var = (variable*)malloc(sizeof(variable));

	var->name = C_STRDUP(name);
	var->type = VAR_TYPE_STRING;
	var->is_array = false;
    } else {
	if (var->value.string != NULL)
		free(var->value.string);
    }

    var->value.string = C_STRDUP(value);

    dict_put(_dict, name, var);

    return var;
}


float
vars_get_numeric(const char *name)
{
//  printf("Var name: '%s'\n", name);
    variable *var = dict_get(_dict, name);

    if (var == NULL)
	var = vars_set_numeric(name, 0);

    return var->value.num;
}


variable *
vars_set_numeric(const char *name, float value)
{
//  printf("set var '%s' to %f\n", name, value); 
    variable *var = dict_get(_dict, name);

    if (var == NULL) {
	var = (variable*)malloc(sizeof(variable));
	var->name = C_STRDUP(name);
	var->type = VAR_TYPE_NUMERIC;
	var->is_array = false;
    }
    var->value.num = value;

    dict_put(_dict, name, var);

    return var;
}


var_type
vars_get_type(const char *name)
{
    variable *var = dict_get(_dict, name);

    return var->type;  
}


// Calculates array size
static size_t
calc_size(variable *var)
{
    int i, size = 1;

    for (i = 0; i < var->nr_dimensions; ++i)
	size *= var->dimensions[i];

//  printf("array size %s = %i\n", var->name, size);

    return size;
}


static bool
check_in_bounds(variable *var, int *vector)
{
    int vector_i;
    int i;

//  printf("check in bounds\n");
//  printf("dimensions %i\n", var->nr_dimensions);

    for (i = 0; i < var->nr_dimensions; i++) {
	vector_i = vector[i];
//	printf("vector_%i = %i\n", i, vector_i);
//	printf("dimension_%i = %i\n", i, var->dimensions[i]); 

//	DIM A(3) -> A(1), A(2), A(3)
	if (vector_i > var->dimensions[i])
		return false;
    }

    return true;
}  


/*
  DIM A(2,3)
    w h
    x y
  A(1,1) 0 
  A(1,2) 1 
  A(1,3) 2 
  A(2,1) 3 
  A(2,2) 4 
  A(2,3) 5 
 
  (x*ySize*zSize + y*zSize + z)

  v[0] = x
  v[1] = y
  v[2] = z

  v[0] * d[1] * ... d[n] + v[1] * d[2] ... d[n] + ... + v[n]


 */
static int
calc_index(variable *var, int *vector)
{
    int i, j, index = 0;

//  printf("calc_index\n");
//  vars_dump(var);

    for (i = 0; i < var->nr_dimensions; ++i) {
//	int product = vector[i] - 1;
	int product = vector[i];

	for (j = i+1; j < var->nr_dimensions; ++j)
		product *= var->dimensions[j];
	index += product;
    }

#if 0
    printf("index[ %s", var->name);
    for (i = 0; i < var->nr_dimensions; i++) {
	printf("%j", vector[i]);
	if ( i < var->nr_dimensions-1) printf(",");
    }
    printf(") ] = %j\n", index); 
#endif

    return index;
}


variable *
vars_array_init(const char *name, var_type type, int dims, int *vector)
{
    variable *var = (variable *)malloc(sizeof(variable));

    var->name = C_STRDUP(name);
    var->is_array = true;
    var->value.string = NULL;
    var->value.num = 0;
    var->type = type;
    var->nr_dimensions = dims;
    var->dimensions[0] = vector[0] + 1;
    var->dimensions[1] = vector[1] + 1;
    var->dimensions[2] = vector[2] + 1;
    var->dimensions[3] = vector[3] + 1;
    var->dimensions[4] = vector[4] + 1;
    var->array = array_new(sizeof(var_value));

    size_t size = calc_size(var);
    array_alloc(var->array, size);

    if (type == VAR_TYPE_STRING) {
	for (size_t i = 0 ; i < size ; ++i) {
		var_value val;

		val.string = C_STRDUP("");
		array_set(var->array, i, &val);
	}
    }

    dict_put(_dict, name, var);

    return var;
}


char *
vars_array_get_string(const char *name, int *vector)
{
    var_value *val;
    size_t index; 
    variable *var = dict_get(_dict, name);

    if (var == NULL) {
	basic_error(E_VAR_NOT_FOUND);
	return NULL;
    }

    if (! check_in_bounds(var, vector)) {
	basic_error(E_INDEX_OUT_OF_BOUNDS);
	return NULL;
    }

    index = calc_index(var, vector); 
    val = array_get(var->array, index);

    return val->string;
}


variable *
vars_array_set_string(const char *name, const char *value, int *vector)
{
    var_value val;
    size_t index; 
    variable *var = dict_get(_dict, name);

    if (var == NULL) {
	basic_error(E_VAR_NOT_FOUND);
	return NULL;
    }

    if (! check_in_bounds(var, vector)) {
	basic_error(E_INDEX_OUT_OF_BOUNDS);
	return NULL;
    }

    index = calc_index(var, vector); 
    val.string = C_STRDUP(value);
    array_set(var->array, index, &val);

    return var;
}


float
vars_array_get_numeric(const char *name, int *vector)
{
    var_value *val;
    size_t index; 
    variable *var = dict_get(_dict, name);

    if (var == NULL) {
	basic_error(E_VAR_NOT_FOUND);
	return 0;
    }

    if (! check_in_bounds(var, vector)) {
	basic_error(E_INDEX_OUT_OF_BOUNDS);
	return 0;
    }

    index = calc_index(var, vector); 
    val = array_get(var->array, index);

    return val->num;
}


variable *
vars_array_set_numeric(const char *name, float value, int *vector)
{
//  printf("set numeric %s, %f, %i\n", name, value, vector[0]);
    var_value val;
    size_t index; 
    variable *var = dict_get(_dict, name);

    if (var == NULL) {
	basic_error(E_VAR_NOT_FOUND);
	return NULL;
    }

    if (! check_in_bounds(var, vector)) {
	basic_error(E_INDEX_OUT_OF_BOUNDS);
	return NULL;
    }

//  vars_dump(var);

    index = calc_index(var, vector); 
//  printf("index = %i\n", index);

    val.num = value;
    array_set(var->array, index, &val);

//  vars_dump(var);
  
    return var;
}


struct each_v_ctx {
    variables_each_cb cb;
    void	*context;
};


static void
vars_each_cb(const char *name, void *value, void *context)
{
    struct each_v_ctx* ctx = (struct each_v_ctx *)context;
    variable *var = (variable *)value;

    ctx->cb(var, ctx->context);
}


void
vars_each(variables_each_cb each, void *context)
{
    struct each_v_ctx ctx = {
	.cb = each,
	.context = context
    };

    dict_each(_dict, vars_each_cb, &ctx);
}


void
vars_dump(const variable *var)
{
    int i;

    printf("-- variable\n" 
	   "\tname:'%s'\n"
	   "\ttype: %s\n",
	var->name,
	(var->type == VAR_TYPE_NUMERIC) ? "number" : "string");

    if (var->is_array) {
	printf("\tdimensions: %i\n", var->nr_dimensions);
	for (i = 0; i < var->nr_dimensions; i++)
		printf("\tdim %i size = %i\n", i, (int)var->dimensions[i]);
	printf("\tarray size: %i\n", (int)array_size(var->array));
	for (i = 0; i < (int)array_size(var->array); i++) {
		int vector[5];
		var_value *val;

		calc_vector(var, i, vector);
		printf("\t%3i %s", i, var->name);
		vector_print(vector, var->nr_dimensions);
		printf(") = ");
		val = array_get(var->array, i);
		if (var->type == VAR_TYPE_STRING)
			printf("%s\n", (val->string) ? val->string : "");
		else
			printf("%f\n", val->num); 
	}
    } else {
	if (var->type == VAR_TYPE_STRING)
		printf("\tvalue: '%s'\n", var->value.string);
	else
		printf("\tvalue: %f\n", var->value.num);
    }
}
