#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#define COMMODORE '.'
#define CONTROL   '.' 
#define KEY_0     '0'
#define KEY_1     '1'
#define KEY_2     '2'
#define KEY_3     '3'
#define KEY_4     '4'
#define KEY_5     '5'
#define KEY_6     '6'
#define KEY_7     '7'
#define KEY_8     '8'
#define KEY_9     '9'
#define KEY_A     'A'
#define KEY_B     'B'
#define KEY_BACKSLASH '\\'
#define KEY_BACKTICK  '`'
#define KEY_C         'C'
#define KEY_COLON     ':'
#define KEY_COMMA     ','
#define KEY_D         'D'
#define KEY_DELETE    '.'
#define KEY_DOWNARROW '.'
#define KEY_E         'E'
#define KEY_ENTER     '\n'
#define KEY_EQUAL     '='
#define KEY_ESCAPE    '.'
#define KEY_F         'F'
#define KEY_F1        '.'
#define KEY_F3        '.'
#define KEY_F5        '.'
#define KEY_F7        '.'
#define KEY_G         'G'
#define KEY_H         'H'
#define KEY_HOME      '.'
#define KEY_I         'I'
#define KEY_J         'J'
#define KEY_K         'K'
#define KEY_L         'L'
#define KEY_M         'M'
#define KEY_MINUS     '-'
#define KEY_N         'N'
#define KEY_O         'O'
#define KEY_P         'P'
#define KEY_PAGEDOWN  '.'
#define KEY_PERIOD    '.'
#define KEY_PLUS      '+'
#define KEY_Q         'Q'
#define KEY_R         'R'
#define KEY_RIGHTARROW '.'
#define KEY_S         'S'
#define KEY_SEMICOLON ';'
#define KEY_SLASH     '/'
#define KEY_SPACE     ' '
#define KEY_STAR      '*' 
#define KEY_T         'T'
#define KEY_AT        '@'
#define KEY_U         'U'
#define KEY_V         'V'
#define KEY_W         'W'
#define KEY_X         'X'
#define KEY_Y         'Y'
#define KEY_Z         'Z'
#define LEFT_SHIFT    '.'
#define RIGHT_SHIFT   '.'
#define KEY_POUND     '#'
#define KEY_UP        '|'
#define KEY_LEFT      '<'
#define KEY_STOP      '.'

char keys[8][8] = {
  {KEY_DELETE, KEY_ENTER, KEY_RIGHTARROW, KEY_F7, KEY_F1, KEY_F3, KEY_F5, KEY_DOWNARROW},
  {KEY_3, KEY_W, KEY_A, KEY_4, KEY_Z, KEY_S, KEY_E, LEFT_SHIFT},
  {KEY_5, KEY_R, KEY_D, KEY_6, KEY_C, KEY_F, KEY_T, KEY_X},
  {KEY_7, KEY_Y, KEY_G, KEY_8, KEY_B, KEY_H, KEY_U, KEY_V},
  {KEY_9, KEY_I, KEY_J, KEY_0, KEY_M, KEY_K, KEY_O, KEY_N},
  {KEY_PLUS, KEY_P, KEY_L, KEY_MINUS, KEY_PERIOD, KEY_COLON, KEY_AT, KEY_COMMA},
  {KEY_POUND, KEY_STAR, KEY_SEMICOLON, KEY_HOME, RIGHT_SHIFT, KEY_EQUAL, KEY_UP, KEY_SLASH},
  {KEY_1, KEY_LEFT, CONTROL, KEY_2, KEY_SPACE, COMMODORE, KEY_Q, KEY_STOP}
};

