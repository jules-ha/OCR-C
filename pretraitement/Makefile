#Makefile

CC = gcc -lm -std=gnu99 -fsanitize=address -g
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image`
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image`

all: pretraitement

pretraitement: filtre.o rotation.o pretraitement.o
	$(CC) $(CFLAGS) $(LDLIBS) filtre.o rotation.o pretraitement.o -o pretraitement

pretraitement.o : pretraitement.c
	$(CC) -c $(CFLAGSNE) pretraitement.c -o pretraitement.o $(LDLIBS)

filtre.o: filtre.c
	$(CC) -c $(CFLAGS) filtre.c -o filtre.o

rotation.o: rotation.c
	$(CC) -c $(CFLAGS) rotation.c -o rotation.o

SRC = pretraitement.c rotation.c filtre.c main.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END
