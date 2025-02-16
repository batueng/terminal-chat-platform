#include <cstddef>
#include <cstdlib>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <ncurses.h>

void print_centered(WINDOW *win, int y, int width, const std::string &text);

int display_help_screen(WINDOW *win);

void print_header(WINDOW* win);
