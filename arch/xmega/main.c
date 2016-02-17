#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "parser.h"

int out(int ch)
{
  putchar(ch);
  return 1;
}

int in(void)
{
  return getchar();
}

int main(int argc, char *argv[])
{
  char memory[4096];
  char input[256];

  puts(" _               _      ");
  puts("| |__   __ _ ___(_) ___ ");
  puts("| '_ \\ / _` / __| |/ __|");
  puts("| |_) | (_| \\__ \\ | (__ ");
  puts("|_.__/ \\__,_|___/_|\\___|");
  puts("(c) 2015-2016 Johan Van den Brande");

  basic_init(memory, sizeof(memory), 512);
  basic_register_io(out, in);
  
  while(1)
  {
    basic_io_readline("", input, sizeof(input)); 
    basic_eval(input);
    if (evaluate_last_error()) {
      printf("ERR: %s\n", evaluate_last_error());
    }
  }
  
  return EXIT_SUCCESS;
}
