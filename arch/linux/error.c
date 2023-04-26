#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdint.h>
#include "../arch.h"
// or inttypes.h


extern uint16_t __line;

const char *last_error = NULL;

  void
error(const char *error_msg)
{
#ifdef _DEBUG
  void *array[10];
  size_t size;
  char **strings;
  size_t i;
#endif

  last_error = error_msg;

  printf("--- ERROR: %d %s\n", __line, error_msg);

#ifdef _DEBUG
  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);

  printf ("SHOW %zd STACK FRAMES:\n", size);

  for (i = 0; i < size; i++)
  {
    printf ("  %s\n", strings[i]);
  }

  free (strings);
#endif
}
