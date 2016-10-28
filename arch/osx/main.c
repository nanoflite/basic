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

  // basic_eval("10 FOR X=1 TO 30");
  // basic_eval("20 FOR Y=1 TO 30");
  // basic_eval("30 PRINT X, X");
  // basic_eval("40 NEXT Y");
  // basic_eval("50 NEXT X");
  // basic_eval("LIST");
  // basic_eval("RUN");
  // basic_eval("LIST");
 
  using_history();
 
   char *input;
   while ((input = readline_gets()) != NULL )
   {
     if (strcmp(input, "quit") == 0) {
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


  basic_destroy();

  // 
  // puts("");
  // puts("bye...");

  return EXIT_SUCCESS;
}
