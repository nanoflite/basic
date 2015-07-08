// -- Lines BASIC program storage

#include <stdlib.h>
#include <string.h>

#include "lines.h"

static char* __memory;
static size_t __memory_size;

  void
lines_init(char *memory, size_t memory_size)
{
  __memory = memory;
  __memory_size = memory_size;
 
  // Signal end 
  line* l = (line*) __memory;
  l->number = 0;
  l->length = 0;
}

  bool
lines_store(uint16_t number, char* contents)
{
  // store 1 line
  line* l = (line*) __memory;
  l->number = number;
  l->length = strlen(contents) + 1; // Length is offset to next line
  strcpy( &(l->contents), contents );
  
  return true;
}

  bool
lines_delete(uint16_t number)
{
  // delete first line
  line* l = (line*) __memory;
  l->number = 0;
  l->length = 0;
  strcpy( &(l->contents), "" );

  return true;
}

  void
lines_list(lines_list_cb out)
{
  char *p = __memory;
  
  line* l = (line*) p;
  while( l->number && l->length )
  {
    out(l->number, &(l->contents) );
    p += sizeof(line) + l->length;
    l = (line*) p;
  }
}
