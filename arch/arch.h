#ifndef __ARCH_H__
# define __ARCH_H__


/* Some compilers define or, or the other, or both.. */
#if defined(WIN32) && !defined(_WIN32)
# define _WIN32
#endif

#ifdef _WIN32
# define C_STRDUP       _strdup
#else
# define C_STRDUP       strdup
#endif


#define PLATFORM_WIN32	1
#define PLATFORM_LINUX	2
#define PLATFORM_OSX	3
#define PLATFORM_XMEGA	10


#ifdef _WIN32
# define PLATFORM	PLATFORM_WIN32
#endif


typedef void (*arch_load_out_cb)(char *line, void* context);
typedef uint16_t (*arch_save_cb)(char** line, void* context);
typedef void (*arch_dir_out_cb)(char *name, size_t size, bool label, void* context);


#if PLATFORM == PLATFORM_XMEGA
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
