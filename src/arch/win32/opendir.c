/*
 * VARCem	Virtual ARchaeological Computer EMulator.
 *		An emulator of (mostly) x86-based PC systems and devices,
 *		using the ISA,EISA,VLB,MCA  and PCI system buses, roughly
 *		spanning the era between 1981 and 1995.
 *
 *		This file is part of the VARCem Project.
 *
 *		Implementation POSIX OpenDir(3) and friends for Win32 API.
 *
 *		Based on old original code @(#)dir_win32.c 1.2.0 2007/04/19
 *
 * Version:	@(#)win_opendir.c	1.0.10	2021/11/27
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
//#define UNICODE
#ifdef UNICODE
# undef UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef UNICODE
# include <wchar.h>
#endif
#include "../../arch.h"
#include "../../basic.h"
#include "opendir.h"


#ifdef UNICODE
# define SUFFIX		L"\\*"
# define FINDATA	struct _wfinddata_t
# define FINDFIRST	_wfindfirst
# define FINDNEXT	_wfindnext
# define X_LEN		wcslen
# define X_CAT		wcscat
# define X_CPY		wcscpy
# define X_NPY		wcsncpy
#else
# define SUFFIX		"\\*"
# define FINDATA	struct _finddata_t
# define FINDFIRST	_findfirst
# define FINDNEXT	_findnext
# define X_LEN		strlen
# define X_CAT		strcat
# define X_CPY		strcpy
# define X_NPY		strncpy
#endif


/* Open a directory. */
DIR *
opendir(const XCHAR *name)
{
    DIR *p;

    /* Create a new control structure. */
    p = (DIR *)malloc(sizeof(DIR));
    if (p == NULL)
	return NULL;
    memset(p, 0x00, sizeof(DIR));
    p->flags = DIR_F_LOWER | DIR_F_SANE;
    p->offset = 0;
    p->sts = 0;

    /* Create a work area. */
    p->dta = (char *)malloc(sizeof(FINDATA));
    if (p->dta == NULL) {
	free(p);
	return NULL;
    }

    /* Add search filespec. */
    X_CPY(p->dir, name);
    X_CAT(p->dir, SUFFIX);

    /* Special case: flag if we are in the root directory. */
    if (X_LEN(p->dir) == 3)
	p->flags |= DIR_F_ISROOT;

    /* Start the searching by doing a FindFirst. */
    p->handle = FINDFIRST(p->dir, (FINDATA *)p->dta);
    if (p->handle < 0L) {
	free(p->dta);
	free(p);
	return NULL;
    }

    /* All OK. */
    return p;
}


/* Close an open directory. */
int
closedir(DIR *p)
{
    if (p == NULL)
	return 0;

    if (p->handle)
	_findclose(p->handle);

    if (p->dta != NULL)
	free(p->dta);
    free(p);

    return 0;
}


/*
 * Read the next entry from a directory.
 *
 * Note that the DOS (FAT), Windows (FAT, FAT32) and Windows NTFS
 * file systems do not have a root directory containing the UNIX-
 * standard "." and ".." entries.  Many applications do assume
 * this anyway, so we simply fake these entries.
 */
struct direct *
readdir(DIR *p)
{
    FINDATA *ffp;
    CHAR *foo;

    if (p == NULL || p->sts == 1)
	return NULL;

    /* Format structure with current data. */
    ffp = (FINDATA *)p->dta;
    foo = ffp->name;	// avoid GCC warning
    p->dent.d_ino = 1L;
    p->dent.d_off = p->offset++;
    switch (p->offset) {
	case 1:		/* . */
		X_NPY(p->dent.d_name, ".", sizeof(p->dent.d_name) - 1);
		p->dent.d_reclen = 1;
		break;

	case 2:		/* .. */
		X_NPY(p->dent.d_name, "..", sizeof(p->dent.d_name) - 1);
		p->dent.d_reclen = 2;
		break;

	default:	/* regular entry. */
		X_NPY(p->dent.d_name, foo, sizeof(p->dent.d_name) - 1);
		p->dent.d_reclen = (unsigned short)X_LEN(p->dent.d_name);
    }

    /* Read next entry. */
    p->sts = 0;

    /* Fake the "." and ".." entries here.. */
    if ((p->flags & DIR_F_ISROOT) && (p->offset <= 2))
	return &(p->dent);

    /* Get the next entry if we did not fake the above. */
    if (FINDNEXT(p->handle, ffp) < 0)
	p->sts = 1;

    return &(p->dent);
}


/* Report current position within the directory. */
long
telldir(DIR *p)
{
    return(p->offset);
}


void
seekdir(DIR *p, long newpos)
{
    short pos;

    /* First off, rewind to start of directory. */
    p->handle = FINDFIRST(p->dir, (FINDATA *)p->dta);
    if (p->handle < 0L) {
	p->sts = 1;
	return;
    }
    p->offset = 0;
    p->sts = 0;

    /* If we are rewinding, that's all... */
    if (newpos == 0L)
	return;

    /* Nope.. read entries until we hit the right spot. */
    pos = (short)newpos;
    while (p->offset != pos) {
	p->offset++;
	if (FINDNEXT(p->handle, (FINDATA *)p->dta) < 0) {
		p->sts = 1;
		return;
	}
    }
}
