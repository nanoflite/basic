/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Definitions for the console driver.
 *
 * Version:	@(#)console.h	1.1.0	2023/05/05
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
#ifndef ARCH_CONSOLE_H
# define ARCH_CONSOLE_H


typedef enum Color {
    C_OFF = -1,
    C_BLACK = 0,		// RGB: 0,0,0
    C_BLUE,			// RGB: 0,0,128
    C_GREEN,			// RGB: 0,128,0
    C_CYAN,			// RGB: 0,128,128
    C_RED,			// RGB: 128,0,0
    C_MAGENTA,			// RGB: 128,0,128
    C_BROWN,			// RGB: 128,128,0
    C_GRAY,			// RGB: 128,128,128
    C_LIGHTGRAY,		// RGB: 0,0,0+I
    C_LIGHTBLUE,		// RGB: 0,0,128+I
    C_LIGHTGREEN,		// RGB: 0,128+I,0
    C_LIGHTCYAN,		// RGB: 0,128+I,128+I
    C_LIGHTRED,			// RGB: 128+I,0,0
    C_LIGHTMAGENTA,		// RGB: 128+I,0,128+I
    C_YELLOW,			// RGB: 128+I,128+I,0
    C_WHITE,			// RGB: 128+I,128+I,128+I
  
    C_MAX
} color_t;


extern int	con_init(void);
extern void	con_close(void);
extern int	con_getc(int);
extern int	con_putc(int);
extern int	con_printf(const char *, ...);
extern void	con_cls(void);
extern void	con_locate(int, int);
extern void	con_colors(color_t, color_t);
extern void	con_nocolors(void);

extern void	con_demo(void);


#endif	/*ARCH_CONSOLE_H*/
