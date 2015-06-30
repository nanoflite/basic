#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <variables.h>
#include <dictionary.h>

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
  var->value.num = value;
  dictionary_put(_dictionary, name, var);
  return var;
}

variable_type
variable_get_type(char* name)
{
  return variable_type_unknown;  
}


