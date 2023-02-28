#ifndef OCRNN_H
#define OCRNN_H

#include <string.h>
#include <err.h>
#include "tools.h"

void initNetwork(struct nn* network, int epoch, int nodes);
void train(struct nn *networkn, char* datasetPath);
int readDigit(SDL_Surface* image, char* surface);

#endif