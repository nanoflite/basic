#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

  static void
delay_us(size_t duration)
{
  while(duration--)
  {
    _delay_us(1);
  }
}  

  void
sound_init(void)
{
  PORTE.DIRSET = PIN0_bm;
}  

  void
sound_play(float freq, float duration)
{
  size_t wait_us = (size_t) 1000000.0 / ( 2 * freq );    
  size_t counter = (size_t) duration / wait_us;

  printf("beep stuff wait_us: %d, counter: %d\n", wait_us, counter);

  while (counter)
  {
    PORTE.OUTCLR = PIN0_bm;
    delay_us((size_t)wait_us);
    PORTE.OUTSET = PIN0_bm;
    delay_us(wait_us);
    counter--;
  }
}
