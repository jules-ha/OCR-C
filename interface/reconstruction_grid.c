#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include "../NeuralNets/tools.h"

// Initial width and height of the surface.
const int INIT_WIDTH = 504;
const int INIT_HEIGHT = 504;
const int DIGIT_LEN = 56;

SDL_Surface* drawGrid(int** grid)
{
    //create a white surface of the right size
    SDL_Surface* res = SDL_CreateRGBSurface(0, DIGIT_LEN * 9, DIGIT_LEN * 9, 24, 0, 0, 0, 0);
    SDL_FillRect(res, NULL, SDL_MapRGB(res->format, 255, 255, 255));
    // draw each digit at the right place
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            int n = grid[i][j];
            char path[100];
            sprintf(path, "./nb_images/%d.png", n);
            SDL_Surface* digit = load_image(path);
            SDL_Rect pos;
            pos.x = DIGIT_LEN * j;
            pos.y = DIGIT_LEN * i;
            SDL_BlitSurface(digit, NULL, res, &pos);
            SDL_FreeSurface(digit);
        }
    }
    // draw the lines of the sudoku grid
    SDL_Rect line;
    line.w = 3;
    line.h = DIGIT_LEN * 9;
    SDL_Rect thickLine;
    thickLine.w = 6;
    thickLine.h = DIGIT_LEN * 9;
    for (int i = 1; i < 9; i++)
    {
        line.x = DIGIT_LEN * i;
        line.y = 0;
        if (i % 3 == 0)
        {
            thickLine.x = line.x;
            thickLine.y = line.y;
            SDL_FillRect(res, &thickLine, SDL_MapRGB(res->format, 0, 0, 0));
        }
        else
        {
            SDL_FillRect(res, &line, SDL_MapRGB(res->format, 0, 0, 0));
        }
    }
    line.w = DIGIT_LEN * 9;
    line.h = 3;
    thickLine.w = DIGIT_LEN * 9;
    thickLine.h = 6;
    for (int i = 1; i < 9; i++)
    {
        line.x = 0;
        line.y = DIGIT_LEN * i;
        if (i % 3 == 0)
        {
            thickLine.x = line.x;
            thickLine.y = line.y;
            SDL_FillRect(res, &thickLine, SDL_MapRGB(res->format, 0, 0, 0));
        }
        else
        {
            SDL_FillRect(res, &line, SDL_MapRGB(res->format, 0, 0, 0));
        }
    }
    return res;
}

/*
int main()
{
    SDL_Surface* emptyGrid = load_image("./sud_grid.jpeg");
    int** grid = malloc(9 * sizeof(int*));
    for (int i = 0; i < 9; i++)
    {
        grid[i] = malloc(9 * sizeof(int));
        for (int j = 0; j < 9; j++)
        {
            grid[i][j] = 1;
        }
    }
    SDL_Surface* gridWithDigits = drawGrid(grid);
    SDL_SaveBMP(gridWithDigits, "test.bmp");
    SDL_FreeSurface(emptyGrid);
    SDL_FreeSurface(gridWithDigits);
    for (int i = 0; i < 9; i++)
    {
        free(grid[i]);
    }
    free(grid);
    return 0;
}
*/