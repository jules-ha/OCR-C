# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -ldl -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image`
LDFLAGS = -fsanitize=address
LDLIBS = `pkg-config --libs sdl2 SDL2_image` -lm

all: grid_detection

SRC = grid_detection.c  ../pretraitement/filtre.c ../pretraitement/rotation.c ../NeuralNets/tools.c ../NeuralNets/ocrNN.c ../solver/solver.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

grid_detection: grid_detection.o ../pretraitement/filtre.o ../pretraitement/rotation.o ../NeuralNets/tools.o ../NeuralNets/ocrNN.o ../solver/solver.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END
