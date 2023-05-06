/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Console driver for Windows "console mode" applications.
 *
 *		We support both the original Win32 "console" API (where we
 *		have to work directly with the Input and Output buffers of
 *		the console window), as well as the new "Virtual Terminal"
 *		mode introduced with Windows 10, where things are more like
 *		the UNIX "terminal" environment.
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
//#define UNICODE
#ifdef UNICODE
# undef UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include "../../arch.h"
#include "console.h"


static HANDLE	hStdIn = INVALID_HANDLE_VALUE,
		hStdOut = INVALID_HANDLE_VALUE;
static DWORD	oldmode;
static WORD		oldattr;

 
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


static void
_cls(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos = { 0, 0 };
    DWORD written, size;

    if (! GetConsoleScreenBufferInfo(hStdOut, &csbi))
        return;

    size = csbi.dwSize.X * csbi.dwSize.Y;

    if (! FillConsoleOutputCharacter(hStdOut,
				     (TCHAR)' ', size, pos, &written))
	return;

    if (! FillConsoleOutputAttribute(hStdOut,
				     csbi.wAttributes, size, pos, &written))
	return;

    SetConsoleCursorPosition(hStdOut, pos);
}


static void
_locate(int row, int col)
{
    COORD pos = { col, row };

    SetConsoleCursorPosition(hStdOut, pos);
}


