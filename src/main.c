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

/*
float expansion_do_led(float param)
{
  printf("LED %s", (param > 0) ? "ON" : "OFF" );
  return 1;
}
*/

int main(int argc, char *argv[])
{
  char memory[4096];

  puts(" _               _      ");
  puts("| |__   __ _ ___(_) ___ ");
  puts("| '_ \\ / _` / __| |/ __|");
  puts("| |_) | (_| \\__ \\ | (__ ");
  puts("|_.__/ \\__,_|___/_|\\___|");
  puts("(c) 2015 Johan Van den Brande");

  // basic_register_function("LED", expansion_do_led, basic_returns_numeric, 1, basic_param_numeric);
  basic_init(memory, sizeof(memory), 512);

  char *input;
  while ((input = readline_gets()) != NULL ) {

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
