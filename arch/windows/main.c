#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#ifdef USE_READLINE
# include <readline/readline.h>
# include <readline/history.h>
#endif
#include "../arch/arch.h"
#include "../../src/tokenizer.h"
#include "../../src/io.h"
#include "../../src/parser.h"


extern bool __RUNNING;
extern bool __STOPPED;


static void
sigint_handler(int signum)
{
  signal(SIGINT, sigint_handler);
  if(__RUNNING){
    __RUNNING = false;
    __STOPPED = true;
    printf("STOP\n");
    fflush(stdout);
  }
}


int
out(int ch)
{
  putchar(ch);

  return 1;
}


int
in(void)
{
  return getchar();
}


void
repl(void)
{
#ifndef USE_READLINE
  char line[1024];
#endif
  char *input;
 
  puts(" _               _      ");
  puts("| |__   __ _ ___(_) ___ ");
  puts("| '_ \\ / _` / __| |/ __|");
  puts("| |_) | (_| \\__ \\ | (__ ");
  puts("|_.__/ \\__,_|___/_|\\___|");
  puts("(c) 2015-2016 Johan Van den Brande");

#ifdef USE_READLINE
  using_history();
#else
#endif

  for (;;) {
#ifdef USE_READLINE
    input = readline("");
    if (input == NULL)
        break;
#else
    memset(line, 0, sizeof(line));
    input = fgets(line, sizeof(line), stdin);
    if (ferror(stdin) || feof(stdin))
        break;
    if (input == NULL)
        continue;
    line[strlen(line) - 1] = '\0';
#endif

    if (input && *input) {
#ifdef USE_READLINE
      add_history(input);
#endif

      if (strcmp(input, "QUIT") == 0) {
#ifndef USE_READLINE
        memset(line, 0, sizeof(line));
#endif
        break;
      }

      basic_eval(input);

      if (evaluate_last_error()) {
        printf("ERROR: %s\n", evaluate_last_error());
        clear_last_error();
      }
    } else
      input = NULL;
  }

#ifdef USE_READLINE
  clear_history();
#endif
}


void
run(char *file_name)
{
  char line[tokenizer_string_length];
  FILE *file = fopen(file_name, "r");

  if (file == NULL) {
    fprintf(stderr, "Can't open %s\n", file_name);
    return;  
  }  

  while (fgets(line, sizeof(line), file)) {
    if (line[strlen(line)-1] != '\n')
    {
      printf("ERROR: NO EOL\n");
      exit(1);      
    }
    basic_eval(line);
  }
  fclose(file);

  basic_run();
}


int
main(int argc, char *argv[])
{
  signal(SIGINT, sigint_handler);

  basic_init(1024*8, 2048);
  basic_register_io(out, in);

  if (argc > 1){
    run(argv[1]);
  } else {  
    repl();
  }

  basic_destroy();

  return EXIT_SUCCESS;
}
