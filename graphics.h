#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

void clear_screen();

void get_terminal_size(int &rows, int &cols);

std::string apply_grad(const char &ch, int row, int col, int maxDimension);

void print_ascii_grad(const std::string &asciiArt, int rows, int cols);

void print_centered(const std::string &text, int width);

int display_help_screen(int rows, int cols);

void print_header();
