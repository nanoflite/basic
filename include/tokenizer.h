#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

typedef enum {
  // 0
  T_PLUS,
  T_MINUS,
  T_MULTIPLY,
  T_DIVIDE,
  T_LEFT_BANANA,
  T_RIGHT_BANANA,
  T_COLON,
  T_SEMICOLON,
  T_EQUALS,
  T_LESS,
  // 10
  T_GREATER,
  T_NUMBER,
  T_STRING,
  T_FUNC_ABS,
  T_FUNC_SIN,
  T_FUNC_COS,
  T_FUNC_RND,
  T_FUNC_INT,
  T_FUNC_TAN,
  T_FUNC_SQR,
  // 20
  T_FUNC_SGN,
  T_FUNC_LOG,
  T_FUNC_EXP,
  T_FUNC_ATN,
  T_FUNC_NOT,
  T_STRING_FUNC_CHR,
  T_STRING_FUNC_MID$,
  T_OP_OR,
  T_OP_AND,
  T_KEYWORD_PRINT,
  T_KEYWORD_GOTO,
  // 30
  T_KEYWORD_IF,
  T_KEYWORD_THEN,
  T_KEYWORD_LET,
  T_KEYWORD_INPUT,
  T_KEYWORD_GOSUB,
  T_KEYWORD_RETURN,
  T_KEYWORD_CLEAR,
  T_KEYWORD_LIST,
  T_KEYWORD_RUN,
  T_KEYWORD_END,
  // 40
  T_KEYWORD_FOR,
  T_KEYWORD_TO,
  T_KEYWORD_STEP,
  T_KEYWORD_NEXT,
  T_VARIABLE_NUMBER,
  T_VARIABLE_STRING,
  T_COMMA,
  T_ERROR,
  T_EOF,
  T_THE_END
} token;

void tokenizer_init(char *input);
token tokenizer_get_next_token(void);

float tokenizer_get_number(void);
char * tokenizer_get_string(void);
char * tokenizer_get_variable_name(void);

char *tokenizer_token_name(token);

char* tokenizer_char_pointer(char* set);

#endif // __TOKENIZER_H__
