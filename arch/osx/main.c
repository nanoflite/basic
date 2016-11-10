#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>

#include "parser.h"

static char *
readline_gets ()
{
  char * line_read = readline ("");

  // if (line_read && *line_read) {
  //   add_history (line_read);
  // }

  return line_read;
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

void repl(void)
{
  puts(" _               _      ");
  puts("| |__   __ _ ___(_) ___ ");
  puts("| '_ \\ / _` / __| |/ __|");
  puts("| |_) | (_| \\__ \\ | (__ ");
  puts("|_.__/ \\__,_|___/_|\\___|");
  puts("(c) 2015-2016 Johan Van den Brande");

  using_history();
 
  char *input;
  while ((input = readline_gets()) != NULL )
  {
    if (strcmp(input, "QUIT") == 0) {
      free(input);
      break;
    }
    
    basic_eval(input);
    
    if (evaluate_last_error()) {
      printf("ERROR: %s\n", evaluate_last_error());
    }

    free(input);
  }

  clear_history();
}

void run(char *file_name){
  FILE* file = fopen(file_name, "r");

  if (file == NULL) {
    fprintf(stderr, "Can't open %s\n", file_name);
    return;  
  }  

  char line[256];
  if (fgets(line, sizeof(line), file)) { // Skip shebang
    while (fgets(line, sizeof(line), file)) {
      if(strlen(line)==1 && line[0]=='\n') continue;
      if(line[strlen(line)-1] == '\n'){
        line[strlen(line)-1] = '\0';
      }
      basic_eval(line);
    }
  }
  fclose(file);

  basic_run();
}

int main(int argc, char *argv[])
{
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
