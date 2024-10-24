#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>

// Flag to indicate that the window was resized
volatile sig_atomic_t resized = false;

const size_t BUFFER_SIZE = 30;

// Signal handler for window resize
void handle_winch(int sig) {
    resized = true;
}

int main() {
  // Initialize the screen
  initscr();
  raw(); // No signals from control keys
  curs_set(1);
  keypad(stdscr, TRUE); // Enable keypad keys to signal

  // Set up the signal handler for window resizing
  struct sigaction sa;
  sa.sa_handler = handle_winch;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART; // Restart functions if interrupted by handler
  sigaction(SIGWINCH, &sa, NULL);

  // Initial window size
  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  // Move the cursor to bottom left corner
  move(rows - 1, 0);
  refresh();

  char buffer[BUFFER_SIZE];

  std::ofstream fout("log.txt");

  // Main loop
  while (true) {
    // Check if a resize event occurred
    if (resized) {
      resized = false;

      // Handle the resize
      endwin();
      refresh();

      // Get the new window size
      getmaxyx(stdscr, rows, cols);

      // Move the cursor to the new bottom-left corner
      move(rows - 1, 0);
      refresh();
    }

    getnstr(buffer, BUFFER_SIZE-1);
    fout << buffer << std::endl;
    move(rows - 1, 0);
    clrtoeol();
    refresh();
  }

  // End ncurses mode
  endwin();
  return 0;
}
