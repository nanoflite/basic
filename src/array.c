#include <stdlib.h>
#include <string.h>

#include "array.h"

struct array {
  size_t element_size;
  size_t size;
  char* ptr;
};

  array*
array_new(size_t element_size)
{
  array* array = malloc(sizeof(array));  
  array->element_size = element_size;
  array->size = 0;
  array->ptr = NULL;
  return array;
}

  void
array_destroy(array* array)
{
  free(array->ptr);
  free(array);
}

  void
array_push(array* array, void* value)
{
  array->size++;
  realloc((char*)array->ptr, array->element_size * array->size);
  void* element = array->ptr + array->element_size * (array->size - 1);
  memcpy(element, value, array->element_size);
}

  void*
array_get(array* array, size_t index)
{
  return array->ptr + index * array->element_size;
}

  size_t
array_size(array* array)
{
  return array->size;
}

