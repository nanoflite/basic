#ifndef __ARRAY_H__
#define __ARRAY_H__

typedef struct array array;

array* array_new(size_t element_size);

void array_destroy(array* array);

void* array_push(array* array, void* value);

void* array_get(array* array, size_t index);

size_t array_size(array* array);

#endif // __ARRAY_H__
