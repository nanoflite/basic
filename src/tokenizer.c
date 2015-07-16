#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include "tokenizer.h"

#define std_token_entry(t, k) { t, #t, k }

static token_entry tokens[] = {
 std_token_entry( T_ERROR, NULL ),
 std_token_entry( T_EOF, NULL ),
 std_token_entry( T_NUMBER, NULL ),
 std_token_entry( T_STRING, NULL ),
 std_token_entry( T_VARIABLE_STRING, NULL ),
 std_token_entry( T_VARIABLE_NUMBER, NULL ),
 std_token_entry( T_PLUS, "+" ),
 std_token_entry( T_MINUS, "-" ),
 std_token_entry( T_MULTIPLY, "*" ),
 std_token_entry( T_DIVIDE, "/" ),
 std_token_entry( T_LEFT_BANANA, "(" ),
 std_token_entry( T_RIGHT_BANANA, ")" ),
 std_token_entry( T_COLON, ":" ),
 std_token_entry( T_SEMICOLON, ";" ),
 std_token_entry( T_EQUALS, "=" ),
 std_token_entry( T_LESS, "<" ),
 std_token_entry( T_GREATER, ">" ),
 std_token_entry( T_COMMA, "," ),
 std_token_entry( T_THE_END, NULL )
};

static token_entry* extra_tokens = NULL;

/*
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
  { ':', T_COLON },
  { ';', T_SEMICOLON },
  { '=', T_EQUALS },
  { '<', T_LESS },
  { '>', T_GREATER },
  { ',', T_COMMA },
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
  { "PRINT", T_KEYWORD_PRINT },
  { "GOTO", T_KEYWORD_GOTO },
  { "IF", T_KEYWORD_IF },
  { "THEN", T_KEYWORD_THEN },
  { "LET", T_KEYWORD_LET },
  { "INPUT", T_KEYWORD_INPUT },
  { "GOSUB", T_KEYWORD_GOSUB },
  { "RETURN", T_KEYWORD_RETURN },
  { "FOR", T_KEYWORD_FOR },
  { "TO", T_KEYWORD_TO },
  { "STEP", T_KEYWORD_STEP },
  { "NEXT", T_KEYWORD_NEXT },
  { "CLEAR", T_KEYWORD_CLEAR },
  { "LIST", T_KEYWORD_LIST },
  { "RUN", T_KEYWORD_RUN },
  { "END", T_KEYWORD_END },
  { "CHR$", T_STRING_FUNC_CHR },
  { "MID$", T_STRING_FUNC_MID$ },
  { NULL,  T_EOF }

};
*/

char *tokenizer_line = NULL;
char *tokenizer_p = NULL;
char *tokenizer_next_p = NULL;

token tokenizer_actual_token;
float tokenizer_actual_number;
char tokenizer_actual_char;
char *tokenizer_actual_string = NULL;
char *tokenizer_actual_variable = NULL;

void tokenizer_init(char *input)
{
  tokenizer_line = input;
  tokenizer_p = tokenizer_next_p = tokenizer_line;
}

char* tokenizer_char_pointer(char* set)
{
  if ( set != NULL )
  {
    tokenizer_p = set; 
    return NULL;
  }

  // Skip white space
  while ( *tokenizer_p && isspace(*tokenizer_p) ) {
    tokenizer_p++;
  } 
  return tokenizer_p;
}

static bool
isvarchar(char c)
{

  if (c >= 'A' && c <= 'Z') {
    return true;
  }

  if ( c == '$' ) {
    return true;
  }

  return false;
}

token _find(token_entry* tokens)
{
  for(size_t i=0;; i++) {
    token_entry entry = tokens[i];
    if ( entry.token == T_THE_END ) break;

    if (strncmp(tokenizer_p, entry.keyword, strlen(entry.keyword)) == 0) {
       printf("found '%s'\n", entry.keyword);
       tokenizer_next_p = tokenizer_p + strlen(entry.keyword);
       tokenizer_p = tokenizer_next_p;
       return entry.token;
    }
  }
  return T_THE_END;
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
  // for(size_t i=0;;i++) {
  //   char_to_token ctt = char_to_tokens[i];
  //   if (ctt._char == '\0') {
  //     break;
  //   }
  //   if (*tokenizer_p == ctt._char) {
  //     tokenizer_actual_char = ctt._char;
  //     tokenizer_actual_number = NAN;
  //     tokenizer_actual_token = ctt._token;

  //     tokenizer_p++;
  //     tokenizer_next_p = tokenizer_p;
  //      
  //     return ctt._token;
  //   }
  // } 

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

  // Check for string
  if ( '"' == *tokenizer_p ) {
    // puts("read string");
    tokenizer_p++; // skip "
    tokenizer_next_p = tokenizer_p;
    size_t l=0;
    while(*tokenizer_next_p && '"' != *tokenizer_next_p) {
      l++;
      tokenizer_next_p++;
    }

    if (*tokenizer_next_p) {
      tokenizer_next_p++; // skip trailing "
    }

    if (tokenizer_actual_string != NULL) {
      free(tokenizer_actual_string);
    }
    tokenizer_actual_string = strndup(tokenizer_p, l);
   
    tokenizer_p = tokenizer_next_p;

    return T_STRING; 
  }

  // Check for function
  // for(size_t i=0;;i++) {
  //   keyword_to_token ktt = keyword_to_tokens[i];
  //   if (ktt._keyword == NULL) {
  //     break;
  //   }
  //   if (strncmp(tokenizer_p, ktt._keyword, strlen(ktt._keyword)) == 0) {
  //     // printf("%s\n",ktt._keyword);
  //     tokenizer_next_p = tokenizer_p + strlen(ktt._keyword);
  //     tokenizer_p = tokenizer_next_p;
  //     return ktt._token;
  //   }
  // 
  // }
 //  
 //  for(size_t i=0;; i++) {
 //    token_entry entry = tokens[i];
 //    if ( entry.token == T_THE_END ) break;

 //    if (strncmp(tokenizer_p, entry.keyword, strlen(entry.keyword)) == 0) {
 //       printf("found '%s'\n", entry.keyword);
 //       tokenizer_next_p = tokenizer_p + strlen(entry.keyword);
 //       tokenizer_p = tokenizer_next_p;
 //       return entry.token;
 //    }
 //  }
  token t;
  t = _find( tokens );
  if ( t != T_THE_END ) return t;
  
  if ( extra_tokens )
  {
    t = _find( extra_tokens );
    if ( t != T_THE_END ) return t;
  }

  // Check for variable
  tokenizer_next_p = tokenizer_p;
  size_t len = 0;
  while(*tokenizer_next_p && isvarchar(*tokenizer_next_p)) {
    len++;
    tokenizer_next_p++;
  }

  if (len > 0) {
    tokenizer_actual_variable = strndup(tokenizer_p, len);
    tokenizer_p = tokenizer_next_p;
    if (tokenizer_actual_variable[len-1] == '$') {
      return T_VARIABLE_STRING;
    }
    return T_VARIABLE_NUMBER;
  }

  return T_ERROR; 
}

