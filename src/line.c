// -- Lines and BASIC program storage

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "line.h"

static line* __line = NULL;
static char* __memory = NULL;
static size_t __memory_size = 0;

  void
lines_init(char *memory, size_t memory_size)
{
  __memory = memory;
  __memory_size = memory_size; 
  lines_reset();
}

  void
lines_reset(void)
{
  __line = (line*) __memory;
}

  line*
lines_next(void)
{
  if (__line && __line->next_line)
  {
    __line = __line->next_line;
  }

  return NULL;
}

  line*
lines_get_by_number(size_t line_number)
{
  line *l = (line*) __memory;
  while ( l->number != line_number && l->next_line )
  {
    l = l->next_line;
  }
  
  if ( l->number == line_number )
  {
    return l;
  }

  return NULL;
}

  line*
lines_current(void)
{
  return __line;
}

  line*
lines_previous(line* current)
{
  line* previous = (line*) __memory;

  while ( previous->next_line && previous->next_line->number != current->number )
  {
    previous = previous->next_line;  
  }
  
  return previous;
}

  line*
lines_last(void)
{
  line* l = (line*) __memory;
  while (l->next_line) 
  {
    l = l->next_line;
  }

  return l;
}

  void
lines_set_by_number(size_t line_number)
{
  line* l = lines_get_by_number(line_number);
  if ( l != NULL )
  {
    __line = l;
  }
  else
  {
    // error condition
  }
}

  bool
lines_exists(size_t number)
{
  line* l = lines_get_by_number(number);
  return l != NULL;
}

  bool
lines_insert(size_t number, char *contents)
{
  line* last = lines_last();

  line* new = (line*) last + sizeof(line) + strlen(last->contents);

  size_t new_size = sizeof(line) + strlen(contents);
  char* end = (char*) new + new_size;

  if ( end - __memory > __memory_size )
  {
    return false;
  }

  new->number = number;
  new->next_line = NULL;
  new->deleted = false;
  strncpy(new->contents, contents, strlen(contents));
 
  return true; 
}

  void
lines_delete(size_t number)
{
  line* to_delete = lines_get_by_number(number);
  line* previous = lines_previous(to_delete);
  line* next = to_delete->next_line;

  previous->next_line = next;
  to_delete->deleted = true;  
  // Move everything down so we do not waste space
}

  void
lines_replace(size_t number, char *contents)
{
  lines_delete(number);
  lines_insert(number, contents);
}

  void
lines_store(size_t number, char* contents )
{
  if (lines_exists(number)) {
    lines_replace(number, contents);
  } else {
    lines_insert(number, contents);
  }
}


