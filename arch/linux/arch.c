#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../arch.h"


int
arch_init(void)
{
  // stub
  return 0;
}


static char *
_get_path(void)
{
  static char *_path = NULL;

  _path = getenv("BASIC_PATH");
  if (_path == NULL) {
    _path = ".";
  }
  if(_path[strlen(_path)-1] == '/'){
    _path[strlen(_path)-1] = '\0';
  }

  return _path;
}  


int
arch_load(char *name, arch_load_out_cb cb, void *context)
{
  char line[256];
  char *filename;
  FILE *fp;

  asprintf(&filename, "%s/%s.bas", _get_path(), name);
  fp = fopen(filename, "r");
  if(!fp){
    return 1;
  }
  while(fgets(line, 256, fp) != NULL) {
    cb(line, context);
  }
  fclose(fp);
  free(filename);

  return 0;
}


int
arch_save(char *name, arch_save_cb cb, void *context)
{
  char *filename;
  char *line;
  FILE *fp; 

  asprintf(&filename, "%s/%s.bas", _get_path(), name);
 
  fp = fopen(filename, "w"); 
  if(!fp){
    return 1;
  }
  for(;;){
    uint16_t number = cb(&line, context);
    if (line == NULL){
      break;
    }
    fprintf(fp, "%d %s\n",number, line);
  }
  fclose(fp);

  free(filename);

  return 0;
}


int
arch_dir(arch_dir_out_cb cb, void *context)
{
  char out[512];
  struct stat stats;
  struct dirent *ent;
  DIR *dir;

  snprintf(out, sizeof(out), "dir: %s", _get_path());
  cb(out, 0, true, context);

  dir = opendir(_get_path());
  while ((ent = readdir(dir)) != NULL) {
    char* name = ent->d_name;
    if (strlen(name)>4){
      char *ext = name + strlen(name) - 4;
      if (strncmp(ext, ".bas", 4)==0){ 
        snprintf(out,sizeof(out),"%s/%s", _get_path(), name);
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
arch_delete(char *name)
{
  char *filename;

  asprintf(&filename, "%s/%s.bas", _get_path(), name);
  remove(filename);
  free(filename);

  return 0;
}