char shifted[8][8] = {
  {KEY_DELETE, KEY_ENTER, KEY_RIGHTARROW, KEY_F7, KEY_F1, KEY_F3, KEY_F5, KEY_DOWNARROW},
  {'#', KEY_W, KEY_A, '$', KEY_Z, KEY_S, KEY_E, LEFT_SHIFT},
  {'%', KEY_R, KEY_D, '&', KEY_C, KEY_F, KEY_T, KEY_X},
  {'\'', KEY_Y, KEY_G, '(', KEY_B, KEY_H, KEY_U, KEY_V},
  {')', KEY_I, KEY_J, KEY_0, KEY_M, KEY_K, KEY_O, KEY_N},
  {KEY_PLUS, KEY_P, KEY_L, KEY_MINUS, '>', '[', KEY_AT, '<'},
  {KEY_POUND, KEY_STAR, ']', KEY_HOME, RIGHT_SHIFT, KEY_EQUAL, KEY_UP, '?'},
  {'!', KEY_LEFT, CONTROL, '"', KEY_SPACE, COMMODORE, KEY_Q, KEY_STOP}
};

  void
c64kb_init(void)
{
  PORTD.DIR = 0x00; // INPUT
  PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc;
  PORTD.PIN1CTRL = PORT_OPC_PULLUP_gc;
  PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc;
  PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc;
  PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc;
  PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc;
  // PD6, PD7 are connected to USB D+-, so we use PB0, PB1
  PORTB.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.PIN0CTRL = PORT_OPC_PULLUP_gc;
  PORTB.PIN1CTRL = PORT_OPC_PULLUP_gc;

  PORTA.DIR = 0xFF; // OUTPUT
}

  static uint8_t
get_columns(void)
{
  uint8_t columns = ~ ( (0b00111111 & PORTD.IN) | ( (0b00000011 & PORTB.IN) << 6 ) );
  return columns;
}

  static uint8_t
get_rows(void)
{
  uint8_t rows = 0;

  for(size_t row=0; row<8; row++)
  {
    PORTA.OUT = ~ _BV(row);
    _delay_ms(1);
    if (get_columns())
    {
      rows |= _BV(row);
    }
  } 

  return rows;
}

  static uint8_t
_to_index(uint8_t value)
{
  uint8_t index = 0;
  while(value)
  {
    if (value & 1) break;
    value >>= 1;
    index++;
  }
  return index;
}  

  static bool
_key_pressed()
{
  PORTA.OUT = 0b00000000;
  _delay_ms(1);
  return get_columns();
}  

  static bool
_read(uint8_t* column, uint8_t* row)
{
  *row = 0;
  *column = 0;
  if (_key_pressed())
  {
    uint8_t c1 = get_columns();
    _delay_ms(10);
    uint8_t c2 = get_columns();
    // printf("c1:%02x, c2: %02x\n", c1, c2);
    if (c1 == c2)
    {
      uint8_t r = get_rows();
      // printf("r: %02x, c: %02x\n", r, c1);
      // printf("(%d, %d)\n", _to_index(r), _to_index(c1));
      *row = r;
      *column = c1;
      _delay_ms(240);
      return true;
    }
  }
  return false;
}  

  static bool
_is_shift(uint8_t* row, uint8_t* column)
{
  if ( *row & _BV(1) && *column & _BV(7) )
  {
    // detect when to delete the bits
    if ( *row == _BV(1) && *column == _BV(7) )
    {
      // exact shift
      *row = 0;
      *column = 0;
    }
    else
    {
      // delete  when values differ
      if (*row != _BV(1))
      {
        *row &= ~ _BV(1);
      }
      if (*column != _BV(7))
      {
        *column &= ~ _BV(7);
      }
    }

    return true; 
  } 
  return false;
}  

  char
c64kb_read(void)
{
  uint8_t row, column;
  while(1)
  {
    if ( _read(&row, &column) )
    {
      char ch = '_';
      if (_is_shift(&row, &column))
      {
        if ( row && column )
        {
          ch = shifted[_to_index(row)][_to_index(column)];    
          // printf("r: %02x, c: %02x, ch: %c\n", _to_index(row), _to_index(column), ch);
          return ch;
        }
      }
      else
      {
        ch = keys[_to_index(row)][_to_index(column)];    
        // printf("r: %02x, c: %02x, ch: %c\n", _to_index(row), _to_index(column), ch);
        return ch;
      }
      // printf("r: %02x, c:%02x, %c\n", row, column, ch);
    }
  }
}

