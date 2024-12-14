# A simple Makefile for compiling small SDL projects

# set the compiler
CC := gcc

# set the compiler flags
CFLAGS := -Wall -Wextra -g
INCLUDES := $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lm

# add header files here
HDRS :=

# add source files here
SRCS := main.c

# generate names of object files
OBJS := $(SRCS:.c=.o)

# name of executable
EXEC := game

# default recipe
all: $(EXEC)

# recipe for building the final executable
$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(INCLUDES) $(LDFLAGS) -o $@

# recipe for building object files
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# recipe to clean the workspace
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean
