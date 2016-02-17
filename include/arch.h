#ifndef __ARCH_H__
#define __ARCH_H__

#if ARCH==ARCH_XMEGA
int asprintf(char **ret, const char *format, ...);
float strtof(const char *restrict nptr, char **restrict endptr);
char *strndup(const char *s1, size_t n);
#endif

#endif // __ARCH_H__
