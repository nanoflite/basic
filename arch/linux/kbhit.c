#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "../arch.h"


int kbhit(void)
{
  static const int STDIN = 0;
  static bool initialized = false;
  int bytesWaiting;

  if (! initialized) {
    // Use termios to turn off line buffering
    struct termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
    initialized = true;
  }

  ioctl(STDIN, FIONREAD, &bytesWaiting);

  return bytesWaiting;
}
