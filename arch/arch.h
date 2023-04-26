#ifndef __ARCH_H__
# define __ARCH_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


typedef enum archs {
    ARCH_WINDOWS = 1,
    ARCH_LINUX,
    ARCH_OSX,

    ARCH_XMEGA
} archs_t;


typedef void (*arch_load_out_cb)(char *line, void* context);
typedef uint16_t (*arch_save_cb)(char** line, void* context);
typedef void (*arch_dir_out_cb)(char *name, size_t size, bool label, void* context);


#if ARCH == ARCH_XMEGA
int asprintf(char **ret, const char *format, ...);
float strtof(const char *restrict nptr, char **restrict endptr);
char *strndup(const char *s1, size_t n);
#endif

int arch_init(void);

int arch_load(char* filename, arch_load_out_cb cb, void* context);

int arch_save(char* filename, arch_save_cb cb, void* context);

int arch_dir(arch_dir_out_cb cb, void* context);

int arch_delete(char* filename);

void arch_sleep(int milliseconds);


#endif	/*__ARCH_H__*/
