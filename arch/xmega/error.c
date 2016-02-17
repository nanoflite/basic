#include <stdlib.h>
#include <stdio.h>

#include <io.h>

extern uint16_t __line;

const char *last_error = NULL;

  void
error(const char *error_msg)
{
  char buffer[80];

  last_error = error_msg;
  snprintf(buffer, sizeof(buffer), "ERR: %d %s\n", __line, error_msg);
  basic_io_print(buffer);
}

