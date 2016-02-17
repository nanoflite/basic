#include <stdlib.h>
#include <string.h>

#include <io.h>

extern basic_putchar __putch;
extern basic_getchar __getch;

  void
basic_io_print(char* buffer)
{
    for(size_t i=0; i<strlen(buffer); ++i)
    {
      __putch(buffer[i]);
    }
}  

char*
basic_io_readline(char* prompt, char* buffer, size_t buffer_size)
{
  size_t len = 0;
  char ch;
  basic_io_print(prompt);
  while ((ch = __getch()) != '\n' && len < buffer_size - 1)
  { 
    buffer[len++] = ch;
  }
  buffer[len] = '\0';
  return buffer;
}

