#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <error.h>
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

#if ARCH!=ARCH_XMEGA
static void vector_print(size_t* vector, size_t dimensions);
#endif

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


// Calculates array size
//  static size_t
//calc_array_size(variable* var)
//{
//  size_t size = 1;
//  for(size_t i=0; i<var->nr_dimensions; i++)
//  {
//    size *= var->dimensions[i];
//  }
//
//  printf("size = %ld\n", size);
//
//  return size;
//}
//
// Calculates array size
  static size_t
calc_size(variable* var)
{
  size_t size = 1;
  for(size_t i=0; i<var->nr_dimensions; ++i)
  {
    size *= var->dimensions[i];
  }

  // printf("array size %s = %ld\n", var->name, size);

  return size;
}


  static bool
check_in_bounds(variable* var, size_t* vector)
{
  // printf("check in bounds\n");
  // printf("dimensions %ld\n", var->nr_dimensions);
  for(size_t i=0; i<var->nr_dimensions; i++)
  {
    size_t vector_i = vector[i];
    // printf("vector_%ld = %ld\n", i, vector_i);
    // printf("dimension_%ld = %ld\n", i, var->dimensions[i]); 
    // DIM A(3) -> A(1), A(2), A(3)
    if (vector_i < 1 || vector_i > var->dimensions[i])
    { 
      return false;
    }  
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

  static size_t
calc_index(variable* var, size_t* vector)
{
  // printf("calc_index\n");
  // variable_dump(var);

  size_t index = 0;
  for(size_t i=0; i<var->nr_dimensions; ++i)
  {
    size_t product = vector[i] - 1;
    for(size_t j=i+1; j<var->nr_dimensions; ++j)
    {
      product *= var->dimensions[j];
    }
    index += product;
  }

  // printf("index[ %s", var->name);
  // for(size_t i=0; i<var->nr_dimensions; i++){
  //   printf("%ld", vector[i]);
  //   if (i<var->nr_dimensions-1) printf(",");
  // }
  // printf(") ] = %ld\n", index); 

  return index;
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
  // array_alloc(var->array, calc_size(var, var->nr_dimensions, -1)); 
  array_alloc(var->array, calc_size(var)); 
  dictionary_put(_dictionary, name, var);
  return var;
}

variable*
variable_array_set_string(char *name, char *value, size_t* vector)
{
  variable* var = dictionary_get(_dictionary, name);
  if ( ! check_in_bounds(var, vector) )
  {
    error("Index not in bounds");
  }
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
  if ( ! check_in_bounds(var, vector) )
  {
    error("Index not in bounds");
  }
  size_t index = calc_index(var, vector); 
  variable_value* val = array_get(var->array, index);
  return val->string;
}

variable*
variable_array_set_numeric(char *name, float value, size_t* vector)
{
  // printf("set numeric %s, %f, %ld\n", name, value, vector[0]);
  variable* var = dictionary_get(_dictionary, name);
  if ( ! check_in_bounds(var, vector) )
  {
    error("Index not in bounds");
  }

  // variable_dump(var);
  size_t index = calc_index(var, vector); 
  // printf("index = %ld\n", index);
  variable_value val;
  val.num = value;
  array_set(var->array, index, &val);
  // variable_dump(var);
  return var;
}

float
variable_array_get_numeric(char *name, size_t* vector)
{
  variable* var = dictionary_get(_dictionary, name);
  if ( ! check_in_bounds(var, vector) )
  {
    error("Index not in bounds");
  }
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

#if ARCH!=ARCH_XMEGA

  static void
calc_vector(variable* var, size_t index, size_t* vector)
{
  size_t product = 1;
  for(size_t i=1; i<var->nr_dimensions; ++i)
  {
    product *= var->dimensions[i];
  }

  for(int i=0; i<var->nr_dimensions; ++i)
  {
    vector[i] = index / product;
    index %= product;
    if( (i+1) < var->nr_dimensions )
    {
      product /= var->dimensions[i+1];
    }
  }
}  

  static void
vector_print(size_t* vector, size_t dimensions)
{
  for(size_t j=0; j<dimensions; j++)
  {
    printf("%ld", vector[j]);
    if ( j < dimensions - 1 )
    {
      printf(",");
    }
  }
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
      printf("\tdim %ld size = %ld\n", d, var->dimensions[d]);
    }
    printf("\tarray size: %ld\n", array_size(var->array));
    for(size_t i=0; i<array_size(var->array); i++)
    {
      size_t vector[5];
      calc_vector(var, i, vector);
      printf("\t%3ld %s", i, var->name);
      vector_print(vector, var->nr_dimensions);
      printf(") = ");
      variable_value* val = array_get(var->array, i);
      if (var->type == variable_type_string)
      {
        printf("%s\n", (val->string) ? val->string : "");
      }
      else
      {
        printf("%f\n", val->num); 
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
#endif
