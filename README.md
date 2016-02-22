![BASIC](./basic.png)

A from-scratch BASIC interpreter with a focus on being easy to extend and port.

[![asciicast](https://asciinema.org/a/37018.png)](https://asciinema.org/a/37018)

# Introduction

This is a BASIC interpreter written from scratch in C. For the moment two architectures are supported, POSIX and AVR XMega via the avr-gcc toolchain.

Most of the common BASIC keywords are supported:

  * PRINT expression-list [ ; ]
  * IF relation-expression THEN statement
  * GOTO expression
  * INPUT variable-list
  * LET variable = expression
  * GOSUB expression
  * RETURN
  * FOR numeric\_variable '=' numeric\_expression TO numeric_expression [ STEP number ] 
  * CLEAR
  * LIST
  * RUN
  * END
  * DIM variable "(" expression ")"
  * ABS, AND, ATN, COS, EXP, INT, LOG, NOT, OR, RND, SGN, SIN, SQR, TAN
  * LEN, CHR$, MID$, LEFT$, RIGHT$, ASC 

# Extend

It should be easy to register a new BASIC function, as shown here with a `sleep` function for the XMEGA.

```C
register_function_1(basic_function_type_keyword, "SLEEP", do_sleep, kind_numeric);
...
int do_sleep(basic_type* delay, basic_type* rv)
{
  delay_ms(delay->value.number);
  
  rv->kind = kind_numeric;
  rv->value.number = 0;

  return 0;
}
```

Let's use that new keyword.

```REALbasic
10 INPUT "YOUR NAME?", NAME$
20 CLS
30 FOR I=1 TO LEN(NAME$)
40 PRINT MID$(NAME$,I,1); 
50 SLEEP(500)
60 NEXT
70 PRINT
```

# Port

It should be easy to port the interpreter to other architectures. As an example there is a port to an XMega 128A4U included using the [Batsocks breadmate board](http://www.batsocks.co.uk/products/BreadMate/XMega%20PDI%20AV.htm).

# Use

For the moment I have only programmed a simple REPL for the BASIC interpreter. You can use it in an interactive way, just as you would do on a 80's era computer.

However, it is easy to embed the interpreter into you own application.

```C
  char memory[4096];
  basic_init(memory, sizeof(memory), 512); // memory*, memory size, stack size
  basic_register_io(putchar, getchar);
  basic_eval("10 \"PRINT HELLO\"");
  basic_eval("RUN"); 
```

# Copyright

(c) 2015 - 2016 Johan Van den Brande
