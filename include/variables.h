#ifndef __VARIABLES_H__
#define __VARIABLES_H__

#include <stdbool.h>

typedef enum {
  variable_type_unknown,
  variable_type_numeric,
  variable_type_string
} variable_type;

typedef struct variable variable;

bool variables_init(void);
void variables_destroy(void);

variable* variable_get(char* name);

char* variable_get_string(char* name);
float variable_get_numeric(char* name);

variable* variable_set_string(char* name, char* value);
variable* variable_set_numeric(char* name, float value); 

variable_type variable_get_type(char* name);

#endif // __VARIABLES_H__
