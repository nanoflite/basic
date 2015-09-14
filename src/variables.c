#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <variables.h>
#include <dictionary.h>
#include <array.h>

typedef union
{
  float num;
  char *string;
} variable_value;

struct variable
{
  char *name;
  int type;
  variable_value value;
  bool is_array;
  size_t nr_dimensions;
  size_t dimensions[5];
  array* array;
};

dictionary *_dictionary = NULL;

bool
variables_init(void)
{
  _dictionary = dictionary_new();
  return _dictionary != NULL;
}

void
variables_destroy(void)
{}

variable*
variable_get(char* name)
{
  return dictionary_get(_dictionary, name);
}

char*
variable_get_string(char* name)
{
  // printf("Var name: '%s'\n", name);
  variable *var = dictionary_get(_dictionary, name);
  return var->value.string;
}

float
variable_get_numeric(char* name)
{
  // printf("Var name: '%s'\n", name);
  variable *var = dictionary_get(_dictionary, name);
  return var->value.num;
}

variable*
variable_set_string(char* name, char* value)
{
  // printf("set var '%s' to '%s'\n", name, value); 
  variable* var = (variable*) malloc(sizeof(variable));
  var->name = strdup(name);
  var->is_array = false;
  var->value.string = strdup(value);
  dictionary_put(_dictionary, name, var);
  return var;
}

variable*
variable_set_numeric(char* name, float value)
{
  // printf("set var '%s' to %f\n", name, value); 
  variable* var = (variable*) malloc(sizeof(variable));
  var->name = strdup(name);
  var->is_array = false;
  var->value.num = value;
  dictionary_put(_dictionary, name, var);
  return var;
}

variable_type
variable_get_type(char* name)
{
  return variable_type_unknown;  
}

  static size_t
calc_size(variable* var)
{
  size_t size = 1;
  for(size_t i=0; i<var->nr_dimensions; i++)
  {
    size *= var->dimensions[i];
  }
  return size;
}

  static size_t
calc_index(variable* var, size_t* vector)
{
  size_t index = 0;
  size_t size = calc_size(var);

  for(size_t i= 0; i<var->nr_dimensions; ++i)
  {
    index += size * vector[i];
    if( (i+1) < var->nr_dimensions)
    {
        size /= var->dimensions[i+1];
    }
  }

  return index;
}

variable*
variable_array_init(char* name, size_t dimensions, size_t* vector)
{
  variable* var = (variable*) malloc(sizeof(variable));
  var->name = strdup(name);
  var->is_array = true;
  var->nr_dimensions = dimensions;
  var->dimensions[0] = vector[0];
  var->dimensions[1] = vector[1];
  var->dimensions[2] = vector[2];
  var->dimensions[3] = vector[3];
  var->dimensions[4] = vector[4];
  var->array = array_new(sizeof(variable_value));
  array_alloc(var->array, calc_size(var)); 
  dictionary_put(_dictionary, name, var);
  return var;
}

variable*
variable_array_set_string(char *name, char *value, size_t* vector)
{
  variable* var = dictionary_get(_dictionary, name);
  size_t index = calc_index(var, vector); 
  variable_value val;
  val.string = strdup(value);
  array_set(var->array, index, &val);
  return var;
}

  char*
variable_array_get_string(char *name, size_t* vector)
{
  variable* var = dictionary_get(_dictionary, name);
  size_t index = calc_index(var, vector); 
  variable_value* val = array_get(var->array, index);
  return val->string;
}

variable*
variable_array_set_numeric(char *name, float value, size_t* vector)
{
  variable* var = dictionary_get(_dictionary, name);
  size_t index = calc_index(var, vector); 
  variable_value val;
  val.num = value;
  array_set(var->array, index, &val);
  return var;
}

float
variable_array_get_numeric(char *name, size_t* vector)
{
  variable* var = dictionary_get(_dictionary, name);
  size_t index = calc_index(var, vector); 
  variable_value* val = array_get(var->array, index);
  return val->num;
}
