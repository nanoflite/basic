#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdlib.h>

void
joystick_init(void)
{
  PORTA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;
}

int
joystick_read_button(void){
  return PORTA.IN & PIN0_bm;
}

int
joystick_read_stick(void){
  uint8_t stick = PORTA.IN;
  
  // u,r,d,l
  // 0 1 2 3
  stick >>= 1;

  return stick;
}
