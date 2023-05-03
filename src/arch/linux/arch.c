/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Platform support for Linux (and similar *IX-ish systems.)
 *
 * Version:	@(#)arch.c	1.1.0	2023/05/01
 *
 * Authors:	Fred N. van Kempen, <waltje@varcem.com>
 *		Aaron Clarking, <clarkaaron@hotmail.com>
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
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "../../arch.h"
#include "../../basic.h"


int
arch_init(void)
{
    return 0;
}


static char *
_get_path(void)
{
    static char *_path = NULL;

    _path = getenv("BASIC_PATH");
    if (_path == NULL)
	_path = ".";
    if (_path[strlen(_path)-1] == '/')
	_path[strlen(_path)-1] = '\0';

    return _path;
}  


int
arch_load(const char *name, arch_load_cb cb, void *context)
{
    char filename[1024];
    char line[256];
    FILE *fp;

    sprintf(filename, "%s/%s.bas", _get_path(), name);
    fp = fopen(filename, "r");
    if (fp == NULL)
	return 1;

    while (fgets(line, 256, fp) != NULL)
	cb(line, context);
    (void)fclose(fp);

    return 0;
}


int
arch_save(const char *name, arch_save_cb cb, void *context)
{
    char filename[1024];
    char *line;
    FILE *fp; 

    sprintf(filename, "%s/%s.bas", _get_path(), name);
    fp = fopen(filename, "w"); 
    if (fp == NULL)
	return 1;

    for (;;) {
	uint16_t number = cb(&line, context);

	if (line == NULL)
		break;
	fprintf(fp, "%i %s\n",number, line);
    }
    (void)fclose(fp);

    return 0;
}


int
arch_dir(arch_dir_cb cb, void *context)
{
    char out[512];
    struct stat stats;
    struct dirent *ent;
    DIR *dir;

    snprintf(out, sizeof(out), "dir: %s", _get_path());
    cb(out, 0, true, context);

    dir = opendir(_get_path());
    if (dir == NULL)
	return 1;

    while ((ent = readdir(dir)) != NULL) {
	char *name = ent->d_name;

	if (strlen(name) > 4) {
		char *ext = name + strlen(name) - 4;
		if (! strncmp(ext, ".bas", 4)) { 
			snprintf(out,sizeof(out),"%s/%s", _get_path(), name);
			stat(out, &stats);
			name[strlen(name)-4] = '\0';

			cb(name, stats.st_size, false, context);
		}
	}
    }
    (void)closedir(dir);

    return 0;
}


int
arch_delete(const char *name)
{
    char filename[1024];

    sprintf(filename, "%s/%s.bas", _get_path(), name);

    (void)remove(filename);

    return 0;
}


void
arch_sleep(int msec)
{
    struct timespec ts;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
