#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "../../arch.h"
#include "spi.h"
#include "ff.h"


int
asprintf(char **str, const char *format, ...)
{
    char one_char[1];
    va_list argp;

    va_start(argp, format);

    int len = vsnprintf(one_char, 1, format, argp);
    if (len < 1){
	va_end(argp);
	*str = NULL;

	return len;
    }

    *str = malloc(len+1);
    if (*str == NULL) {
	va_end(argp);

	return -1;
    }

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


char *
strndup(const char *s, size_t n)
{
    size_t len = strnlen (s, n);
    char *new = (char *) malloc (len + 1);

    if (new == NULL)
	return NULL;

    new[len] = '\0';

    return (char *) memcpy (new, s, len);
}


// -- SD Card specific
FATFS FatFs;


int
arch_init(void)
{
    spi_init();

    f_mount(&FatFs, "", 0); 

    return 0;
}


int
arch_load(const char *name, arch_load_cb cb, void *context)
{
    char filename[13]; // 8 + '.' + 3 + '\0'
    char line[128];
    FIL fil;

    snprintf(filename, sizeof(filename), "%s.bas", name);
    f_open(&fil, filename, FA_READ);

    while (f_gets(line, sizeof line, &fil))
	cb(line, context);

    f_close(&fil);

    return 0;
}


int
arch_save(const char *name, arch_save_cb cb, void *context)
{
    char filename[13];
    char buffer[128], *line;
    FIL fil;

    snprintf(filename, sizeof(filename), "%s.bas", name);
    f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS); 

    for (;;) {
	uint16_t number = cb(&line, context);
	if (line == NULL)
		break;
	snprintf(buffer, sizeof(buffer), "%i %s\n", number, line);
	f_puts(buffer, &fil);
    }

    f_close(&fil);

    return 0;
}


int
arch_dir(arch_dir_cb cb, void *context)
{
    char out[24];
    FRESULT res;
    FILINFO fno;
    DIR dir;

    f_getlabel("", out, 0);
    cb(out, 0, true, context);

    res = f_findfirst(&dir, &fno, "", "*.bas");
    while (res == FR_OK && fno.fname[0]) {
	fno.fname[strlen(fno.fname)-4] = '\0';
	cb(fno.fname, (size_t) fno.fsize, false, context);

	res = f_findnext(&dir, &fno);
    }

    f_closedir(&dir);

    return 0;
}


int
arch_delete(const char *name)
{
    char filename[13];

    snprintf(filename, sizeof(filename), "%s.bas", name);
    (void)f_unlink(filename);

    return 0;
}


void
delay_ms(uint16_t count)
{
    while(count--) {
	_delay_ms(1)
    }
}


void
arch_sleep(int msec)
{
    delay_ms(msec);
}
