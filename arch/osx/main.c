#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <readline/readline.h>
#include <string.h>

#include "parser.h"

static char *line_read = (char *) NULL;

static char *
readline_gets ()
{
  if (line_read) {
    free (line_read);
    line_read = (char *) NULL;
  }

  line_read = readline ("");

  if (line_read && *line_read) {
    add_history (line_read);
  }

  return (line_read);
}

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
  puts(" _               _      ");
  puts("| |__   __ _ ___(_) ___ ");
  puts("| '_ \\ / _` / __| |/ __|");
  puts("| |_) | (_| \\__ \\ | (__ ");
  puts("|_.__/ \\__,_|___/_|\\___|");
  puts("(c) 2015-2016 Johan Van den Brande");

  basic_init(1024*8, 2048);
  basic_register_io(out, in);
  
  char *input;
  while ((input = readline_gets()) != NULL )
  {
    if (strcmp(input, "quit") == 0) {
      break;
    }
    
    basic_eval(input);
    
    if (evaluate_last_error()) {
      printf("ERROR: %s\n", evaluate_last_error());
    }

  }
  
  puts("");
  puts("bye...");

  return EXIT_SUCCESS;
}
