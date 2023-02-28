# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -ldl -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image` `pkg-config --cflags gtk+-3.0` 
LDFLAGS = -fsanitize=address
LDLIBS = `pkg-config --libs sdl2 SDL2_image` `pkg-config --libs gtk+-3.0` -lm

all: interface

SRC = interface.c ../pretraitement/filtre.c ../pretraitement/rotation.c ../NeuralNets/tools.c ../NeuralNets/ocrNN.c ../solver/solver.c ../grid_detection/grid_detection.c reconstruction_grid.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

interface: interface.o ../pretraitement/filtre.o ../pretraitement/rotation.o ../NeuralNets/tools.o ../NeuralNets/ocrNN.o ../solver/solver.o ../grid_detection/grid_detection.o reconstruction_grid.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END
