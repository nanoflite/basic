#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

typedef enum {
  T_PLUS,
  T_MINUS,
  T_MULTIPLY,
  T_DIVIDE,
  T_LEFT_BANANA,
  T_RIGHT_BANANA,
  T_NUMBER,
  T_FUNC_ABS,
  T_FUNC_SIN,
  T_FUNC_COS,
  T_FUNC_RND,
  T_FUNC_INT,
  T_FUNC_TAN,
  T_FUNC_SQR,
  T_FUNC_SGN,
  T_FUNC_LOG,
  T_FUNC_EXP,
  T_FUNC_ATN,
  T_OP_OR,
  T_ERROR,
  T_EOF
} token;

void tokenizer_init(char *input);
token tokenizer_get_next_token(void);

float tokenizer_get_number(void);

#endif // __TOKENIZER_H__