static void
_color(int fg, int bg)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    WORD a;

    if (fg == C_OFF && bg == C_OFF) {
	(void)SetConsoleTextAttribute(hStdOut, oldattr);
	return;
    }

    if (! GetConsoleScreenBufferInfo(hStdOut, &csbi))
        return;
    a = csbi.wAttributes;

    if (fg != C_OFF) {
	a &= ~(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	switch (fg) {
		case C_BLACK:
			break;

		case C_BLUE:
			a |= FOREGROUND_BLUE;
			break;

		case C_GREEN:
			a |= FOREGROUND_GREEN;
			break;

		case C_RED:
			a |= FOREGROUND_RED;
			break;

		case C_CYAN:
			a |= (FOREGROUND_BLUE|FOREGROUND_GREEN);
			break;

		case C_MAGENTA:
			a |= (FOREGROUND_BLUE|FOREGROUND_RED);
			break;

		case C_BROWN:
			a |= (FOREGROUND_GREEN|FOREGROUND_RED);
			break;

		case C_GRAY:
			a |= (FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
			break;

		case C_LIGHTGRAY:
			a |= FOREGROUND_INTENSITY;
			break;

		case C_LIGHTBLUE:
			a |= (FOREGROUND_BLUE|FOREGROUND_INTENSITY);
			break;

		case C_LIGHTGREEN:
			a |= (FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			break;

		case C_LIGHTRED:
			a |= (FOREGROUND_RED|FOREGROUND_INTENSITY);
			break;

		case C_LIGHTCYAN:
			a |= (FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			break;

		case C_LIGHTMAGENTA:
			a |= (FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY);
			break;

		case C_YELLOW:
			a |= (FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
			break;

		case C_WHITE:
			a |= (FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
			break;
	}
    }

    if (bg != C_OFF) {
	a &= ~(BACKGROUND_RED|BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_INTENSITY);
	switch (bg) {
		case C_BLACK:
			break;

		case C_BLUE:
			a |= BACKGROUND_BLUE;
			break;

		case C_GREEN:
			a |= BACKGROUND_GREEN;
			break;

		case C_RED:
			a |= BACKGROUND_RED;
			break;

		case C_CYAN:
			a |= (BACKGROUND_BLUE|BACKGROUND_GREEN);
			break;

		case C_MAGENTA:
			a |= (BACKGROUND_BLUE|BACKGROUND_RED);
			break;

		case C_BROWN:
			a |= (BACKGROUND_GREEN|BACKGROUND_RED);
			break;

		case C_GRAY:
			a |= (BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED);
			break;

		case C_LIGHTGRAY:
			a |= BACKGROUND_INTENSITY;
			break;

		case C_LIGHTBLUE:
			a |= (BACKGROUND_BLUE|BACKGROUND_INTENSITY);
			break;

		case C_LIGHTGREEN:
			a |= (BACKGROUND_GREEN|BACKGROUND_INTENSITY);
			break;

		case C_LIGHTRED:
			a |= (BACKGROUND_RED|BACKGROUND_INTENSITY);
			break;

		case C_LIGHTCYAN:
			a |= (BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_INTENSITY);
			break;

		case C_LIGHTMAGENTA:
			a |= (BACKGROUND_BLUE|BACKGROUND_RED|BACKGROUND_INTENSITY);
			break;

		case C_YELLOW:
			a |= (BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_INTENSITY);
			break;

		case C_WHITE:
			a |= (BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_INTENSITY);
			break;
	}
    }

    (void)SetConsoleTextAttribute(hStdOut, a);
}


int
con_init(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD mode;

    if (hStdOut != INVALID_HANDLE_VALUE)
	return 0;

    hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if ((hStdIn == INVALID_HANDLE_VALUE) || (hStdOut == INVALID_HANDLE_VALUE))
	return (int)GetLastError();

    if (! GetConsoleMode(hStdOut, &oldmode)) {
	hStdOut = INVALID_HANDLE_VALUE;
	return (int)GetLastError();
    }

    /* Try to set the new VT mode. */
    mode = oldmode;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (! SetConsoleMode(hStdOut, mode)) {
	/* That did not work, must be Windows < Win10. */
	oldmode = 0xffffffff;

	/* Retrieve and save current attributes. */
	if (! GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
		hStdOut = INVALID_HANDLE_VALUE;
		return (int)GetLastError();
	}
	oldattr = csbi.wAttributes;
    }

    return 0;
}


void
con_close(void)
{
    if (oldmode != 0xffffffff) {
	con_nocolors();
	(void)SetConsoleMode(hStdOut, oldmode);
    } else
	(void)SetConsoleTextAttribute(hStdOut, oldattr);

    hStdIn = hStdOut = INVALID_HANDLE_VALUE;
}


/* Read one character from the console. Wait for it if so requested. */
int
con_getc(int wait)
{
    static int virtkey = 0;
    INPUT_RECORD ir;
    DWORD count;
    int ch = -1;

    /* We had a virtual key last time; return that code now. */
    if (virtkey != 0) {
	ch = virtkey;
	virtkey = 0;

	return ch;
    }

    do {
	/* Get the number of input events available. */
	if (! GetNumberOfConsoleInputEvents(hStdIn, &count))
		break;

	/* If we have events, read (just) one. */
	if (count == 0)
		continue;

	/* Is it a KeyEvent with KeyDown set? */
	if (! ReadConsoleInput(hStdIn, &ir, 1, &count))
		break;
	if (ir.EventType != KEY_EVENT)
		continue;
	if (! ir.Event.KeyEvent.bKeyDown)
		continue;

	/* Grab the ASCII character. */
	ch = ir.Event.KeyEvent.uChar.AsciiChar;
	if (ch == 0) {
		/*
		 * If we have a virtual key pressed, save it and
		 * return 0. We return the virtual key on the
		 * next call, compatible with the old _getch().
		 */
		if (ir.Event.KeyEvent.wVirtualKeyCode != 0)
			virtkey = ir.Event.KeyEvent.wVirtualKeyCode;
	}

	break;
    } while (wait);

    return ch;
}


int
con_putc(int ch)
{
    DWORD written = 0;

    if (hStdOut == INVALID_HANDLE_VALUE)
	con_init();

    (void)WriteConsole(hStdOut, &ch, 1, &written, NULL);

    return 1;
}


int
con_printf(const char *fmt, ...)
{
    char buff[1024];
    DWORD written = 0;
    va_list args;
    int i;

    if (hStdOut == INVALID_HANDLE_VALUE)
	con_init();

    va_start(args, fmt);
    i = vsprintf(buff, fmt, args);
    va_end(args);

    if (! WriteConsole(hStdOut, buff, (DWORD)strlen(buff), &written, NULL))
        return (int)GetLastError();

    return i;
}


void
con_cls(void)
{
    if (hStdOut == INVALID_HANDLE_VALUE)
	con_init();

    if (oldmode != 0xffffffff) {
	con_printf("\033[2J");
	con_locate(0, 0);
    } else 
	_cls();
}


void
con_locate(int y, int x)
{
    if (hStdOut == INVALID_HANDLE_VALUE)
	con_init();

    if (oldmode == 0xffffffff)
	_locate(y, x);
    else
	con_printf("\033[%u;%uH", y+1, x+1);
}


void
con_colors(int fg, int bg)
{
    if (hStdOut == INVALID_HANDLE_VALUE)
	con_init();

    if (fg == C_OFF && bg == C_OFF) {
	con_nocolors();
	return;
    }

    if (oldmode != 0xffffffff) {
	if (fg == C_OFF) {
		con_printf("\033[%im", bgmap[bg]);
	} else if (bg == C_OFF) {
		con_printf("\033[%im", fgmap[fg]);
	} else
		con_printf("\033[%i;%im", fgmap[fg], bgmap[bg]);
    } else
	_color(fg, bg);
}


void
con_nocolors(void)
{
    if (hStdOut == INVALID_HANDLE_VALUE)
	con_init();

    if (oldmode == 0xffffffff)
	_color(-1, -1);
    else
	con_printf("\033[0m");  
}


#if 0
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
