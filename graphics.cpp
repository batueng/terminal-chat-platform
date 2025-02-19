#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <ncurses.h>

#include "graphics.h"
#include "protocol.h"

void print_centered(WINDOW *win, int y, int width, const std::string &text) {
  int visible_length = text.size(); // text no longer has ANSI escape codes
  int x = (width - visible_length) / 2;
  mvwprintw(win, y, x, "%s", text.c_str());
}

int display_help_screen(WINDOW *win) {
  int rows, cols;
  getmaxyx(win, rows, cols);
  int printed_rows = 0;

  // Set border color (assume pair 6 is set to cyan)
  wattron(win, COLOR_PAIR(6));
  std::string top_border(cols, '=');
  mvwprintw(win, printed_rows, 0, "%s", top_border.c_str());
  printed_rows++;
  wattroff(win, COLOR_PAIR(6));

  // Print the help screen title centered
  print_centered(win, printed_rows, cols, "Help Screen");
  printed_rows++;

  // Blank line for spacing
  printed_rows++;

  // Define command list (command and description)
  std::vector<std::pair<std::string, std::string>> commands = {
      {"create <session_name>", "Create a group chat with the name session_name"},
      {"join <session_name>",   "Join a group chat with the name session_name"},
      {"where <user_name>",     "Returns the session_name that the user_name is in"},
      {"leave",                 "Leave the session you are currently in"},
      {"exit",                  "Stop the TCP application"}
  };

  // Determine the maximum command length for formatting
  int max_cmd_length = 0;
  for (const auto &command : commands) {
      max_cmd_length = std::max(max_cmd_length, static_cast<int>(command.first.size()));
  }
  int description_width = cols - max_cmd_length - 6; // extra space for borders and padding

  // For each command, print the command and its (wrapped) description
  for (const auto &command : commands) {
      const std::string &cmd = command.first;
      const std::string &desc = command.second;

      // Wrap the description text
      std::istringstream desc_stream(desc);
      std::string word, line;
      std::vector<std::string> wrapped_lines;
      while (desc_stream >> word) {
          if (line.size() + word.size() + (line.empty() ? 0 : 1) > static_cast<size_t>(description_width)) {
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

      // Print the first line with the command text.
      // Print left border in cyan.
      wattron(win, COLOR_PAIR(6));
      mvwprintw(win, printed_rows, 0, "| ");
      wattroff(win, COLOR_PAIR(6));

      // Print the command text
      wprintw(win, "%s", cmd.c_str());
      // Pad the command to reach max_cmd_length
      int pad = max_cmd_length - static_cast<int>(cmd.size());
      for (int i = 0; i < pad; i++) {
          wprintw(win, " ");
      }
      wprintw(win, " ");

      // Print the first line of the wrapped description
      if (!wrapped_lines.empty()) {
          wprintw(win, "%s", wrapped_lines[0].c_str());
      }
      // Pad the description to the required width
      int desc_pad = description_width - (wrapped_lines.empty() ? 0 : wrapped_lines[0].size());
      for (int i = 0; i < desc_pad; i++) {
          wprintw(win, " ");
      }
      // Print right border in cyan.
      wattron(win, COLOR_PAIR(6));
      wprintw(win, " |");
      wattroff(win, COLOR_PAIR(6));
      printed_rows++;

      // Print any additional wrapped lines of the description
      for (size_t i = 1; i < wrapped_lines.size(); i++) {
          wattron(win, COLOR_PAIR(6));
          mvwprintw(win, printed_rows, 0, "| ");
          wattroff(win, COLOR_PAIR(6));

          // Print spaces for the command column
          for (int j = 0; j < max_cmd_length + 1; j++) {
              wprintw(win, " ");
          }
          // Print the additional wrapped line
          wprintw(win, "%s", wrapped_lines[i].c_str());
          int pad2 = description_width - static_cast<int>(wrapped_lines[i].size());
          for (int j = 0; j < pad2; j++) {
              wprintw(win, " ");
          }
          wattron(win, COLOR_PAIR(6));
          wprintw(win, " |");
          wattroff(win, COLOR_PAIR(6));
          printed_rows++;
      }
  }

  // Print the bottom border
  wattron(win, COLOR_PAIR(6));
  mvwprintw(win, printed_rows, 0, "%s", top_border.c_str());
  wattroff(win, COLOR_PAIR(6));
  printed_rows++;

  // Refresh the window to show the help screen
  wrefresh(win);
  return printed_rows;
}

void print_header(WINDOW *win) {
    int win_height, win_width;
    getmaxyx(win, win_height, win_width);

    // Define header and footer text
    const char *header = "WELCOME TO";
    const char *footer = "Terminal Chat Platform";

    // Define the ASCII art logo as an array of strings
    const char *logo[] = {
        " _______           ",
        "|__   __|          ",
        "   | | ____ _____  ",
        "   | |/ ___|  _  \\ ",
        "   | | |____ (_) | ",
        "   |_|\\____| |___/ ",
        "           | |     ",
        "           |_|     "
    };
    const int logo_lines = 8;
    const int ascii_cols = 25;  // Used for gradient calculation

    // Clear the window and draw a border
    werase(win);

    int y = 1;  // Starting row (inside the border)

    // Center and print the header text
    int header_x = (win_width - (int)std::strlen(header)) / 2;
    mvwprintw(win, y, header_x, "%s", header);
    y += 2;  // Add some vertical spacing

    // Print the ASCII art logo with a gradient effect.
    // The gradient is computed based on the current row and column.
    for (int i = 0; i < logo_lines; i++) {
        const char *line = logo[i];
        int line_length = std::strlen(line);
        int x = (win_width - line_length) / 2;  // Center the line horizontally

        for (int j = 0; j < line_length; j++) {
            char ch = line[j];
            if (ch == ' ') {
                // Print spaces normally
                mvwaddch(win, y, x + j, ' ');
            } else {
                // Compute gradient index: choose one of 4 color pairs
                int num_colors = 4;
                int gradientIndex = (((i + j) * num_colors) / ascii_cols) % num_colors + static_cast<int>(color::BLUE);
                // Use color pair (gradientIndex + 1)
                wattron(win, COLOR_PAIR(gradientIndex));
                mvwaddch(win, y, x + j, ch);
                wattroff(win, COLOR_PAIR(gradientIndex));
            }
        }
        y++;  // Move to the next line for the logo
    }

    // Print the footer text centered
    y++;  // Add extra spacing after the logo
    int footer_x = (win_width - (int)std::strlen(footer)) / 2;
    mvwprintw(win, y, footer_x, "%s", footer);

    // Refresh the window to display the header
    wrefresh(win);
}

void redraw_prompt(WINDOW *win, int height, int prompt_x, const std::string &uname) {
  mvwprintw(win, height - 1, 1, "Enter your username: ");
  wclrtoeol(win);
  mvwprintw(win, height - 1, prompt_x, "%s", uname.c_str());
  wmove(win, height - 1, prompt_x + static_cast<int>(uname.size()));
  wrefresh(win);
}

void handle_resize(WINDOW *&win, int &height, int &width) {
  clear();
  refresh();
  getmaxyx(stdscr, height, width);
  if (win != nullptr) {
    delwin(win);
  }
  win = newwin(height, width, 0, 0);
  keypad(win, TRUE);
}
