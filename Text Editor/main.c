#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
	int cols;
	int row;
} CursorPosition;

static CursorPosition cursorPos;
static struct termios raw, orig_termios;

void dieProgram(const char *c) {
	char clearScreen[] = "\033[2J";
	char moveCursor[] = "\033[H";
	write(STDOUT_FILENO, clearScreen, sizeof(clearScreen) - 1);
	write(STDOUT_FILENO, moveCursor, sizeof(moveCursor) - 1);
	perror(c);
	exit(1);
}

void editorRefreshScreen(int row, int col) {
	char buf[32];
	int len = snprintf(buf, sizeof(buf), "\033[2J\033[%d;%dH", row, col);
	write(STDOUT_FILENO, buf, len);
}

void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
		write(STDOUT_FILENO, "\033[2J\033[H", 7); // Clear and reset
		dieProgram("tcsetattr");
		
	}
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) dieProgram("tcgetattr");

	raw = orig_termios;
	raw.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) dieProgram("tcsetattr");
}

void cursorMovement(int input) {
	if (input == 27) {
	char seq[3];
	if (read(STDIN_FILENO, &seq[0], 1) != 1 || read(STDIN_FILENO, &seq[1], 1) != 1) return;

	if (seq[0] == '[') {
		switch (seq[1]) {
			case 'A': // Up
				if (cursorPos.row > 0) cursorPos.row--;
				break;
			case 'B': // Down
				cursorPos.row++;
				break;
			case 'C': // Right
				cursorPos.cols++;
				break;
			case 'D': // Left
				if (cursorPos.cols > 0) cursorPos.cols--;
				break;
			}
		fprintf(stderr, "%s: %d\n",
		(seq[1] == 'A' || seq[1] == 'B') ? "Rows" : "Cols",
		(seq[1] == 'A' || seq[1] == 'B') ? cursorPos.row : cursorPos.cols);
	}
	}else {
	fprintf(stderr, "Key pressed: %c\n", input);
				}
}

int main() {
	cursorPos = (CursorPosition){0, 0};
	enableRawMode();
	atexit(disableRawMode);

	while (1) {
	char c;
	if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) dieProgram("read");
	cursorMovement(c);
	editorRefreshScreen(cursorPos.row, cursorPos.cols);
	if (c == 3) break;
	}

	return 0;
}
