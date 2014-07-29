#ifndef __PARSER_H__
#define __PARSER_H__

float evaluate(char *expression_string);

void evaluate_print(char *line);

void evaluate_print_func_param( char *func, float param);

const char *evaluate_last_error(void);

#endif // __PARSER_H__
