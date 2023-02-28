#ifndef XORNN_H
#define XORNN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

void forward_propagation(double *input, double w1[2][2], double w2[1][2], double b1[2][1], double b2[1][1]);
void train_network();
void initializeNetwork();
void saveNetwork();
void loadNetwork();

#endif