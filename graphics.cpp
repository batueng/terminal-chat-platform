#include <cstddef>
#include <cstdlib>
#include <ncurses.h>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/ttycom.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include "graphics.h"
#include "protocol.h"

void print_centered(WINDOW *win, int y, int width, const std::string &text) {
  int visible_length = text.size();
  int x = (width - visible_length) / 2;
  mvwprintw(win, y, x, "%s", text.c_str());
}

int display_help_screen(WINDOW *win) {

  int rows, cols;
  getmaxyx(win, rows, cols);

  int row_cnt = 0;
  int pad_x = 1;
  int indent = 2;
  int descr_space = 4;
  std::string line = "Usage:";
  mvwprintw(win, row_cnt++, pad_x, "%s", line.c_str());

  line = "tcp";

  wattron(win, COLOR_PAIR(color::CYAN));
  mvwprintw(win, row_cnt++, indent + pad_x, "%s", line.c_str());
  wattroff(win, COLOR_PAIR(color::CYAN));

  std::vector<std::pair<std::string, std::string>> commands = {
      {"create <session_name>", "Create a group chat called <session_name>."},
      {"join <session_name>", "Join group chat called <session_name>."},
      {"where <user_name>",
       "Find the group chat that user <username> is currently in."},
      {":leave", "Leave the current session."},
      {"exit", "Exit Tcp."}};

  line = "Commands:";
  mvwprintw(win, (++row_cnt)++, pad_x, "%s", line.c_str());

  int max_cmd_length = 0;
  for (const auto &command : commands) {
    max_cmd_length =
        std::max(max_cmd_length, static_cast<int>(command.first.size()));
  }
  int description_width =
      cols - 2 * pad_x - max_cmd_length - descr_space - indent;

  for (const auto &command : commands) {
    const std::string &cmd = command.first;
    const std::string &desc = command.second;

    std::istringstream desc_stream(desc);
    std::string word, line;
    std::vector<std::string> wrapped_lines;
    while (desc_stream >> word) {
      if (line.size() + word.size() + (line.empty() ? 0 : 1) >
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

    wattron(win, COLOR_PAIR(color::CYAN));
    mvwprintw(win, row_cnt, indent + pad_x, "%s", cmd.c_str());
    wattroff(win, COLOR_PAIR(color::CYAN));

    for (size_t i = 0; i < wrapped_lines.size(); i++) {
      mvwprintw(win, row_cnt++, indent + pad_x + max_cmd_length + descr_space,
                "%s", wrapped_lines[i].c_str());
    }
  }

  return row_cnt;
}

void print_header(WINDOW *win) {
  int win_height, win_width;
  getmaxyx(win, win_height, win_width);

  // Define header and footer text
  const char *header = "WELCOME TO";
  const char *footer = "Terminal Chat Platform";

  // Define the ASCII art logo as an array of strings
  const char *logo[] = {" _______           ", "|__   __|          ",
                        "   | | ____ _____  ", "   | |/ ___|  _  \\ ",
                        "   | | |____ (_) | ", "   |_|\\____| |___/ ",
                        "           | |     ", "           |_|     "};
  const int logo_lines = 8;
  const int ascii_cols = 25; // Used for gradient calculation
  int y = 1; // Starting row (inside the border)

  // Center and print the header text
  int header_x = (win_width - (int)std::strlen(header)) / 2;
  mvwprintw(win, y, header_x, "%s", header);
  y += 2; // Add some vertical spacing

  // Print the ASCII art logo with a gradient effect.
  // The gradient is computed based on the current row and column.
  for (int i = 0; i < logo_lines; i++) {
    const char *line = logo[i];
    int line_length = std::strlen(line);
    int x = (win_width - line_length) / 2; // Center the line horizontally

    for (int j = 0; j < line_length; j++) {
      char ch = line[j];
      if (ch == ' ') {
        // Print spaces normally
        mvwaddch(win, y, x + j, ' ');
      } else {
        // Compute gradient index: choose one of 4 color pairs
        int num_colors = 4;
        int gradientIndex = (((i + j) * num_colors) / ascii_cols) % num_colors +
                            static_cast<int>(color::BLUE);
        // Use color pair (gradientIndex + 1)
        wattron(win, COLOR_PAIR(gradientIndex));
        mvwaddch(win, y, x + j, ch);
        wattroff(win, COLOR_PAIR(gradientIndex));
      }
    }
    y++; // Move to the next line for the logo
  }

  // Print the footer text centered
  y++; // Add extra spacing after the logo
  int footer_x = (win_width - (int)std::strlen(footer)) / 2;
  mvwprintw(win, y, footer_x, "%s", footer);

  // Refresh the window to display the header
  wrefresh(win);
}

void redraw_home_screen(WINDOW *home_win, int height, int width,
                        int header_height, const std::string &username) {
  werase(home_win);

  std::string user_string = "User: ";
  mvwprintw(home_win, 0, 1, user_string.c_str());

  wattron(home_win, COLOR_PAIR(color::CYAN));
  mvwprintw(home_win, 0, user_string.size()+1, username.c_str());
  wattroff(home_win, COLOR_PAIR(color::CYAN));

  print_header(home_win);

  WINDOW *help_win =
      derwin(home_win, height - header_height, width, header_height, 0);
  display_help_screen(help_win);

  wrefresh(home_win);
  delwin(help_win);
}

void handle_session_resize(WINDOW *&messages_win, WINDOW *&input_win,
                           int &height, int &width, int header_height,
                           const std::string &curr_sess,
                           const std::string &username,
                           color user_color) {
  clear();
  refresh();
  getmaxyx(stdscr, height, width);

  if (messages_win) {
    delwin(messages_win);
  }
  if (input_win) {
    delwin(input_win);
  }

  messages_win = newwin(height - 2, width, 0, 0);
  input_win = newwin(2, width, height - 2, 0);

  keypad(messages_win, TRUE);
  keypad(input_win, TRUE);

  scrollok(messages_win, TRUE);

  werase(messages_win);
  wrefresh(messages_win);

  werase(input_win);
  wrefresh(input_win);

  int msg_width = getmaxx(messages_win);

  std::string user_string = "User: ";
  mvwprintw(messages_win, 0, 1, user_string.c_str());

  wattron(messages_win, COLOR_PAIR(user_color));
  mvwprintw(messages_win, 0, user_string.size()+1, username.c_str());
  wattroff(messages_win, COLOR_PAIR(user_color));

  wattron(messages_win, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(messages_win, 1, (msg_width - curr_sess.size()) / 2, "%s",
            curr_sess.c_str());
  wattroff(messages_win, COLOR_PAIR(1) | A_BOLD);
  wrefresh(messages_win);
}

void redraw_prompt(WINDOW *win, int height, int prompt_x,
                   const std::string &uname) {
  mvwprintw(win, height - 1, 1, "Enter your username: ");
  wclrtoeol(win);
  mvwprintw(win, height - 1, prompt_x, "%s", uname.c_str());
  wmove(win, height - 1, prompt_x + static_cast<int>(uname.size()));
  wrefresh(win);
}

void redraw_session_prompt(WINDOW *input_win, int prompt_x,
                           const std::string &line) {
  mvwprintw(input_win, 1, 1, "> ");
  wclrtoeol(input_win);
  mvwprintw(input_win, 1, prompt_x, "%s", line.c_str());
  wmove(input_win, 1, prompt_x + static_cast<int>(line.size()));
  wrefresh(input_win);
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
