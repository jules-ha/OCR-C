CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 -std=c99 `pkg-config --cflags sdl2 SDL2_image`
LDFLAGS = -fsanitize=address
LDLIBS = `pkg-config --libs sdl2 SDL2_image` -lm

all: xorNN ocrNN

SRC = xorNN.c ocrNN.c tools.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

xorNN: xorNN.o tools.o
ocrNN: ocrNN.o tools.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END