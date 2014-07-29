#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>

#include "tokenizer.h"

typedef struct {
  char _char;
  token _token;
} char_to_token;

char_to_token char_to_tokens[] = 
{

  { '+', T_PLUS },
  { '-', T_MINUS },
  { '*', T_MULTIPLY },
  { '/', T_DIVIDE },
  { '(', T_LEFT_BANANA },
  { ')', T_RIGHT_BANANA },
  { '\0', T_EOF }

};

typedef struct {
  char *_keyword;
  token _token;
} keyword_to_token;

keyword_to_token keyword_to_tokens[] =
{

  { "ABS", T_FUNC_ABS },
  { "SIN", T_FUNC_SIN },
  { "COS", T_FUNC_COS },
  { "RND", T_FUNC_RND },
  { "INT", T_FUNC_INT },
  { "TAN", T_FUNC_TAN },
  { "SQR", T_FUNC_SQR },
  { "SGN", T_FUNC_SGN },
  { "LOG", T_FUNC_LOG },
  { "EXP", T_FUNC_EXP },
  { "ATN", T_FUNC_ATN },
  { "NOT", T_FUNC_NOT },
  { "OR",  T_OP_OR },
  { "AND", T_OP_AND },
  { NULL,  T_EOF }

};

char *tokenizer_line = NULL;
char *tokenizer_p = NULL;
char *tokenizer_next_p = NULL;

token tokenizer_actual_token;
float tokenizer_actual_number;
char tokenizer_actual_char;

void tokenizer_init(char *input)
{
  tokenizer_line = input;
  tokenizer_p = tokenizer_next_p = tokenizer_line;
}

token tokenizer_get_next_token(void)
{
  if ( ! *tokenizer_p ) {
    return T_EOF;
  } 

  // Skip white space
  while ( *tokenizer_p && isspace(*tokenizer_p) ) {
    tokenizer_p++;
  } 

  // read single char token
  for(size_t i=0;;i++) {
    char_to_token ctt = char_to_tokens[i];
    if (ctt._char == '\0') {
      break;
    }
    if (*tokenizer_p == ctt._char) {
      tokenizer_actual_char = ctt._char;
      tokenizer_actual_number = NAN;
      tokenizer_actual_token = ctt._token;

      tokenizer_p++;
      tokenizer_next_p = tokenizer_p;
       
      return ctt._token;
    }
  } 

  // Check for number
  if (isdigit(*tokenizer_p)) {
    // puts("read a number");
    tokenizer_next_p = tokenizer_p;
    size_t l=0;
    while (*tokenizer_next_p && ( isdigit(*tokenizer_next_p) || *tokenizer_next_p == '.') ) {
      l++;
      tokenizer_next_p++;
    }
    char number[l+1];
    strlcpy(number, tokenizer_p, sizeof(number) );
    tokenizer_p = tokenizer_next_p;
    float f;
    sscanf(number, "%f", &f);
    // printf("Got float: '%f'\n", f);
    tokenizer_actual_number = f;
    return T_NUMBER;
  }

  // Check for function
  for(size_t i=0;;i++) {
    keyword_to_token ktt = keyword_to_tokens[i];
    if (ktt._keyword == NULL) {
      break;
    }
    if (strncmp(tokenizer_p, ktt._keyword, strlen(ktt._keyword)) == 0) {
      // printf("%s\n",ktt._keyword);
      tokenizer_next_p = tokenizer_p + strlen(ktt._keyword);
      tokenizer_p = tokenizer_next_p;
      return ktt._token;
    }
  
  }

  return T_ERROR; 
}

float tokenizer_get_number(void)
{
  return tokenizer_actual_number;
}
