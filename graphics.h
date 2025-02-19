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

void redraw_prompt(WINDOW *win, int height, int prompt_x, const std::string &uname);

void handle_resize(WINDOW *&win, int &height, int &width);

void redraw_home_screen(WINDOW *home_win, int height, int width, int header_height, const std::string &username);
