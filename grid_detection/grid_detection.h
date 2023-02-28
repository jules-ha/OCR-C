#ifndef GRID_DETECTION_H
#define GRID_DETECTION_H

#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

typedef struct
{
    // coordinates of the left end of the line if horizontal
    // coordinates of the top end of the line if vertical
    int x1;
    int y1;

    // coordinates of the right end of the line if horizontal
    // coordinates of the bottom end of the line if vertical
    int x2;
    int y2;
    
    int length; // length of the line in pixels
    int vertical; // 1 if vertical, 0 if horizontal
} Line;
Line* getVerticalLines(SDL_Surface* finiteLines, int minLength);
Line* getHorizontalLines(SDL_Surface* finiteLines, int minLength);
void sobelEdgeDetection(SDL_Surface *image);
SDL_Surface *drawFiniteLines(SDL_Surface *image, SDL_Surface *edge_image);
int **houghTransform(SDL_Surface *edge_image);
SDL_Surface *drawLines(SDL_Surface *edge_image, int **accumulator);
SDL_Surface* imageToFiniteLines(SDL_Surface* image);
SDL_Surface* buildGrid(SDL_Surface* finite_lines, SDL_Surface* original_image);

#endif
