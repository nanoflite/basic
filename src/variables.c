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
  variable_type type;
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
  var->type = variable_type_string;
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
  var->type = variable_type_numeric;
  var->is_array = false;
  var->value.num = value;
  dictionary_put(_dictionary, name, var);
  return var;
}

variable_type
variable_get_type(char* name)
{
  variable *var = dictionary_get(_dictionary, name);
  return var->type;  
}

  static size_t
calc_size(variable* var, size_t n)
{
  if ( n >= var->nr_dimensions )
  {
    // puts("OOOOPS");
    return 1;
  }

  size_t size = 1;
  for(size_t i=0; i <= n; i++)
  {
    size *= var->dimensions[i];
  }
  return size;
}

  static size_t
calc_index(variable* var, size_t* vector)
{
  size_t index = 0;
  size_t size = calc_size(var, var->nr_dimensions-1);

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

  static void
calc_vector(variable* var, size_t index, size_t* vector)
{
/*

1. dimensional

    a[10]
    a[i] = a[i]

    v = (i)

2. dimensional

    dim a[10][3]
      i   j
    a[i][j] = a[i*10+3]

    dim a[s1][s2]
          i   j
    a[i][j] = a[i*s1+j]

    v = (  i - ( i % s1 ) / s1    ,i % s1 )

    i = 3
    j = 2
    index = 3 * 10 + 2 = 32

    j -> 32 % 10 = 2
    32 - ( 32 % 10 ) = 30
    i -> 30 / 10 = 3
    

3. dimensional

    dim a[s1][s2][s3]
          i   j   k
    a[i][j][k] = a[ i*s1*s2 + j*s1 + k ]

    index = i*s1*s2 + j*s1 + k
    
    i = index / (s1*s2)
    j = ( index - i * s1 * s2 ) / s2
    k = index - i * s1 *s2 - j * s1

    s1 = 7, s2 = 9, s3 = 4
    i = 5
    j = 8
    k = 2
    index = 5*7*9 + 8*7 + 2 =  373
  
    i = 373 / (7*9) = 5
    j = ( 373 - 5*7*9 ) / 7 = 8
    k = 373 - 5*7*9 - 8*7 = 2

n. dimensional

    dim a[s1][s2]...[sn]
          i0  i1     in

    P(n) = s1*s2*...*sn
    a[i0][i1]...[in] = a[ i0*P(n-1) + i1*P(n-2) + in ]

    index = i0*P(n-1) + i1*P(n-2) + in 

    index0 = index
    i0 = index // P(n-1)

    index1 = index - i0 * (Pn-1)
    i1 = ( index - i0 * P(n-1 ) // P(n-2)
    i1 = index1 // P(n-2)   

     
    Pn = s1*s2*...sn
    index(n) = index(n-1) - i(n-1) * P(n-1)
    i(n) = index(n) // P(n-2)

*/
  // printf("calc vector\n"); 
  for(size_t i = 0; i<5; i++)
  {
    vector[i] = 0;
  }
  for(size_t i = 0; i<var->nr_dimensions; i++)
  {
    // printf("i=%ld, dimensions=%ld, index=%ld\n", i, var->nr_dimensions, index);
    size_t size = calc_size(var, var->nr_dimensions - 1 - i - 1);
    // printf("size=%ld\n", size);

    size_t in = index / size;
    // printf("v%ld=%ld\n", (var->nr_dimensions-i-1), in);
    vector[var->nr_dimensions-i-1] = in;
    index = index - in * size;
  }
  
}

variable*
variable_array_init(char* name, variable_type type, size_t dimensions, size_t* vector)
{
  variable* var = (variable*) malloc(sizeof(variable));
  var->name = strdup(name);
  var->is_array = true;
  var->type = type;
  var->nr_dimensions = dimensions;
  var->dimensions[0] = vector[0];
  var->dimensions[1] = vector[1];
  var->dimensions[2] = vector[2];
  var->dimensions[3] = vector[3];
  var->dimensions[4] = vector[4];
  var->array = array_new(sizeof(variable_value));
  array_alloc(var->array, calc_size(var, var->nr_dimensions-1)); 
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

struct each_v_ctx
{
  variables_each_cb cb;
  void* context;
};

void each_v(char* name, void* value, void* context)
{
  struct each_v_ctx* ctx = (struct each_v_ctx*) context;
  variable* var = (variable*) value;
  ctx->cb(var, ctx->context);
}

void
variables_each(variables_each_cb each, void* context)
{
  struct each_v_ctx ctx =
  {
    .cb = each,
    .context = context
  };
  dictionary_each(_dictionary, each_v, &ctx);
}

void
variable_dump(variable* var)
{
  printf(
    "-- variable\n" 
    "\tname:'%s'\n"
    "\ttype: %s\n",
      var->name,
      (var->type == variable_type_numeric) ? "number" : "string"
  );

  if (var->is_array)
  {
    printf("\tdimensions: %ld\n", var->nr_dimensions);
    for(size_t d=0; d<var->nr_dimensions; d++)
    {    
      printf("\tsize%ld: %ld\n", d, var->dimensions[d]);
    }
    printf("array size: %ld\n", array_size(var->array));
    for(size_t i=0; i<array_size(var->array); i++)
    {
      size_t vector[5];
      calc_vector(var, i, vector);
      printf("\t%s%ld,%ld,%ld,%ld,%ld) = ", var->name, vector[0], vector[1], vector[2], vector[3], vector[4]);
      if (var->type == variable_type_string)
      {
        printf("%s\n", (var->value.string) ? var->value.string : "");
      }
      else
      {
        printf("%f\n", var->value.num); 
      }
    }
  }
  else
  {
    if (var->type == variable_type_numeric)
    {
      printf("\tvalue: %f\n", var->value.num);
    }
    else
    {
      printf("\tvalue: '%s'\n", var->value.string);
    }

  }
}

