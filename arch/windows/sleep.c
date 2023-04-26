#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "../arch.h"


void
arch_sleep(int msec)
{
  Sleep(msec);
}
