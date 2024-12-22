/*** includes ***/
#include <asm-generic/errno-base.h>
#include <asm-generic/ioctls.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>


/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)


/*** data ***/
struct editorConfig{
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/

void die(const char *s){
	write(STDOUT_FILENO, "\x1b[2j", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
}


void disableRawMode(){
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios)== 1)
		die("tscetattr");
}


void enableRawMode(){
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = E.orig_termios;
	raw.c_iflag &= ~(ICRNL | IXON | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag &= ~(CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey(){
	int charread;
	char c;
	while ((charread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (charread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

int getCursorPos(int *rows, int *cols){
	char buff[32];
	unsigned int i = 0;
	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
	
	while (i < sizeof(buff) - 1) {
	
	if (read(STDIN_FILENO, &buff[i], 1) != 1)  break;
	if (buff[i] == 'R')break;
	i++;
	}

	buff[i] = '\0';
	
	if (buff[0] != '\x1b' || buff[1] != '[')return -1;
	if (sscanf(&buff[2], "%d;%d", rows, cols) != 2)return -1;
	
	return 0;

}

int windowSize(int *rows, int *cols){
	struct winsize ws;

	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
		return getCursorPos(rows, cols);
	}else{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

/*** Append Buffer ***/

struct appenbuff {
	char *b;
	int len;
};

#define APPENBUFF_INIT {NULL, 0}

void buffAppend(struct appenbuff *ab, const char *c, int len){
	char *new = realloc(ab->b, ab->len + len);

	if (new == NULL) return;
	memcpy(&new[ab->len], c, len);
	ab->b = new;
	ab->len += len;
	
}

void buffFree(struct appenbuff *ab){
	free(ab->b);
}

/*** output ***/

void editorDrawRows(){
	int y;
	for (y = 0; y < E.screenrows; y++) {
	write(STDOUT_FILENO, "~", 1);
	if (y < E.screenrows - 1) {
	write(STDIN_FILENO, "~\r\n", 2);
		}
	}
}

void editorRefreshScreen(){
	write(STDOUT_FILENO, "\x1b[2j", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	write(STDIN_FILENO, "\x1b[H", 3);
}


/*** input ***/
void editorProcessKeypress(){
	char c = editorReadKey();

	switch (c) {
	case CTRL_KEY('q'):
		write(STDOUT_FILENO, "\x1b[2j", 4);
		write(STDOUT_FILENO, "\x1b[H", 3);
		exit(0);
		break;
	}
}



/*** init ***/

void initEditor(){
	if (windowSize(&E.screenrows, &E.screencols) == -1) die("windowSize");
}


int main(){
	enableRawMode();
	initEditor();

	while (1) {
	editorRefreshScreen();
	editorProcessKeypress();	
	}
	return 0;
}
