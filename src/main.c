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

  line_read = readline ("expression> ");

  if (line_read && *line_read) {
    add_history (line_read);
  }

  return (line_read);
}

int main(int argc, char *argv[])
{

  puts("                                   _             ");
  puts("  _____  ___ __  _ __ ___  ___ ___(_) ___  _ __  ");
  puts(" / _ \\ \\/ / '_ \\| '__/ _ \\/ __/ __| |/ _ \\| '_ \\ ");
  puts("|  __/>  <| |_) | | |  __/\\__ \\__ \\ | (_) | | | |");
  puts(" \\___/_/\\_\\ .__/|_|  \\___||___/___/_|\\___/|_| |_|");
  puts("          |_|                                    ");
  puts("                    (c) 2014 Johan Van den Brande");

  char *input;
  while ((input = readline_gets()) != NULL ) {
    if (strcmp(input, "quit") == 0) {
      break;
    }
    evaluate_print(input);
    if (evaluate_last_error()) {
      printf("ERROR: %s\n", evaluate_last_error());
    }
  }
  
  puts("");
  puts("bye...");

  return EXIT_SUCCESS;
}
