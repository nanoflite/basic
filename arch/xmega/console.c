#include <stdio.h>

#include "io.h"

  void
console_def_char(unsigned char code, char* definition)
{
  printf("\x1bT%02x%s", code, definition);
}

  void
console_plot(int x, int y, unsigned char code)
{
  putchar(0x1b);
  putchar('Y');
  putchar(' ' + x);
  putchar(' ' + y);
}

