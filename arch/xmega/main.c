#ifndef F_CPU
#   define F_CPU 2000000UL
#endif

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

#include "diskio.h"

char c64kb_read(void);
void c64kb_init(void);

static int led_blink_counter = 0;


// 100 Hz
ISR(TCC0_OVF_vect)
{
  if (led_blink_counter == 0){
    PORTE.OUTTGL = PIN0_bm;
    led_blink_counter = 50;
  }
  led_blink_counter--;
  disk_timerproc();
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

// #define C64KB

#ifdef C64KB
  return c64kb_read();;
#else
  while ( ! (USARTE0.STATUS & USART_RXCIF_bm) ) {};
  char ch = (char) USARTE0.DATA;
  return ch;
#endif  
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
  init_xtal();

  // init_uart_bscale_bsel(&USARTC1, -7, 1539); // 9K6
  // init_uart_bscale_bsel(&USARTC1, -7, 705); // 19K2
  init_uart_bscale_bsel(&USARTE0, -7, 2049); // 115200
  stdout = stdin = &uart_stdio;

  _delay_ms(100);

  _uart_putc(0x1b);
  _uart_putc('E');

  sound_init();

  for(size_t i=0; i<4; i++)
  {
    sound_play(440, 0.5);
    _delay_ms(125);
  }

}

  static int
do_sound(basic_type* freq, basic_type* duration, basic_type* rv)
{
  sound_play(freq->value.number, duration->value.number);
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

int main(int argc, char *argv[])
{
  char memory[2048];
  char input[128];

  init_xmega();

  PORTE.DIRSET = PIN0_bm;
  TCC0.CTRLB = TC_WGMODE_NORMAL_gc;
  TCC0.CTRLA = TC_CLKSEL_DIV256_gc; 
  TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
  TCC0.PER = 1249; // t = N * (PER+1) / f 
  PMIC.CTRL |= PMIC_LOLVLEN_bm;
  sei();

  c64kb_init();

  puts("  (\\/)");
  puts(" ( ..)");
  puts("C(\")(\") ~BASIC-1~ Johan Van den Brande");

  basic_register_io(uart_putc, uart_getc);
  basic_init(memory, sizeof(memory), 512);
  
  register_function_2(basic_function_type_keyword, "SOUND", do_sound, kind_numeric, kind_numeric);

  while(1)
  {
    basic_io_readline("", input, sizeof(input)); 
    basic_eval(input);
    if (evaluate_last_error()) {
      printf("ERR: %s\n", evaluate_last_error());
      puts(tokenizer_char_pointer(NULL));
    }
  }
  
  return EXIT_SUCCESS;
}
