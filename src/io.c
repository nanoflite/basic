/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Handle (console) input and output.
 *
 * Version:	@(#)io.c	1.1.1	2023/05/05
 *
 * Authors:	Fred N. van Kempen, <waltje@varcem.com>
 *		Johan Van den Brande <johan@vandenbrande.com>
 *
 *		Copyright 2023 Fred N. van Kempen.
 *		Copyright 2015,2016 Johan Van den Brande.
 *
 *		Redistribution and  use  in source  and binary forms, with
 *		or  without modification, are permitted  provided that the
 *		following conditions are met:
 *
 *		1. Redistributions of  source  code must retain the entire
 *		   above notice, this list of conditions and the following
 *		   disclaimer.
 *
 *		2. Redistributions in binary form must reproduce the above
 *		   copyright  notice,  this list  of  conditions  and  the
 *		   following disclaimer in  the documentation and/or other
 *		   materials provided with the distribution.
 *
 *		3. Neither the  name of the copyright holder nor the names
 *		   of  its  contributors may be used to endorse or promote
 *		   products  derived from  this  software without specific
 *		   prior written permission.
 *
 * THIS SOFTWARE  IS  PROVIDED BY THE  COPYRIGHT  HOLDERS AND CONTRIBUTORS
 * "AS IS" AND  ANY EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING, BUT  NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE  ARE  DISCLAIMED. IN  NO  EVENT  SHALL THE COPYRIGHT
 * HOLDER OR  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES;  LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON  ANY
 * THEORY OF  LIABILITY, WHETHER IN  CONTRACT, STRICT  LIABILITY, OR  TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING  IN ANY  WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "arch.h"
#include "basic.h"
#include "private.h"


/* Write one character to the BASIC console. */
void
bputc(int c)
{
    arch_putc(c);
}


/* Write one string to the BASIC console, followed by a newline. */
void
bputs(const char *ptr)
{
    if (ptr == NULL) {
#ifndef _WIN32
	fflush(stdout);
#endif
	return;
    }

    while (*ptr != '\0')
	arch_putc(*ptr++);

    arch_putc('\n');
}


/* Write a formatted string to the BASIC console. */
int
bprintf(const char *fmt, ...)
{
    char temp[1024];
    char *p = temp;
    va_list args;
    int i = 0;

    if (fmt == NULL) {
	fflush(stdout);
	return i;
    }

    va_start(args, fmt);
    i = vsprintf(temp, fmt, args);

    while (*p != '\0')
	arch_putc(*p++);

    return i;
}


/* Read one character from the BASIC console. Wait for it if so requested. */
int
bgetc(int wait)
{
    int ch;

    ch = arch_getc(wait);
    if (ch == 0) {
	/*
	 * Virtual keys are indicated by first returning
	 * a 0 code, and then the real (virtual) keycode.
	 *
	 * Since we do not need those keys, we just drop
	 * them.
	 */
	(void)arch_getc(wait);
    }

    /* Force NEWLINE to CR. */
    if (ch == '\n')
	ch = '\r';

    return ch;
}


/* Read a string from the BASIC console. */
char *
bgets(char *buffer, size_t buffer_size)
{
    size_t len = 0;
    char ch;

    while ((ch = bgetc(1)) != '\r' && len < buffer_size - 1) {
	/* Special keys return a 0x00 first. */
	if (ch == 0x00)
		ch = arch_getc(1);

	switch (ch) {
		case '\b':
		case 127:
			if (len > 0) {
				buffer[--len] = '\0';
				arch_putc('\b');
				arch_putc(' ');
				arch_putc('\b');
			}  
			break;

		default:
			buffer[len++] = ch;
			arch_putc(ch);
	}
    }
    buffer[len] = '\0';

    arch_putc('\n');

    return buffer;
}
