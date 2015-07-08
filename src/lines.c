// -- Lines BASIC program storage

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hexdump.h"
#include "lines.h"

static char* __memory;
static size_t __memory_size;

/*
  static void
_terminate(line* l)
{
  char* p = (char*) l;
  p += sizeof(line) - 1 + l->length;
  line* end = (line*) p;
  end->number = 0;
  end->length = 0;
}
*/

  static line*
_next(line* l)
{
  char* p = (char*) l;
  p += sizeof(line) - 1 + l->length;
  return (line*) p;
}

  static bool
_is_end(line* l)
{
  return l && l->number == 0 && l->length == 0;
}

  static line*
_find_end(line* l)
{
  line* n = l;
  while ( ! _is_end( n ) )
  {
    n = _next( n );
  }
  return n;  
}

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
  // support insert


  char *p = __memory;
  line* l = (line*) p;
  while ( ! _is_end( l ) )
  {
    line* next = _next( l );

    // Find line that is to be insert after. That line has a line number < insert and the next line has a >
    if ( l->number < number && next->number > number )
    {
      // We need to insert
      printf("insert %d\n", number);
      
      // The address of the insert is the same as the next line
      line* insert = next;
     
      // But we need to move the memory block holding the rest to the right.
      line* end = _find_end( insert );
      end = _next(end); // Move to next empty slot (we keep the sentinel in the copy)
      
      // We have the end*,  calculate size to move
      char* m_src = (char*) insert;
      char* m_end = (char*) end;
      size_t m_size = m_end - m_src;
     
      // Calculate offset to move 
      size_t insert_size = sizeof(line) - 1 + strlen(contents) + 1;
      char* m_dst = m_src + insert_size;
      
      // Move the memory block
      memmove( m_dst, m_src, m_size );

      // Set the data of the insert
      insert->number = number;
      insert->length = strlen(contents) + 1;
      strcpy( &(insert->contents), contents );

      hexdump( "insert", __memory, 256 );
      
      return true;
    }
    // Replace
    if ( l->number == number )
    {
      printf("replace %d\n", number);
      
      // We need to shift the memory to the new offset determined by the size of the line to be inserted
      
      line* end = _find_end( l );
      end = _next(end); // Move to next empty slot (we keep the sentinel in the copy)
      
      // Calculate size of bloack
      char* m_src = (char*) next;
      char* m_end = (char*) end;
      size_t m_size = m_end - m_src;

      // Calculate offset to move 
      size_t replace_size = sizeof(line) - 1 + strlen(contents) + 1;
      size_t actual_size = sizeof(line) - 1 + strlen(&(l->contents)) + 1;
      int offset = replace_size - actual_size ;
      char* m_dst = m_src + offset;
      
      // Move the memory block
      memmove( m_dst, m_src, m_size );

      // Set the data of the replace
      l->length = strlen(contents) + 1;
      strcpy( &(l->contents), contents );

      hexdump( "replace", __memory, 256 );
      
      return true;
    }
    // Prepend
    if ( l->number > number )
    {
      printf("prepend %d\n", number);
      
      // The address of the insert is the same as the actual line
      line* insert = l;
     
      // But we need to move the memory block holding the rest to the right.
      line* end = _find_end( insert );
      end = _next(end); // Move to next empty slot (we keep the sentinel in the copy)
      
      // We have the end*,  calculate size to move
      char* m_src = (char*) insert;
      char* m_end = (char*) end;
      size_t m_size = m_end - m_src;
     
      // Calculate offset to move 
      size_t insert_size = sizeof(line) - 1 + strlen(contents) + 1;
      char* m_dst = m_src + insert_size;
      
      // Move the memory block
      memmove( m_dst, m_src, m_size );

      // Set the data of the insert
      insert->number = number;
      insert->length = strlen(contents) + 1;
      strcpy( &(insert->contents), contents );

      hexdump( "prepend", __memory, 256 );
 
      return true; 
    }

    l = next;
  }
  
  l->number = number;
  l->length = strlen(contents) + 1; // Length is offset to next line
  strcpy( &(l->contents), contents );
 
  line* end = _next( l );
  end->number = 0;
  end->length = 0;
 
  hexdump( "append", __memory, 256 );

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
  while( ! _is_end( l ) )
  {
    out(l->number, &(l->contents) );
    l = _next( l );
  }
}
