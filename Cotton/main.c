#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ESC_KEY 27
#define ENTER_KEY 10
#define BACKSPACE 127

typedef struct {
	int cols;
	int row;
} CursorPosition;

static CursorPosition cursorPos;
static struct termios raw, orig_termios;

void clearAndExit(const char *msg) {
	write(STDOUT_FILENO, "\033[2J\033[H", 7);
	perror(msg);
	exit(1);
}

void displayBuffer(char **buffer, int rows, int cols) {
	write(STDOUT_FILENO, "\033[H", 3);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols && buffer[i][j]; j++) {
			write(STDOUT_FILENO, &buffer[i][j], 1);
		}
		write(STDOUT_FILENO, "\r\n", 2);
	}
}

void refreshScreen(char **buffer, int rows, int cols) {
	write(STDOUT_FILENO, "\033[2J", 4);
	displayBuffer(buffer, rows, cols);
	char posStr[32];
	int len = snprintf(posStr, sizeof(posStr), "\033[%d;%dH", cursorPos.row + 1, cursorPos.cols + 1);
	write(STDOUT_FILENO, posStr, len);
}

void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
		clearAndExit("tcsetattr");
	}
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) clearAndExit("tcgetattr");
	raw = orig_termios;
	raw.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) clearAndExit("tcsetattr");
}

void extendBuffer(char ***buffer, int *rows, int *cols) {
	*buffer = realloc(*buffer, (*rows + 1) * sizeof(char *));
	if (!*buffer) clearAndExit("realloc");
	(*buffer)[*rows] = calloc(*cols + 1, sizeof(char));
	if (!(*buffer)[*rows]) clearAndExit("calloc");
	(*rows)++;
}

void handleArrowKeys(char seq[], int rows, char **buffer, int cols) {
	if (seq[0] == '[') {
		switch (seq[1]) {
			case 'B': if (cursorPos.row < rows - 1) cursorPos.row++; break;
			case 'C': {
				int textEnd = 0;
				while (textEnd < cols && buffer[cursorPos.row][textEnd] != '\0') textEnd++;
				if (cursorPos.cols < textEnd) cursorPos.cols++;
				break;
			}
			case 'A': if (cursorPos.row > 0) cursorPos.row--; break;
			case 'D': if (cursorPos.cols > 0) cursorPos.cols--; break;
		}
	}
}

void cursorMovement(char ***buffer, int *rows, int *cols, int input) {
	if (input == ESC_KEY) {
		char seq[3];
		if (read(STDIN_FILENO, &seq[0], 1) != 1 ||
			read(STDIN_FILENO, &seq[1], 1) != 1) return;
		handleArrowKeys(seq, *rows, *buffer, *cols);
	} else if (input == ENTER_KEY) {
		cursorPos.row++;
		if (cursorPos.row >= *rows) {
			extendBuffer(buffer, rows, cols);
		}
		cursorPos.cols = 0;
	} else if (input == BACKSPACE) {
		if (cursorPos.cols > 0) {
			cursorPos.cols--;
			(*buffer)[cursorPos.row][cursorPos.cols] = '\0';
		} else if (cursorPos.row > 0) {
			cursorPos.row--;
			int lastCol = 0;
			while (lastCol < *cols && (*buffer)[cursorPos.row][lastCol] != '\0') {
				lastCol++;
			}
			cursorPos.cols = lastCol;
		}
	} else if ((input >= 32 && input <= 126) || input == ' ') {
		if (cursorPos.row >= *rows) {
			extendBuffer(buffer, rows, cols);
		}
		if (cursorPos.cols >= *cols) {
			*cols = cursorPos.cols + 10; // Increase by larger increment
			for (int i = 0; i < *rows; i++) {
				(*buffer)[i] = realloc((*buffer)[i], (*cols + 1) * sizeof(char));
				if (!(*buffer)[i]) clearAndExit("realloc");
				(*buffer)[i][*cols] = '\0';
			}
		}
		(*buffer)[cursorPos.row][cursorPos.cols] = input;
		cursorPos.cols++;
	}
}

int main() {
	cursorPos = (CursorPosition){0, 0};
	int rows = 1;
	int cols = 80; // Start with reasonable column width
	char **buffer = malloc(rows * sizeof(char*));
	buffer[0] = calloc(cols, sizeof(char));

	enableRawMode();
	atexit(disableRawMode);
	write(STDOUT_FILENO, "\033[2J\033[H", 7);

	while (1) {
		char c;
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) clearAndExit("read");
		if (c == CTRL_KEY('c')) break;

		cursorMovement(&buffer, &rows, &cols, c);
		refreshScreen(buffer, rows, cols);
	}

	for (int i = 0; i < rows; i++) free(buffer[i]);
	free(buffer);
	return 0;
}
