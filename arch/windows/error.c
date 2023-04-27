#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../arch.h"


extern uint16_t __line;


const char *last_error = NULL;


void
error(const char *error_msg)
{
  last_error = error_msg;

  printf("--- ERROR: %d %s\n", __line, error_msg);
}
