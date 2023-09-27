#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>

#include <curses.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	initscr();
	cbreak();
	noecho();
	clear();

	mvaddstr(10, 10, "Hello ncurses :)");

	mvaddstr(20, 20, "Hi");

	refresh();

	getch();
	endwin();

	return 0;
}