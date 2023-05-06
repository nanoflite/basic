/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Driver for the console interface.
 *
 * Version:	@(#)console.c	1.1.0	2023/05/05
 *
 * Author:	Fred N. van Kempen, <waltje@varcem.com>
 *
 *		Copyright 2023 Fred N. van Kempen.
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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "../../arch.h"
#include "../../basic.h"
#include "console.h"


static struct termios oldterm;
static int		hStdIn = -1,
			hStdOut = -1;

static const int8_t fgmap[] = {
    30,		// 00, black
    34,		// 01, blue
    32,		// 02, green
    36,		// 03, cyan (aqua)
    31,		// 04, red
    35,		// 05, magenta (purple)
    33,		// 06, brown (dark yellow)
    37,		// 07, light gray (dark white)
    90,		// 08, dark gray
    94,		// 09, light blue
    92,		// 10, light green
    96,		// 11, light cyan (aqua)
    91,		// 12, light red
    95,		// 13, light magenta (purple)
    93,		// 14, yellow
    97		// 15, white
};
static const int8_t bgmap[] = {
    40,		// 00, black
    44,		// 01, blue
    42,		// 02, green
    46,		// 03, cyan (aqua)
    41,		// 04, red
    45,		// 05, magenta (purple)
    43,		// 06, brown (dark yellow)
    47,		// 07, light gray (dark white)
    100,	// 08, dark gray
    104,	// 09, light blue
    102,	// 10, light green
    106,	// 11, light cyan (aqua)
    101,	// 12, light red
    105,	// 13, light magenta (purple)
    103,	// 14, yellow
    107		// 15, white
};


int
con_init(void)
{
    struct termios term;

    if (hStdIn != -1)
	return 0;

    hStdIn = 0;
    hStdOut = 1;

    tcgetattr(hStdIn, &oldterm);
    tcgetattr(hStdIn, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(hStdIn, TCSANOW, &term);

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    return 0;
}



void
con_close(void)
{
    if (hStdIn != -1)
	tcsetattr(hStdIn, TCSANOW, &oldterm);

    hStdIn = hStdOut = -1;
}


/* Read one character from the console. Wait for it if so requested. */
int
con_getc(int wait)
{
    int count = 0;
    int ch = -1;

    if (hStdIn == -1)
	con_init();

    do {
	/* Get the number of input events available. */
	if (ioctl(hStdIn, FIONREAD, &count) < 0)
		break;

	/* If we have events, read (just) one. */
	if (count == 0)
		continue;

	/* Read the ASCII character. */
	if (read(hStdIn, &ch, 1) != 1)
		break;

	/* Make sure it is 8-bit. */
	ch &= 0xff;

	break;
    } while (wait);

    return ch;
}


int
con_putc(int ch)
{
    if (hStdOut == -1)
	con_init();

#if 0
    write(1, &ch, 1);
#else
    putchar(ch);
//    fflush(stdout);
#endif

    return 1;
}


int
con_printf(const char *fmt, ...)
{
    va_list args;
    int i;

    if (hStdOut == -1)
	con_init();

    va_start(args, fmt);
    i = vprintf(fmt, args);
    va_end(args);

    fflush(stdout);

    return i;
}


void
con_cls(void)
{
    if (hStdOut == -1)
	con_init();

    con_printf("\033[2J");
    con_printf("\033[0;0H");
}


void
con_locate(int y, int x)
{
    if (hStdOut == -1)
	con_init();

    con_printf("\033[%u;%uH", y+1, x+1);
}


void
con_colors(int fg, int bg)
{
    if (hStdOut == -1)
	con_init();

    if (fg == C_OFF && bg == C_OFF) {
	con_nocolors();
	return;
    }

    if (fg == C_OFF) {
	con_printf("\033[%im", bgmap[bg]);
    } else if (bg == C_OFF) {
	con_printf("\033[%im", fgmap[fg]);
    } else
	con_printf("\033[%i;%im", fgmap[fg], bgmap[bg]);
}


void
con_nocolors(void)
{
    if (hStdOut == -1)
	con_init();

    con_printf("\033[0m");
}


#if 1
void
con_demo(void)
{
    int fg, bg;

    for (fg = 0; fg < 8; fg++) {
	for (bg = 0; bg < 8; bg++) {
		con_colors(fg, bg);
		printf("%2i , %2i", fg, bg);
		con_nocolors();
		printf(" ");
	}
	printf("\n");
    }

    printf("\n");

    for (fg = 0; fg < 8; fg++) {
	for (bg = 0; bg < 8; bg++) {
		con_colors(fg+8, bg+8);
		printf("%2i , %2i", fg+8, bg+8);
		con_nocolors();
		printf(" ");
	}
	printf("\n");
    }
}
#endif
