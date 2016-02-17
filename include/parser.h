#ifndef __PARSER_H__
#define __PARSER_H__

#include <io.h>

float evaluate(char *expression_string);

void evaluate_print(char *line);

void evaluate_print_func_param( char *func, float param);

const char *evaluate_last_error(void);

void basic_init(char* memory, size_t memory_size, size_t stack_size);

void basic_register_io(basic_putchar putch, basic_getchar getch);

char* basic_readline(char* prompt, char* buffer, size_t buffer_size);
void basic_eval(char *line);

#endif // __PARSER_H__
