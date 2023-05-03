/*
 * VARCem	Virtual ARchaeological Computer EMulator.
 *		An emulator of (mostly) x86-based PC systems and devices,
 *		using the ISA,EISA,VLB,MCA  and PCI system buses, roughly
 *		spanning the era between 1981 and 1995.
 *
 *		This file is part of the VARCem Project.
 *
 *		Portable GETOPT(3) function.
 *
 *		This is a simple version of the UNIX-standard GetOpt(3)
 *		library routine.  We don't support any of the GNU LibC
 *		enhancements such as long-name options, double-dash
 *		options and such.  We keep it simple and small here.
 *
 * Version:	@(#)getopt.c	1.0.1	2002/08/13
 *
 * Author:	Fred N. van Kempen, <waltje@varcem.com>
 *
 *		Copyright 1998-2018 MicroWalt Corporation
 *		Copyright 2018-2023 The VARCem Team
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
#include <stdio.h>
#include <string.h>
#include "getopt.h" 


char *optarg;
int optind = 0;
int opterr = 1;
int optopt;


static char *scan = (char *)NULL;


int
getopt(int argc, char **argv, const char *optstring)
{
    char c;
    char *place;

    optarg = (char *)NULL;

    if (scan == (char *)NULL || *scan == '\0') {
	if (optind == 0) optind++;

	if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
		return(EOF);
	if (strcmp(argv[optind], "--") == 0) {
		optind++;
		return(EOF);
	}
	scan = argv[optind] + 1;
	optind++;
    }
    optopt = c = *scan++;
    place = strchr(optstring, c);

    if (place == (char *)NULL || c == ':') {
	if (opterr) fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
	return('?');
    }
    place++;
    if (*place == ':') {
	if (*scan != '\0') {
		optarg = scan;
		scan = (char *)NULL;
	} else if (optind < argc) {
		optarg = argv[optind];
		optind++;
	} else {
		if (opterr)
			fprintf(stderr,
				"%s: -%c argument missing\n", argv[0], c);
		return('?');
	}
    }
    return(c);
}
