#ifndef F_CPU
#   define F_CPU 2000000UL
#endif

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


// Hz, sec
  void
sound_play(float freq, float duration)
{
//     __    __    __
//  __|  |__|  |__|  |_...
//  <-T->
//
//   . wait T/2
//   . counter = duration / T
//
  float T = 1 / freq;
  float _t2 = T / 2;
  float _counter = duration / _t2;
  size_t counter = (size_t) _counter;
  size_t wait_us = (size_t) 1000000 * _t2;
  
  while (counter)
  {
    PORTE.OUTCLR = PIN0_bm;
    delay_us(wait_us);
    PORTE.OUTSET = PIN0_bm;
    delay_us(wait_us);
    counter--;
  }
}
