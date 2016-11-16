#include <stdio.h>

#include "io.h"

  void
console_def_char(unsigned char code, char* definition)
{
  char buf[8];
  putchar(0x1b);
  putchar('T');
  snprintf(buf, sizeof(buf), "%02x", code);
  basic_io_print(buf);
  basic_io_print(definition);
}

  void
console_plot(int x, int y, unsigned char code)
{
  putchar(0x1b);
  putchar('Y');
  putchar(' ' + x);
  putchar(' ' + y);
  putchar(code);
}

  void
console_cursor(int cursor)
{
  putchar(0x1b);
  if(cursor){
    putchar('e');
  } else {
    putchar('f');
  }
}

  void
console_cursor_type(int block)
{
  putchar(0x1b);
  if(block){
    putchar('x');
  } else {
    putchar('y');
  }
  putchar('4');
} 
