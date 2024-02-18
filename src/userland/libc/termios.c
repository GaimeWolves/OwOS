#include <termios.h>

#include <__debug.h>

#include <sys/ioctl.h>

int tcgetattr(int filedes, struct termios *termios_p)
{
	TRACE("tcgetattr(%d, %p)\r\n", filedes, termios_p);
	return ioctl(filedes, TCGETS, termios_p);
}

int tcsetattr(int filedes, int optional_actions, const struct termios *termios_p)
{
	TRACE("tcsetattr(%d, %d, %p)\r\n", filedes, optional_actions, termios_p);
	(void)optional_actions;
	return ioctl(filedes, TCSETS, termios_p);
}

int tcflush(int fildes, int queue_selector)
{
	TRACE("tcflush(%d, %d)\r\n", fildes, queue_selector);
	(void)fildes;
	(void)queue_selector;
	puts("putenv() not implemented");
	abort();
}

speed_t cfgetospeed(const struct termios *termios_p)
{
	TRACE("cfgetospeed(%p)\r\n", termios_p);
	if (!termios_p)
		return 0;

	return termios_p->c_ospeed;
}
