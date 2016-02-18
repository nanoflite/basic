#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

  int
asprintf(char **str, const char *format, ...)
{
  va_list argp;
  va_start(argp, format);

  char one_char[1];
  int len = vsnprintf(one_char, 1, format, argp);
  if (len < 1){
    *str = NULL;
    return len;
  }
  va_end(argp);
  *str = malloc(len+1);
  if (!str) {
    return -1;
  }
  va_start(argp, format);
  vsnprintf(*str, len+1, format, argp);
  va_end(argp);
  return len;
}

  float
strtof(const char *restrict nptr, char **restrict endptr)
{
  float f;
  sscanf(nptr, "%f", &f);
  return f;
}

  char*
strndup(const char *s, size_t n)
{
  size_t len = strnlen (s, n);
  char *new = (char *) malloc (len + 1);
  if (new == NULL)
  {
    return NULL;
  }
  new[len] = '\0';
  return (char *) memcpy (new, s, len);
}
