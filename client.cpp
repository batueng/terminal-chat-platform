#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

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
      "\033[34m", // blue
      "\033[36m", // cyan
      "\033[32m", // neon Green
      "\033[33m", // yellow
  };

  int gradientIndex =
      ((row + col) * colors.size()) / maxDimension % colors.size();
  return colors[gradientIndex] + ch + "\033[0m";
}

void print_ascii_grad(const std::string &asciiArt, int rows, int cols) {
  int maxDimension = std::max(rows, cols);
  int row = 0, col = 0;

  for (const char &ch : asciiArt) {
    if (ch == '\n') {
      std::cout << std::endl;
      row++;
      col = 0;
    } else {
      std::cout << apply_grad(ch, row, col, maxDimension);
      col++;
    }
  }
}

void print_centered(const std::string &text, int width) {
  int padding = (width - text.size()) / 2;
  if (padding > 0) {
    std::cout << std::string(padding, ' ');
  }
  std::cout << text << std::endl;
}

void display_help_screen(int rows, int cols) {
  clear_screen();
  print_centered("Help Screen", cols);
  std::cout << std::endl;
  print_centered("Commands:", cols);
  std::cout << std::endl;
  print_centered(
      "join <session_name> - Join a group chat with the name session_name",
      cols);
  print_centered(
      "create <session_name> - Create a group chat with the name session_name",
      cols);
  print_centered(
      "where <user_name> - Returns the session_name that the user_name is in",
      cols);
  print_centered("leave - Leave the session you are currently in", cols);
  print_centered("exit - Stop the TCP application", cols);
  std::cout << std::endl;
  print_centered("Enter your commands below:", cols);
}

void startup() {
  // this says terminal chat platform. it looks weird, please do not do anything
  // to change it. const std::string title_ascii =
  //     "  _______                                  _ \n" " |__   __| o | | \n"
  //     "    | | ___ _ __ _ __  __   _  _ __   __ _| | \n" "    | |/ _ \\ '__|
  //     '  \\/_  \\| || '_  \\/ _` | |                    \n" "    | |  __/ |
  //     | | | | | || || | | | (_| | |                        \n" " |_|\\___|_|
  //     |_| |_| |_||_||_| |_|\\__,_|_|                      \n" "   _____ _ _
  //     _____ _       _    ___                    \n" "  / ____| |         | |
  //     |  _  \\ |     | |  / __|                  \n" " | |    | |__   __ _|
  //     |_  | |_) | | __ _| |_| |__ _  _ __ _ __  __   \n" " | |    | '_ \\ /
  //     _` | __| |  ___/ |/ _` | __|  _|_  \\ '__| '_ \\/_  "
  //     "\\ \n"
  //     " | |____| | | | (_| | |_  | |   | | (_| | |_| | (_) | |  | | | | | |
  //     \n" "  \\_____|_| |_|\\__,_|\\__| |_|   |_|\\__,_|\\__|_|\\____/_|  |_|
  //     "
  //     "|_| "
  //     "|_| \n";

  // this says Tcp
  const std::string title_ascii = "  _______            \n"
                                  " |__   __|           \n"
                                  "    | | ____ _____   \n"
                                  "    | |/ ___|  _  \\  \n"
                                  "    | | |____ (_) |   \n"
                                  "    |_|\\____| |___/  \n"
                                  "            | |       \n"
                                  "            |_|       \n";

  int rows, cols;
  get_terminal_size(rows, cols);

  clear_screen();

  // print the welcome screen
  std::cout << std::endl;
  std::cout << "Welcome to" << std::endl;
  print_ascii_grad(title_ascii, rows, cols);
  // prompt for username
  std::cout << std::endl;
  print_centered("Please enter your username:", cols);
  std::cout << std::endl;

  // move the command prompt to the bottom
  std::string username;
  for (int i = 0; i < rows - 6; ++i) {
    std::cout << std::endl;
  }
  std::cout << "> ";
  std::getline(std::cin, username);

  clear_screen();
  std::cout << "Welcome, " << username << "!\n";

  display_help_screen(rows, cols);
}

void run() {
  startup();

  std::string command;
  while (true) {
    int rows, cols;
    get_terminal_size(rows, cols);

    for (int i = 0; i < rows - 8; ++i) {
      std::cout << std::endl;
    }
    std::cout << "> ";
    std::getline(std::cin, command);

    if (command == "exit") {
      std::cout << "Exiting application. Goodbye!\n";
      break;
    } else {
      std::cout << "Command received: " << command << "\n";
    }
  }
}

int main(int argc, char *argv[]) {
  run();
  return 0;
}
