#include <avr/io.h>
#include <avr/interrupt.h>
#include <ctype.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "parser.h"
#include "sound.h"
#include "joystick.h"
#include "console.h"

#include "diskio.h"

extern uint16_t __line;

// 100 Hz
ISR(TCC0_OVF_vect)
{
  disk_timerproc();
  sound_timerproc();
}

void init_xtal(void)
{
  // Use an external 16Mhz crystal and x 2 PLL to give a clock of 32Mhz

  // Enable the external oscillator
  OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc ;
  OSC.CTRL |= OSC_XOSCEN_bm ;
  while( (OSC.STATUS & OSC_XOSCRDY_bm) == 0 ){} // wait until stable

  // Now configure the PLL to be eXternal OSCillator * 2
  OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | 2 ;
  OSC.CTRL |= OSC_PLLEN_bm ; // enable the PLL...
  while( (OSC.STATUS & OSC_PLLRDY_bm) == 0 ){} // wait until stable

  // And now switch to the PLL as the clocksource
  CCP = CCP_IOREG_gc; // protected write follows
  CLK.CTRL = CLK_SCLKSEL_PLL_gc;
}

void set_usartctrl( USART_t *usart, uint8_t bscale, uint16_t bsel)
{
  usart->BAUDCTRLA = (bsel & USART_BSEL_gm);
  usart->BAUDCTRLB = ((bscale << USART_BSCALE0_bp) & USART_BSCALE_gm) | ((bsel >> 8) & ~USART_BSCALE_gm);
}

void init_uart_bscale_bsel(USART_t *usart, int8_t bscale, int16_t bsel)
{
  PORTE.DIRSET = PIN3_bm;
  PORTE.DIRCLR = PIN2_bm;
  usart->CTRLA = 0;
  usart->CTRLB = USART_TXEN_bm | USART_RXEN_bm;
  usart->CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  set_usartctrl(usart, bscale, bsel);
}


int _uart_putc(int ch)
{
  while ( ! (USARTE0.STATUS & USART_DREIF_bm) ) {};
  USARTE0.DATA = (char) ch;
  return 1;
}


int uart_putc(int ch)
{
  _uart_putc(ch);
  if (ch == '\n')
  {
    _uart_putc('\r');
  }
  return 1;
}

void uart_puts(unsigned char* s)
{
  while(*s){
    uart_putc(*s);
    s++;
  }
}

int uart_getc(void)
{
  while ( ! (USARTE0.STATUS & USART_RXCIF_bm) ) {};
  char ch = (char) USARTE0.DATA;
  return ch;
}

int uart_fputc(char c, FILE* stream)
{
  uart_putc(c);
  return 0;
}

int uart_fgetc(FILE* stream)
{
  return uart_getc();
}

FILE uart_stdio = FDEV_SETUP_STREAM(uart_fputc, uart_fgetc, _FDEV_SETUP_RW);

void init_xmega(void)
{
  // init_uart_bscale_bsel(&USARTC1, -7, 1539); // 9K6 @ 2MHz
  // init_uart_bscale_bsel(&USARTC1, -7, 705); // 19K2 @ 2MHz
  init_uart_bscale_bsel(&USARTE0, -5, 3301); // 19K2 @ 32MHz
  // init_uart_bscale_bsel(&USARTE0, -7, 2049); // 115200 @ 32 MHz
  stdout = stdin = &uart_stdio;

  _delay_ms(500);

  _uart_putc(0x1b);
  _uart_putc('E');
}

  static int
do_joystick(basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = joystick_read_stick(); 
  return 0;
}  

  static int
do_button(basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = joystick_read_button(); 
  return 0;
}  

  static int
do_led(basic_type* status, basic_type* rv)
{
  if(status->value.number>0){
    PORTE.OUTSET = PIN0_bm;
  } else {
    PORTE.OUTCLR = PIN0_bm;
  }  
  rv->kind = kind_numeric;
  rv->value.number = 0; 
  return 0;
}  

  static int
do_sound(basic_type* freq, basic_type* duration, basic_type* rv)
{
  sound_play((uint16_t)freq->value.number, (uint16_t)(1000*duration->value.number));
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

  static int
do_plot(basic_type* x, basic_type* y, basic_type* code, basic_type* rv)
{
  console_plot((int)x->value.number, (int)y->value.number, (unsigned char)code->value.number);
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

  static int
do_defchar(basic_type* code, basic_type* definition, basic_type* rv)
{
  console_def_char((unsigned char)code->value.number,definition->value.string);
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}  

  static int
do_cursor(basic_type* cursor, basic_type* rv)
{
  console_cursor((int)cursor->value.number);
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

int main(int argc, char *argv[])
{
  char input[64];

  init_xtal();
  init_xmega();
  sound_init();
  joystick_init();

  PORTE.DIRSET = PIN0_bm; // LED
  PORTE.OUTSET = PIN0_bm; //  on

  // timer
  TCC0.CTRLB = TC_WGMODE_NORMAL_gc;
  TCC0.CTRLA = TC_CLKSEL_DIV256_gc; 
  TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
  TCC0.PER = 1249; // t = N * (PER+1) / f 
  PMIC.CTRL |= PMIC_LOLVLEN_bm;
  sei();

  console_cursor(1);
  puts("  (\\/)");
  puts(" ( ..)");
  puts("C(\")(\")");
  puts("");
  puts("~BASIC-1~");
  puts("(c) 2015-2016 JVdB");
  puts("");

  for(uint16_t i=0; i<3; i++){
     sound_play(1000, 50);
     sound_play(0, 50);
  }
  sound_play(1000, 100);

  basic_register_io(uart_putc, uart_getc);
  basic_init(2048, 512);
  
  register_function_2(basic_function_type_keyword, "SOUND", do_sound, kind_numeric, kind_numeric);
  register_function_1(basic_function_type_keyword, "LED", do_led, kind_numeric);
  register_function_0(basic_function_type_numeric, "JOYSTICK", do_joystick);
  register_function_0(basic_function_type_numeric, "BUTTON", do_button);
  register_function_3(basic_function_type_keyword, "PLOT", do_plot, kind_numeric, kind_numeric, kind_numeric);
  register_function_2(basic_function_type_keyword, "DEFCHAR", do_defchar, kind_numeric, kind_string);
  register_function_1(basic_function_type_keyword, "CURSOR", do_cursor, kind_numeric);

  while(1)
  {
    basic_io_readline("", input, sizeof(input)); 
    basic_eval(input);
    if (evaluate_last_error()) {
      printf("ERR LINE %d: %s\n", __line, evaluate_last_error());
      clear_last_error();
    }
  }
  
  return EXIT_SUCCESS;
}
