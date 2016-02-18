#ifndef F_CPU
#   define F_CPU 2000000UL
#endif

#include <avr/io.h>
#include <ctype.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "parser.h"

void set_usartctrl( USART_t *usart, uint8_t bscale, uint16_t bsel)
{
  usart->BAUDCTRLA = (bsel & USART_BSEL_gm);
  usart->BAUDCTRLB = ((bscale << USART_BSCALE0_bp) & USART_BSCALE_gm) | ((bsel >> 8) & ~USART_BSCALE_gm);
}

void init_uart_bscale_bsel(USART_t *usart, int8_t bscale, int16_t bsel)
{
  PORTC.DIRSET = PIN7_bm;
  PORTC.DIRCLR = PIN6_bm;
  usart->CTRLA = 0;
  usart->CTRLB = USART_TXEN_bm | USART_RXEN_bm;
  usart->CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  set_usartctrl(usart, bscale, bsel);
}


int uart_putc(int ch)
{

  PORTA.OUT = 1;
  while ( ! (USARTC1.STATUS & USART_DREIF_bm) ) {};
  USARTC1.DATA = (char) ch;
  PORTA.OUT = 0;
  return 1;
}

int uart_getc(void)
{
  PORTA.OUT = 1;
  while ( ! (USARTC1.STATUS & USART_RXCIF_bm) ) {};
  char ch = (char) USARTC1.DATA;
  PORTA.OUT = 0;
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
  init_uart_bscale_bsel(&USARTC1, -7, 1539); // 9K6
  PORTA.DIRSET = 0b11111111 ; // Set all pins on port A to be output.
  stdout = stdin = &uart_stdio;
}

int main(int argc, char *argv[])
{
  char memory[4096];
  char input[256];

  init_xmega();

  puts(" _               _      ");
  puts("| |__   __ _ ___(_) ___ ");
  puts("| '_ \\ / _` / __| |/ __|");
  puts("| |_) | (_| \\__ \\ | (__ ");
  puts("|_.__/ \\__,_|___/_|\\___|");
  puts("(c) 2015-2016 Johan Van den Brande");

  basic_register_io(uart_putc, uart_getc);
  basic_init(memory, sizeof(memory), 512);
   
  while(1)
  {
    basic_io_readline("", input, sizeof(input)); 
    basic_eval(input);
    if (evaluate_last_error()) {
      printf("ERR: %s\n", evaluate_last_error());
    }
  }
  
  return EXIT_SUCCESS;
}
