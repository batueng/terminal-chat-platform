#include "graphics.h"
#include "ncurses.h"

#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

void clear_screen() { clear(); }

void apply_grad(const char &ch, int row, int col, int maxDimension) {
  if (ch == ' ' || ch == '\n') {
    addch(ch);
    return;
  }

  const std::vector<int> colors = {COLOR_BLUE, COLOR_CYAN, COLOR_GREEN,
                                   COLOR_YELLOW};
  int gradientIndex =
      ((row + col) * colors.size()) / maxDimension % colors.size();

  attron(COLOR_PAIR(gradientIndex + 1));
  addch(ch);
  attroff(COLOR_PAIR(gradientIndex + 1));
}

void print_ascii_grad(const std::string &asciiArt, int rows, int cols) {
  std::istringstream stream(asciiArt);
  std::string line;
  int art_row = 4;

  while (std::getline(stream, line)) {
    if (art_row >= rows) {
      break;
    }

    int padding = (cols - line.size()) / 2;
    move(art_row, padding > 0 ? padding : 0);

    int art_col = 0;
    int maxDimension = line.size();

    for (const char &ch : line) {
      apply_grad(ch, art_row, art_col, maxDimension);
      art_col++;
    }
    art_row++;
  }
}

void print_centered(const std::string &text, int width) {
  int padding = (width - text.size()) / 2;
  if (padding > 0) {
    printw("%*s", padding, ""); // Add left padding
  }
  printw("%s\n", text.c_str());
}

int display_help_screen(int start_row, int cols) {
  int printed_rows = start_row;
  std::string top_border(cols, '=');

  mvprintw(printed_rows++, 0, top_border.c_str());
  print_centered("Help Screen", cols);
  printed_rows++;

  std::vector<std::pair<std::string, std::string>> commands = {
      {"create <session_name>",
       "Create a group chat with the name session_name"},
      {"join <session_name>", "Join a group chat with the name session_name"},
      {"where <user_name>",
       "Returns the session_name that the user_name is in"},
      {"leave", "Leave the session you are currently in"},
      {"exit", "Stop the TCP application"}};

  int max_cmd_length = 0;
  for (const auto &command : commands) {
    max_cmd_length = std::max(max_cmd_length, (int)command.first.size());
  }

  int description_width = cols - max_cmd_length - 6;

  for (const auto &command : commands) {
    const std::string &cmd = command.first;
    const std::string &desc = command.second;

    mvprintw(printed_rows++, 0, "%-*s : %s", max_cmd_length, cmd.c_str(),
             desc.c_str());
  }

  mvprintw(printed_rows++, 0, top_border.c_str());

  return printed_rows;
}

void print_header() {
  const std::string title_ascii = " _______           \n"
                                  "|__   __|          \n"
                                  "   | | ____ _____  \n"
                                  "   | |/ ___|  _  \\ \n"
                                  "   | | |____ (_) | \n"
                                  "   |_|\\____| |___/ \n"
                                  "           | |     \n"
                                  "           |_|     \n";

  const std::string header = "WELCOME TO";
  const std::string footer = "Terminal Chat Platform";

  int rows, cols;
  getmaxyx(stdscr, rows, cols); // Get terminal size

  clear();

  // Print the header text in bold and centered
  attron(A_BOLD | COLOR_PAIR(2)); // Bold and color pair 1 (blue, black)
  mvprintw(2, (cols - header.size()) / 2, "%s", header.c_str());
  attroff(A_BOLD | COLOR_PAIR(2));

  int ascii_start_row = 4; // Start row for ASCII art
  print_ascii_grad(title_ascii, rows, cols);

  // Print the footer text in bold and centered below the ASCII art
  attron(A_BOLD | COLOR_PAIR(2)); // Bold and color pair 2 (cyan, black)
  mvprintw(ascii_start_row + 10, (cols - footer.size()) / 2, "%s",
           footer.c_str());
  attroff(A_BOLD | COLOR_PAIR(2));
}

void dynamic_multi_line_input(WINDOW *input_win, std::string &input,
                              int max_rows) {
  int rows, cols;
  getmaxyx(input_win, rows, cols);

  input.clear();
  wclear(input_win);
  mvwprintw(input_win, 0, 2, "> ");
  wrefresh(input_win);

  int pos = 0;
  while (true) {
    int ch = wgetch(input_win);
    if (ch == '\n') {
      break; // Enter key pressed
    } else if (ch == KEY_BACKSPACE || ch == 127) {
      if (pos > 0) {
        input.erase(--pos, 1);
      }
    } else if (isprint(ch)) {
      input.insert(pos++, 1, ch);
    }

    // Update the input display
    wclear(input_win);
    mvwprintw(input_win, 0, 2, "> ");

    std::istringstream iss(input);
    std::string line;
    int line_num = 1;
    while (std::getline(iss, line)) {
      if (line_num >= max_rows) {
        break; // Prevent exceeding input area
      }
      mvwprintw(input_win, line_num++, 2, "%s", line.c_str());
    }

    box(input_win, 0, 0);
    wrefresh(input_win);
  }
}
