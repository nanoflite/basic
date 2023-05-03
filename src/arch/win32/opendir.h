/*
 * VARCem	Virtual ARchaeological Computer EMulator.
 *		An emulator of (mostly) x86-based PC systems and devices,
 *		using the ISA,EISA,VLB,MCA  and PCI system buses, roughly
 *		spanning the era between 1981 and 1995.
 *
 *		This file is part of the VARCem Project.
 *
 *		Definitions for the platform OpenDir module.
 *
 * Version:	@(#)win_opendir.h	1.0.6	2021/06/08
 *
 * Author:	Fred N. van Kempen, <waltje@varcem.com>
 *
 *		Copyright 2017-2022 Fred N. van Kempen.
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
#ifndef WIN_OPENDIR_H
# define WIN_OPENDIR_H


#ifdef UNICODE
# define XCHAR		wchar_t
#else
# define XCHAR		char
#endif

#ifdef _MAX_FNAME
# define MAXNAMLEN	_MAX_FNAME
#else
# define MAXNAMLEN	255
#endif
#define MAXDIRLEN	255


struct direct {
    long		d_ino;
    unsigned short 	d_reclen;
    unsigned short	d_off;
    XCHAR		d_name[MAXNAMLEN+1];
};
#define	d_namlen	d_reclen


typedef struct {
    short	flags;			/* internal flags		*/
    short	offset;			/* offset of entry into dir	*/
    intptr_t	handle;			/* open handle to Win32 system	*/
    short	sts;			/* last known status code	*/
    char	*dta;			/* internal work data		*/
    XCHAR	dir[MAXDIRLEN+1];	/* open dir			*/
    struct direct dent;			/* actual directory entry	*/
} DIR;


/* Directory routine flags. */
#define DIR_F_LOWER	0x0001		/* force to lowercase		*/
#define DIR_F_SANE	0x0002		/* force this to sane path	*/
#define DIR_F_ISROOT	0x0010		/* this is the root directory	*/


/* Function prototypes. */
extern DIR		*opendir(const XCHAR *);
extern struct direct	*readdir(DIR *);
extern long		telldir(DIR *);
extern void		seekdir(DIR *, long);
#define rewinddir(dirp)	seekdir(dirp, 0L)
extern int		closedir(DIR *);


#endif	/*WIN_OPENDIR_H*/
