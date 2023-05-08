#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "../arch.h"


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
    char line[256];
    char *filename;
    FILE *fp;

    asprintf(&filename, "%s/%s.bas", _get_path(), name);
    fp = fopen(filename, "r");
    if (fp == NULL)
	return 1;

    while(fgets(line, sizeof(line), fp) != NULL)
	cb(line, context);

    (void)fclose(fp);

    free(filename);

    return 0;
}


int
arch_save(consy char *name, arch_save_cb cb, void *context)
{
    char *line, *filename;
    uint16_t number;
    FILE *fp;

    asprintf(&filename, "%s/%s.bas", _get_path(), name);
 
    fp = fopen(filename, "w"); 
    if (fp == NULL)
	return 1;

    for (;;) {
	number = cb(&line, context);
	if (line == NULL)
		break;

	fprintf(fp, "%i %s\n", number, line);
    }
    (void)fclose(fp);

    free(filename);

    return 0;
}


int
arch_dir(arch_dir_cb cb, void *context)
{
    char out[256];
    struct stat stats;
    struct dirent *ent;
    char *name, *ext;
    DIR *dir;

    snprintf(out, sizeof(out), "dir: %s", _get_path());
    cb(out, 0, true, context);

    dir = opendir(_get_path());
    while ((ent = readdir(dir)) != NULL) {
	name = ent->d_name;
	if (strlen(name) > 4) {
		ext = name + strlen(name) - 4;
		if (! strncmp(ext, ".bas", 4)) { 
			snprintf(out, sizeof(out), "%s/%s", _get_path(), name);
			stat(out, &stats);
			name[strlen(name)-4] = '\0';

			cb(name, stats.st_size, false, context);
		}
	}
    }
    closedir(dir);

    return 0;
}


int
arch_delete(const char *name)
{
    char *filename;

    asprintf(&filename, "%s/%s.bas", _get_path(), name);

    (void)remove(filename);

    free(filename);

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