float tokenizer_get_number(void)
{
  return tokenizer_actual_number;
}

char *tokenizer_get_string(void)
{
  return tokenizer_actual_string;
}

char *tokenizer_get_variable_name(void)
{
  return tokenizer_actual_variable;
}

/*
typedef struct {
  token t;
  char *l;
} token_to_label;

#define t2l_entry(t) { t, #t }

char *tokenizer_token_name(token t)
{
  token_to_label t2l_table[] = {
  t2l_entry( T_PLUS ),
  t2l_entry( T_MINUS ),
  t2l_entry( T_MULTIPLY ),
  t2l_entry( T_DIVIDE ),
  t2l_entry( T_LEFT_BANANA ),
  t2l_entry( T_RIGHT_BANANA ),
  t2l_entry( T_COLON ),
  t2l_entry( T_SEMICOLON ),
  t2l_entry( T_EQUALS ),
  t2l_entry( T_LESS ),
  t2l_entry( T_GREATER ),
  t2l_entry( T_NUMBER ),
  t2l_entry( T_STRING ),
  t2l_entry( T_FUNC_ABS ),
  t2l_entry( T_FUNC_SIN ),
  t2l_entry( T_FUNC_COS ),
  t2l_entry( T_FUNC_RND ),
  t2l_entry( T_FUNC_INT ),
  t2l_entry( T_FUNC_TAN ),
  t2l_entry( T_FUNC_SQR ),
  t2l_entry( T_FUNC_SGN ),
  t2l_entry( T_FUNC_LOG ),
  t2l_entry( T_FUNC_EXP ),
  t2l_entry( T_FUNC_ATN ),
  t2l_entry( T_FUNC_NOT ),
  t2l_entry( T_STRING_FUNC_CHR ),
  t2l_entry( T_STRING_FUNC_MID$ ),
  t2l_entry( T_OP_OR ),
  t2l_entry( T_OP_AND ),
  t2l_entry( T_KEYWORD_PRINT ),
  t2l_entry( T_KEYWORD_GOTO ),
  t2l_entry( T_KEYWORD_IF ),
  t2l_entry( T_KEYWORD_THEN ),
  t2l_entry( T_KEYWORD_LET ),
  t2l_entry( T_KEYWORD_INPUT ),
  t2l_entry( T_KEYWORD_GOSUB ),
  t2l_entry( T_KEYWORD_RETURN ),
  t2l_entry( T_KEYWORD_CLEAR ),
  t2l_entry( T_KEYWORD_LIST ),
  t2l_entry( T_KEYWORD_RUN ),
  t2l_entry( T_KEYWORD_END ),
  t2l_entry( T_KEYWORD_FOR ),
  t2l_entry( T_KEYWORD_TO ),
  t2l_entry( T_KEYWORD_STEP ),
  t2l_entry( T_KEYWORD_NEXT ),
  t2l_entry( T_VARIABLE_NUMBER ),
  t2l_entry( T_VARIABLE_STRING ),
  t2l_entry( T_COMMA ),
  t2l_entry( T_ERROR ),
  t2l_entry( T_EOF ),

    { -1, "" }
  };

  size_t i = 0;
  while(1) {
    token_to_label t2l = t2l_table[i];
    if (t2l.t == t) {
      return t2l.l;
    }
    if (t2l.t == T_THE_END) {
      break;
    }  
    i++;
  }
  
  return "";
}
*/

/*
  void
tokenizer_register_token( token token, token_name name, token_keyword keyword )
{
  if ( tokens_index >= sizeof(tokens) )
  {
    printf("error: no more token space.\n");
    return;
  }
  token_entry* t = &(tokens[tokens_index]);
  t->token = token;
  t->name = name;
  t->keyword = keyword;
}
*/
  void
tokenizer_add_tokens( token_entry* tokens )
{
  extra_tokens = tokens;
}
