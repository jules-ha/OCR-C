#ifndef TOOLS_H
#define TOOLS_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dirent.h>

double GaussRandom();
double sigmoid(double i);
double sigmoid_d(double i);


struct nn
{
    int input_neurons;
    int hidden_neurons;
    int output_neurons;
    double learning_rate;
    int epochs;
    double decay;

    double biases_h[150];
    double biases_o[10];

    double weights_ih[150][784]; //weights_ih[hidden][input]
    double weights_ho[10][150]; //weigts_ho[output][hidden]

    double hidden_outputs[150];
    double outputs[10];
};
SDL_Surface* load_image(const char* path);
int count_files(char* path);
void printMatrix(int *matrix);
void saveNet(struct nn* network, char* filename);
void loadNet(struct nn* network, char* filename);

#endif