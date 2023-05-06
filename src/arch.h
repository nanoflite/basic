/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Define things for the platform-support modules.
 *
 * Version:	@(#)arch.h	1.1.1	2023/05/05
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
#ifndef ARCH_H
# define ARCH_H


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


typedef void (*arch_load_cb)(const char *, void *);
typedef uint16_t (*arch_save_cb)(char **, void *);
typedef void (*arch_dir_cb)(const char *name, size_t, bool, void *);


#if PLATFORM == PLATFORM_XMEGA
extern int	asprintf(char **ret, const char *format, ...);
extern float	strtof(const char *restrict nptr, char **restrict endptr);
extern char	*strndup(const char *s1, size_t n);
#endif

extern int	arch_init(void);
extern void	arch_exit(void);
extern int	arch_load(const char *, arch_load_cb, void *);
extern int	arch_save(const char *, arch_save_cb, void *);
extern int	arch_delete(const char *);
extern int	arch_dir(arch_dir_cb, void *);
extern void	arch_sleep(int milliseconds);
extern int	arch_getc(int);
extern int	arch_putc(int);
extern void	arch_cls(void);
extern void	arch_locate(int, int);
extern void	arch_color(int, int);


#endif	/*ARCH_H*/
