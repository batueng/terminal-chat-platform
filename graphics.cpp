#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include "graphics.h"

void clear_screen() { std::cout << "\033[2J\033[H"; }

void get_terminal_size(int &rows, int &cols) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  rows = w.ws_row;
  cols = w.ws_col;
}

std::string apply_grad(const char &ch, int row, int col, int maxDimension) {
  if (ch == ' ' || ch == '\n')
    return std::string(1, ch);

  const std::vector<std::string> colors = {
      "\033[34m",     // blue
      "\033[36m",     // cyan
      "\033[32m",     // neon green
      "\033[38;5;49m" // bright green
  };
  int gradientIndex =
      ((row + col) * colors.size()) / maxDimension % colors.size();
  return colors[gradientIndex] + ch + "\033[0m";
}

void print_ascii_grad(const std::string &asciiArt, int rows, int cols) {
  const int ascii_rows = 8;
  const int ascii_cols = 25;

  std::string line;
  std::istringstream stream(asciiArt);
  int art_row = 0;

  while (std::getline(stream, line)) {
    int padding = (cols - line.size()) / 2;
    if (padding > 0) {
      std::cout << std::string(padding, ' ');
    }

    int art_col = 0;
    for (const char &ch : line) {
      std::cout << apply_grad(ch, art_row, art_col, ascii_cols);
      art_col++;
    }
    std::cout << std::endl;

    art_row++;
  }
}

void print_centered(const std::string &text, int width) {
  std::string stripped_text;
  bool in_escape = false;
  for (char c : text) {
    if (c == '\033') {
      in_escape = true;
    } else if (in_escape && c == 'm') {
      in_escape = false;
    } else if (!in_escape) {
      stripped_text += c;
    }
  }

  int visible_length = stripped_text.size();
  int padding = (width - visible_length) / 2;

  if (padding > 0) {
    std::cout << std::string(padding, ' ');
  }

  std::cout << text << std::endl;
}

int display_help_screen(int rows, int cols) {
  const std::string border_color = "\033[36m";
  const std::string reset_color = "\033[0m";

  int printed_rows = 0;

  std::string top_border(cols, '=');
  std::cout << border_color << top_border << reset_color << std::endl;
  printed_rows++;

  print_centered(border_color + "Help Screen" + reset_color, cols);
  std::cout << std::endl;
  printed_rows += 2;

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
    max_cmd_length =
        std::max(max_cmd_length, static_cast<int>(command.first.size()));
  }

  int description_width = cols - max_cmd_length - 6;

  for (const auto &command : commands) {
    const std::string &cmd = command.first;
    const std::string &desc = command.second;

    std::istringstream desc_stream(desc);
    std::string word;
    std::string line;
    std::vector<std::string> wrapped_lines;
    while (desc_stream >> word) {
      if (line.size() + word.size() + 1 >
          static_cast<size_t>(description_width)) {
        wrapped_lines.push_back(line);
        line.clear();
      }
      if (!line.empty()) {
        line += " ";
      }
      line += word;
    }
    if (!line.empty()) {
      wrapped_lines.push_back(line);
    }

    std::cout << border_color << "| " << reset_color << cmd
              << std::string(max_cmd_length - cmd.size(), ' ') << " "
              << wrapped_lines[0]
              << std::string(description_width - wrapped_lines[0].size(), ' ')
              << border_color << " |" << reset_color << std::endl;
    printed_rows++;

    for (size_t i = 1; i < wrapped_lines.size(); ++i) {
      std::cout << border_color << "| " << reset_color
                << std::string(max_cmd_length + 1, ' ') << wrapped_lines[i]
                << std::string(description_width - wrapped_lines[i].size(), ' ')
                << border_color << " |" << reset_color << std::endl;
      printed_rows++;
    }
  }

  std::cout << border_color << top_border << reset_color << std::endl;
  printed_rows++;

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

  const std::string header = "\033[1;37mWELCOME TO\033[0m";
  const std::string footer = "\033[1;37mTerminal Chat Platform\033[0m";

  int rows, cols;
  get_terminal_size(rows, cols);

  std::cout << std::endl << std::endl;
  print_centered(header, cols);
  std::cout << std::endl;

  print_ascii_grad(title_ascii, rows, cols);
  std::cout << std::endl;

  print_centered(footer, cols);
}
