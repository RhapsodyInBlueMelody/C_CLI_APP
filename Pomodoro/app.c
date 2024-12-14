#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>

#define MINUTE_TO_SECOND 60
#define SIZE_OF_COMMAND 265
#define SECOND_INITIAL 0


// Function prototypes
int pomodoro(int minute);
int timeBreak(int breakTime);
void playSound(const char* soundFile);
void notify(const char* title, const char* message);

/*Not really a clear screen but just keep the output in left up corner. YKWIS */
void clearScreen() {
    printf("\e[1;1H\e[2J");
}

void playSound(const char* soundFile){
    char command[SIZE_OF_COMMAND];
    snprintf(command, sizeof(command), "aplay %s >/dev/null 2", soundFile);
    system(command);
}

void notify(const char* title, const char* message) {
    char command[SIZE_OF_COMMAND];
    snprintf(command, sizeof(command), "notify-send '%s' '%s' &", title, message);
    system(command);
}


/*Pomodoro function, for the working invervals */
int pomodoro(int minute) {
    int second = SECOND_INITIAL;
    notify("Pomodoro", "Study time begins! Focus up.");
    playSound("start.wav");
    while(second != -1) {
        clearScreen();
        printf("Study time begins! Focus up ᕙ(⇀‸↼‶)ᕗ : %02d:%02d \n", minute, second); // Added padding
        sleep(1);
        if (second <= 0 && minute != 0) {
            minute--;
            second = MINUTE_TO_SECOND;
        } else if (minute == 0 && second == 0) {
            return 0;
        }
        second--;
    }

    return 1;
}


/*Time Break Functions for the short breaks */
int timeBreak(int breakTime) {
    int second = SECOND_INITIAL;
    notify("Pomodoro", "Break time! Relax a little.");
    playSound("break.wav");
    while (second != -1) {
        clearScreen();
        printf("Break Time (◔‿◔) : %02d:%02d\n", breakTime, second); // Added padding
        sleep(1);
        if (second <= 0 && breakTime != 0) {
            breakTime--;
            second = MINUTE_TO_SECOND;
        } else if (breakTime == 0 && second == 0) {
            return 0;
        }
        second--;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 3) { // Changed to 3 since we only need 2 arguments
        fputs("\
            Sorry, Lack of inputs\n\
            Pomodoro, Tools to manage your time while studying \n\
            Usage:  \n\
                pomodoro <Work Invervals minutes> <Short Breaks minutes>\n\
            \n\
            Example:  \n\
                pomodoro 25 5 \n\
                \n\
             It didn't manage the length of your study, if you wanted to exit   \n\
             press (Ctrl + C) \
            \n", stdout);
        return 1;
    } else if (argc > 3) {
        fputs("\
            Too Much ARGUMENTS!! ಠ_ಠ \n\
            Pomodoro, Tools to manage your time while studying \n\
            Usage:  \n\
                pomodoro <Work Invervals minutes> <Short Breaks minutes>\n\
            \n\
            Example:  \n\
                pomodoro 25 5 \n\
                \n\
             It didn't manage the length of your study, if you wanted to exit   \n\
             press (Ctrl + C) \
            \n", stdout);
        return 1;
    }

    // Check if inputs are integer.. you know.. sometimes humans just wanted to do someting out of bound.
    char *endptr;
    long minute = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || minute <= 0) {
        printf("Error: Work interval must be a positive integer\n");
        return 1;
    }

    long breakTime = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || breakTime <= 0) {
        printf("Error: Break time must be a positive integer\n");
        return 1;
    }
        /*it will keep looping until... the programs dies, which is up to you.*/
        while(1) {
            if (pomodoro(minute) == 0) {
                if (timeBreak(breakTime) == 0) {
                    continue;
                }
            }
        }
    return 0;
}
