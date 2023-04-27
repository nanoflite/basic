#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../arch/arch.h"


void
hexdump(char *desc, void *addr, int len)
{
  uint8_t buff[17];
  uint8_t *pc = (uint8_t*)addr;
  int i;

  if (desc != NULL)
  {
    printf ("%s:\n", desc);
  }

  for (i = 0; i < len; i++)
  {
    if ( (i % 16) == 0 )
    {
      if ( i != 0 )
      {
        printf ("  %s\n", buff);
      }
      printf ("  %04x ", i);
    }

    printf (" %02x", pc[i]);

    if ( (pc[i] < 0x20) || (pc[i] > 0x7e) )
    {
      buff[i % 16] = '.';
    }
    else
    {
      buff[i % 16] = pc[i];
    }
    buff[(i % 16) + 1] = '\0';
  }

  while ( (i % 16) != 0 )
  {
    printf ("   ");
    i++;
  }
  printf ("  %s\n", buff);
}
