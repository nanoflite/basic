#include <stdbool.h>
#include <stdio.h>
//#include <sys/select.h>
//#include <sys/ioctl.h>
//#include <termios.h>

int kbhit(void) {
	return getchar();
}
