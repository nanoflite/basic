#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


int
kbhit(void)
{
//FIXME: we could use _kbhit() here ..
  return getchar();
}
