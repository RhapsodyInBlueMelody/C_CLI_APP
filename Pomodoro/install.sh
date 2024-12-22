#!/bin/bash
gcc -o pomodoro app.c
sudo mv pomodoro /usr/local/bin/
echo "Pomodoro installed successfully. Run it with 'pomodoro <work> <break>'."
