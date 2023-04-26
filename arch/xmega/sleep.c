#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#if ARCH == ARCH_XMEGA
# include <util/delay.h>
#endif
#include "../arch.h"


void
delay_ms(uint16_t count)
{
  while(count--) {
    _delay_ms(1)
  }
}


void
arch_sleep(int msec)
{
  delay_ms(msec);
}
