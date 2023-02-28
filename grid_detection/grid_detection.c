#include "grid_detection.h"
#include "../NeuralNets/ocrNN.h"
#include "../pretraitement/filtre.h"
#include "../pretraitement/rotation.h"
#include "../solver/solver.h"
#include "../interface/reconstruction_grid.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>

// The convolution do the addition point per point of the value in the kernel
// and the image to coordinate row and col wanted
float Convolution(SDL_Surface *image, double kernel[3][3], int row, int col)
{
    float res = 0;
    SDL_LockSurface(image);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int x = row + i;
            int y = col + j;
            if (x >= 0 && y >= 0 && x < image->h && y < image->w)
            {
                Uint32 *pixels = image->pixels;
                Uint8 r, g, b;
                SDL_GetRGB(pixels[x * image->w + y], image->format, &r, &g, &b);
                res += ((r + g + b) / 3 * kernel[i][j]);
            }
        }
    }
    SDL_UnlockSurface(image);
    return res;
}

// Produce an edge image with the Sobel operator
void sobelEdgeDetection(SDL_Surface *image)
{
    double kernelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    double kernelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    Uint32 *pixels = image->pixels;
    int cols = image->w;

    for (int i = 0; i < image->h; i++)
    {
        for (int j = 0; j < image->w; j++)
        {
            // Horizontal and vertical gradient
            double Gx = Convolution(image, kernelX, i, j);
            double Gy = Convolution(image, kernelY, i, j);

            // Gradient
            double G = sqrt(Gx * Gx + Gy * Gy);

            Uint32 color;
            // if the gradient of the pixel is higher than the seuil, the color is
            // white else is black
            if (G > 128)
            {
                color = SDL_MapRGB(image->format, 255, 255, 255);
            }
            else
            {
                color = SDL_MapRGB(image->format, 0, 0, 0);
            }
            SDL_LockSurface(image);
            // Put the pixel (x,y) into color thanks to row major order
            pixels[i * cols + j] = color;
            SDL_UnlockSurface(image);
        }
    }
}

// returns dynamically allocated array accumulator

int **houghTransform(SDL_Surface *edge_image)
{
    int width = edge_image->w;
    int height = edge_image->h;

    // theta is between 0 and 180
    int minTheta = 0;
    int maxTheta = 180;

    int maxRho = sqrt(width * width + height * height);

    int **accumulator = (int **)malloc((maxRho * 2) * sizeof(int *));
    for (int i = 0; i < 2 * maxRho; i++)
    {
        accumulator[i] = malloc(maxTheta * sizeof(int));
    }
    // initialize accumulator
    for (int i = 0; i < 2 * maxRho; i++)
    {
        for (int j = 0; j < maxTheta; j++)
        {
            accumulator[i][j] = 0;
        }
    }

    Uint32 *pixels = edge_image->pixels;

    // For every pixel in the image
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            // Get the color of the pixel (black or white)
            Uint8 r, g, b;
            SDL_GetRGB(pixels[y * width + x], edge_image->format, &r, &g, &b);

            // Check if the image is an edge
            if (r == 255)
            {
                // Check all the possible values of theta
                for (int theta = minTheta; theta < maxTheta; theta++)
                {
                    // Calculate rho
                    int rho = (int)(x * cos(theta - 90) + y * sin(theta - 90));

                    // Incremente the accumalator at the index (rho,theta)
                    accumulator[rho + maxRho][theta]++;
                }
            }
        }
    }
    return accumulator;
}

// function to draw the lines in the image in red

SDL_Surface *drawLines(SDL_Surface *edge_image, int **accumulator)
{
    int width = edge_image->w;
    int height = edge_image->h;

    int maxRho = sqrt(width * width + height * height);
    int maxTheta = 180;

    // Threshold
    int threshold;
    int max = 0;

    SDL_Surface *res = SDL_CreateRGBSurfaceFrom(
        edge_image->pixels, edge_image->w, edge_image->h, edge_image->format->BitsPerPixel, edge_image->pitch,
        edge_image->format->Rmask, edge_image->format->Gmask, edge_image->format->Bmask, edge_image->format->Amask);

    for (int i = 0; i < 2 * maxRho; i++)
    {
        for (int j = 0; j < maxTheta; j++)
        {
            if (accumulator[i][j] > max)
            {
                max = accumulator[i][j];
            }
        }
    }
    threshold = max * 0.4;

    Uint32 *pixels = res->pixels;
    SDL_LockSurface(res);
    // Check all the values in the accumulator
    for (int theta = 0; theta < maxTheta; theta++)
    {
        for (int rho = 0; rho < 2 * maxRho; rho++)
        {
            // Check if the value at the index (rho,theta) is higher than the seuil
            if (accumulator[rho][theta] > threshold)
            {

                // Draw a line in the image
                for (int x = 0; x < width; x++)
                {
                    int y = (int)(((rho - maxRho) - x * cos(theta - 90)) / sin(theta - 90));
                    if (y >= 0 && y < height)
                    {
                        Uint32 color = SDL_MapRGB(res->format, 255, 0, 0);
                        pixels[y * width + x] = color;
                    }
                }
                // draw horizontal lines
                for (int y = 0; y < height; y++)
                {
                    int x = (int)(((rho - maxRho) - y * sin(theta - 90)) / cos(theta - 90));
                    if (x >= 0 && x < width)
                    {
                        Uint32 color = SDL_MapRGB(res->format, 255, 0, 0);
                        pixels[y * width + x] = color;
                    }
                }
            }
        }
    }
    SDL_UnlockSurface(res);
    return res;
}

// function to cut infinite lines into finite lines from image
SDL_Surface *drawFiniteLines(SDL_Surface *image, SDL_Surface *edge_image)
{
    SDL_Surface *finite_lines = SDL_CreateRGBSurfaceFrom(
        edge_image->pixels, edge_image->w, edge_image->h, edge_image->format->BitsPerPixel, edge_image->pitch,
        edge_image->format->Rmask, edge_image->format->Gmask, edge_image->format->Bmask, edge_image->format->Amask);
    int w = finite_lines->w;
    int h = finite_lines->h;
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            Uint8 r1, g1, b1;
            SDL_GetRGB(((Uint32 *)finite_lines->pixels)[j * w + i], finite_lines->format, &r1, &g1, &b1);
            Uint8 r2, g2, b2;
            SDL_GetRGB(((Uint32 *)image->pixels)[j * w + i], image->format, &r2, &g2, &b2);
            if (r1 == 255 && r2 == 255 && b2 == 0)
            {
                Uint32 color = SDL_MapRGB(finite_lines->format, 0, 0, 255);
                ((Uint32 *)finite_lines->pixels)[j * w + i] = color;
            }
        }
    }
    return finite_lines;
}

Line *getHorizontalLines(SDL_Surface *finiteLines, int minLength)
{
    int x1 = -1;
    int x2 = -1;
    int y1 = -1;
    int y2 = -1;
    int count = 0;
    int correction = (int)finiteLines->w / 100;
    Line *lines = malloc(0);
    for (int i = 0; i < finiteLines->h; i++)
    {
        for (int j = 0; j < finiteLines->w; j++)
        {
            Uint8 r, g, b;
            SDL_GetRGB(((Uint32 *)finiteLines->pixels)[i * finiteLines->w + j], finiteLines->format, &r, &g, &b);

            int next_count = 0;
            // count the number of blue pixel in the next 10 pixels
            for (int k = 0; k < correction; k++)
            {
                if (j + k < finiteLines->w)
                {
                    Uint8 r1, g1, b1;
                    SDL_GetRGB(((Uint32 *)finiteLines->pixels)[i * finiteLines->w + j + k], finiteLines->format, &r1,
                               &g1, &b1);
                    if (r1 == 0 && g1 == 0 && b1 == 255)
                    {
                        next_count++;
                    }
                }
            }
            for (int k = 0; k < correction; k++)
            {
                if (j + k < finiteLines->w)
                {
                    Uint8 r1, g1, b1;
                    SDL_GetRGB(((Uint32 *)finiteLines->pixels)[(i + 1) * finiteLines->w + j + k], finiteLines->format,
                               &r1, &g1, &b1);
                    if (r1 == 0 && g1 == 0 && b1 == 255)
                    {
                        next_count++;
                    }
                }
            }

            // if the pixel is blue and the line is not started
            if (r == 0 && b == 255 && x1 == -1)
            {
                x1 = j;
                y1 = i;
            }

            // if the pixel is not blue and the line is started
            else if (!(r == 0 && b == 255 && g == 0) && x1 != -1 && next_count == 0)
            {
                x2 = j;
                y2 = i;
                if (x2 - x1 > minLength)
                {
                    count++;
                    lines = realloc(lines, count * sizeof(Line));
                    Line line;
                    line.x1 = x1;
                    line.y1 = y1;
                    line.x2 = x2;
                    line.y2 = y2;
                    line.length = x2 - x1;
                    line.vertical = 0;
                    lines[count - 1] = line;
                }
                x1 = -1;
                x2 = -1;
                y1 = -1;
                y2 = -1;
            }
        }
        x1 = -1;
        x2 = -1;
        y1 = -1;
        y2 = -1;
    }
    Line null = {-1, -1, -1, -1, -1, -1};
    lines = realloc(lines, (count + 1) * sizeof(Line));
    lines[count] = null;
    return lines;
}

Line *getVerticalLines(SDL_Surface *finiteLines, int minLength)
{
    int x1 = -1;
    int x2 = -1;
    int y1 = -1;
    int y2 = -1;
    int count = 0;
    int correction = (int)finiteLines->h / 100;
    Line *lines = malloc(0);
    for (int i = 0; i < finiteLines->w; i++)
    {
        for (int j = 0; j < finiteLines->h; j++)
        {
            Uint8 r, g, b;
            SDL_GetRGB(((Uint32 *)finiteLines->pixels)[j * finiteLines->w + i], finiteLines->format, &r, &g, &b);

            int next_count = 0;
            // count the number of blue pixels in the next 10 pixels
            for (int k = 0; k < correction; k++)
            {
                if (j + k < finiteLines->h)
                {
                    Uint8 r1, g1, b1;
                    SDL_GetRGB(((Uint32 *)finiteLines->pixels)[(j + k) * finiteLines->w + i], finiteLines->format, &r1,
                               &g1, &b1);
                    if (r1 == 0 && b1 == 255 && g1 == 0)
                    {
                        next_count++;
                    }
                }
            }
            for (int k = 0; k < correction; k++)
            {
                if (j + k < finiteLines->h)
                {
                    Uint8 r1, g1, b1;
                    SDL_GetRGB(((Uint32 *)finiteLines->pixels)[(j + k) * finiteLines->w + i + 1], finiteLines->format,
                               &r1, &g1, &b1);
                    if (r1 == 0 && b1 == 255 && g1 == 0)
                    {
                        next_count++;
                    }
                }
            }
            for (int k = 0; k < correction; k++)
            {
                if (j + k < finiteLines->h)
                {
                    Uint8 r1, g1, b1;
                    SDL_GetRGB(((Uint32 *)finiteLines->pixels)[(j + k) * finiteLines->w + i - 1], finiteLines->format,
                               &r1, &g1, &b1);
                    if (r1 == 0 && b1 == 255 && g1 == 0)
                    {
                        next_count++;
                    }
                }
            }

            // if the pixel is blue and the line is not started
            if (r == 0 && b == 255 && x1 == -1)
            {
                x1 = i;
                y1 = j;
            }

            // if the pixel is not blue and the line is started
            else if (!(r == 0 && b == 255 && g == 0) && x1 != -1 && next_count == 0)
            {
                x2 = i;
                y2 = j;
                if (y2 - y1 > minLength)
                {
                    count++;
                    lines = realloc(lines, count * sizeof(Line));
                    Line line;
                    line.x1 = x1;
                    line.y1 = y1;
                    line.x2 = x2;
                    line.y2 = y2;
                    line.length = y2 - y1;
                    line.vertical = 1;
                    lines[count - 1] = line;
                }
                x1 = -1;
                x2 = -1;
                y1 = -1;
                y2 = -1;
            }
        }
        x1 = -1;
        x2 = -1;
        y1 = -1;
        y2 = -1;
    }
    Line null = {-1, -1, -1, -1, -1, -1};
    lines = realloc(lines, (count + 1) * sizeof(Line));
    lines[count] = null;
    return lines;
}

void draw(Line *lines, SDL_Surface *image)
{
    int i = 0;
    SDL_LockSurface(image);
    while (lines[i].x1 != -1)
    {
        if (lines[i].vertical == 0)
        {
            for (int j = lines[i].x1; j < lines[i].x2; j++)
            {
                Uint32 color = SDL_MapRGB(image->format, 0, 255, 0);
                ((Uint32 *)image->pixels)[lines[i].y1 * image->w + j] = color;
            }
        }
        else
        {
            for (int j = lines[i].y1; j < lines[i].y2; j++)
            {
                Uint32 color = SDL_MapRGB(image->format, 0, 255, 0);
                ((Uint32 *)image->pixels)[j * image->w + lines[i].x1] = color;
            }
        }
        i++;
    }
    SDL_UnlockSurface(image);
}

void removeLines(Line *lines, SDL_Surface *image)
{
    int i = 0;
    SDL_LockSurface(image);
    while (lines[i].x1 != -1)
    {
        if (lines[i].vertical == 0)
        {
            for (int j = lines[i].x1; j < lines[i].x2; j++)
            {
                Uint32 color = SDL_MapRGB(image->format, 255, 255, 255);
                ((Uint32 *)image->pixels)[lines[i].y1 * image->w + j] = color;
            }
        }
        else
        {
            for (int j = lines[i].y1; j < lines[i].y2; j++)
            {
                Uint32 color = SDL_MapRGB(image->format, 255, 255, 255);
                ((Uint32 *)image->pixels)[j * image->w + lines[i].x1] = color;
            }
        }
        i++;
    }
    SDL_UnlockSurface(image);
}

// function to extract the grid from the image
// returns the grid as an image
SDL_Surface *extractGrid(Line *vLines, Line *hLines, SDL_Surface *image, int minLength)
{
    int i = 0;
    int x = -1;
    int y = -1;
    int length = -1;
    int correction = 10;
    while (hLines[i].x1 != -1 && length == -1)
    {
        int j = 0;
        while (vLines[j].x1 != -1 && length == -1)
        {
            if (vLines[j].x1 <= hLines[i].x1 + correction && vLines[j].x1 >= hLines[i].x1 - correction &&
                vLines[j].y1 <= hLines[i].y1 + correction && vLines[j].y1 >= hLines[i].y1 - correction &&
                vLines[j].length > minLength && hLines[i].length > minLength)
            {
                x = (vLines[j].x1 + hLines[i].x1) / 2;
                y = (vLines[j].y1 + hLines[i].y1) / 2;
                length = (vLines[j].length + hLines[i].length) / 2;
            }
            j++;
        }
        i++;
    }
    SDL_Surface *grid = SDL_CreateRGBSurface(0, length, length, 32, 0, 0, 0, 0);
    SDL_LockSurface(grid);
    SDL_LockSurface(image);
    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < length; j++)
        {
            ((Uint32 *)grid->pixels)[i * grid->w + j] = ((Uint32 *)image->pixels)[(y + i) * image->w + (x + j)];
        }
    }
    SDL_UnlockSurface(grid);
    SDL_UnlockSurface(image);
    return grid;
}

// function to resize image to a 28 x 28px image
SDL_Surface *resize_to_28px(SDL_Surface *image)
{
    SDL_Surface *resized = SDL_CreateRGBSurface(0, 28, 28, 32, 0, 0, 0, 0);
    SDL_LockSurface(resized);
    SDL_LockSurface(image);
    for (int i = 0; i < 28; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            ((Uint32 *)resized->pixels)[i * resized->w + j] =
                ((Uint32 *)image->pixels)[(i * image->h / 28) * image->w + (j * image->w / 28)];
        }
    }
    SDL_UnlockSurface(resized);
    SDL_UnlockSurface(image);
    return resized;
}

// function to remove lines of the sudoku grid from the 28 x 28px image
// allows the neural network to recognize the numbers better
void cleanSquare(SDL_Surface *image)
{
    SDL_LockSurface(image);
    Line hLines[28];
    Line vLines[28];
    int white = 0;
    int k = 0;
    int thresold = 20; // minimum number of white pixels to consider a line
    for (int i = 0; i < 28; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            Uint8 r, g, b;
            SDL_GetRGB(((Uint32 *)image->pixels)[i * image->w + j], image->format, &r, &g, &b);
            if (r == 255 && g == 255 && b == 255)
            {
                white++;
            }
        }
        if (white > thresold)
        {
            hLines[k].x1 = 0;
            hLines[k].y1 = i;
            hLines[k].x2 = 28;
            hLines[k].y2 = i;
            hLines[k].length = 28;
            hLines[k].vertical = 0;
            k++;
        }
        white = 0;
    }
    hLines[k] = (Line){-1, -1, -1, -1, -1, -1};
    white = 0;
    k = 0;
    for (int i = 0; i < 28; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            Uint8 r, g, b;
            SDL_GetRGB(((Uint32 *)image->pixels)[j * image->w + i], image->format, &r, &g, &b);
            if (r == 255 && g == 255 && b == 255)
            {
                white++;
            }
        }
        if (white > thresold)
        {
            vLines[k].x1 = i;
            vLines[k].y1 = 0;
            vLines[k].x2 = i;
            vLines[k].y2 = 28;
            vLines[k].length = 28;
            vLines[k].vertical = 1;
            k++;
        }
        white = 0;
    }
    vLines[k] = (Line){-1, -1, -1, -1, -1, -1};
    // draw all the lines in black
    k = 0;
    while (hLines[k].x1 != -1)
    {
        for (int i = hLines[k].x1; i < hLines[k].x2; i++)
        {
            Uint32 color = SDL_MapRGB(image->format, 0, 0, 0);
            ((Uint32 *)image->pixels)[hLines[k].y1 * image->w + i] = color;
        }
        k++;
    }
    k = 0;
    while (vLines[k].x1 != -1)
    {
        for (int i = vLines[k].y1; i < vLines[k].y2; i++)
        {
            Uint32 color = SDL_MapRGB(image->format, 0, 0, 0);
            ((Uint32 *)image->pixels)[i * image->w + vLines[k].x1] = color;
        }
        k++;
    }
    SDL_UnlockSurface(image);
}

// function to count the number of white pixels that are accessible from a given position
int measureObjects(SDL_Surface *image, int position, int *visited)
{
    Uint8 r, g, b;
    SDL_GetRGB(((Uint32 *)image->pixels)[position], image->format, &r, &g, &b);
    visited[position] = 1;
    int count = 0;
    if (r == 255 && g == 255 && b == 255)
    {
        count++;
        if (position + 1 < 28 * 28 && visited[position + 1] == 0)
        {
            count += measureObjects(image, position + 1, visited);
        }
        if (position - 1 >= 0 && visited[position - 1] == 0)
        {
            count += measureObjects(image, position - 1, visited);
        }
        if (position + 28 < 28 * 28 && visited[position + 28] == 0)
        {
            count += measureObjects(image, position + 28, visited);
        }
        if (position - 28 >= 0 && visited[position - 28] == 0)
        {
            count += measureObjects(image, position - 28, visited);
        }
    }
    return count;
}

// function to remove noise from the 28 x 28px image
void removeNoise(SDL_Surface *image)
{
    SDL_LockSurface(image);
    for (int i = 0; i < 28 * 28; i++)
    {
        int visited[28 * 28];
        for (int j = 0; j < 28 * 28; j++)
        {
            visited[j] = 0;
        }
        int count = measureObjects(image, i, visited);
        if (count < 21)
        {
            Uint32 color = SDL_MapRGB(image->format, 0, 0, 0);
            ((Uint32 *)image->pixels)[i] = color;
        }
    }
    SDL_UnlockSurface(image);
}

SDL_Surface *centerDigit(SDL_Surface *digit)
{
    SDL_LockSurface(digit);
    int highest = 28;
    int lowest = 0;
    int leftest = 28;
    int rightest = 0;
    for (int i = 0; i < 28; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            Uint8 r, g, b;
            SDL_GetRGB(((Uint32 *)digit->pixels)[i * digit->w + j], digit->format, &r, &g, &b);
            if (r == 255 && g == 255 && b == 255)
            {
                if (i < highest)
                {
                    highest = i;
                }
                if (i > lowest)
                {
                    lowest = i;
                }
                if (j < leftest)
                {
                    leftest = j;
                }
                if (j > rightest)
                {
                    rightest = j;
                }
            }
        }
    }
    SDL_Surface *centered = SDL_CreateRGBSurface(0, 28, 28, 32, 0, 0, 0, 0);
    SDL_LockSurface(centered);
    for (int i = 0; i < 28; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            Uint32 color = SDL_MapRGB(digit->format, 0, 0, 0);
            ((Uint32 *)centered->pixels)[i * centered->w + j] = color;
        }
    }
    int x = (28 - (rightest - leftest)) / 2;
    int y = (28 - (lowest - highest)) / 2;
    int i = highest;
    int j = leftest;
    while (i <= lowest)
    {
        while (j <= rightest)
        {
            ((Uint32 *)centered->pixels)[y * centered->w + x] = ((Uint32 *)digit->pixels)[i * digit->w + j];
            j++;
            x++;
        }
        j = leftest;
        x = (28 - (rightest - leftest)) / 2;
        i++;
        y++;
    }
    SDL_UnlockSurface(centered);
    SDL_UnlockSurface(digit);
    return centered;
}
// count number of white pixels in the image
int countWhite(SDL_Surface *image)
{
    int white = 0;
    SDL_LockSurface(image);
    for (int i = 0; i < image->w; i++)
    {
        for (int j = 0; j < image->h; j++)
        {
            Uint8 r, g, b;
            SDL_GetRGB(((Uint32 *)image->pixels)[j * image->w + i], image->format, &r, &g, &b);
            if (r == 255 && g == 255 && b == 255)
            {
                white++;
            }
        }
    }
    SDL_UnlockSurface(image);
    return white;
}

int **cutGrid(SDL_Surface *grid)
{
    int k = 0;
    int length = grid->w / 9;
    int **digits = malloc(9 * sizeof(int *));
    int predictions[9][9][2];
    int whites_p[9][9];
    int average = 0;
    for (int i = 0; i < 9; i++)
    {
        digits[i] = malloc(9 * sizeof(int));
    }
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            digits[i][j] = 0;
        }
    }
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            SDL_Surface *digit = SDL_CreateRGBSurface(0, length, length, 32, 0, 0, 0, 0);
            SDL_LockSurface(digit);
            SDL_LockSurface(grid);
            for (int i1 = 0; i1 < length; i1++)
            {
                for (int j1 = 0; j1 < length; j1++)
                {
                    Uint8 r, g, b;
                    SDL_GetRGB(((Uint32 *)grid->pixels)[(i * length + i1) * grid->w + (j * length + j1)], grid->format,
                               &r, &g, &b);
                    if ((r + g + b) / 3 < 128)
                    {
                        ((Uint32 *)digit->pixels)[i1 * digit->w + j1] = SDL_MapRGB(digit->format, 255, 255, 255);
                    }
                    else
                    {
                        ((Uint32 *)digit->pixels)[i1 * digit->w + j1] = SDL_MapRGB(digit->format, 0, 0, 0);
                    }
                }
            }
            SDL_Surface *resized = resize_to_28px(digit);
            SDL_UnlockSurface(digit);
            SDL_UnlockSurface(grid);
            int whites = countWhite(resized);
            average += whites;
            cleanSquare(resized);
            if (whites > 39)
                removeNoise(resized);
            whites = countWhite(resized);
            SDL_Surface *centered = centerDigit(resized);
            predictions[i][j][0] = readDigit(centered, "../NeuralNets/big.dat");
            predictions[i][j][1] = readDigit(centered, "../NeuralNets/small.dat");
            whites_p[i][j] = whites;
            if (validBoard(digits) != 0)
            {
                digits[k / 9][k % 9] = 0;
            }
            k++;
            SDL_FreeSurface(digit);
            SDL_FreeSurface(resized);
            SDL_FreeSurface(centered);
        }
    }
    average /= 81;
    int tmp = average > 14 ? 0 : 1;
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if ((tmp == 0 && whites_p[i][j] < 24) || (tmp == 1 && whites_p[i][j] < 7))
                digits[i][j] = 0;
            else
                digits[i][j] = predictions[i][j][tmp];
        }
    }
    return digits;
}

// image is just the image of the grid without any pretraitements
// runs through the all process of hough transform
// return the image with the finites lines
SDL_Surface* imageToFiniteLines(SDL_Surface* image)
{
    SDL_Surface *image1 = SDL_CreateRGBSurface(0, image->w, image->h, 32, 0, 0, 0, 0);
    SDL_BlitSurface(image, NULL, image1, NULL);
    filtrageGaussien(image1);
    sobelEdgeDetection(image1);
    int **accumulator = houghTransform(image1);
    SDL_Surface *inf_lines = drawLines(image1, accumulator);
    SDL_SaveBMP(inf_lines, "inf_lines.bmp");
    SDL_Surface *edge = SDL_CreateRGBSurface(0, image->w, image->h, 32, 0, 0, 0, 0);
    SDL_BlitSurface(image, NULL, edge, NULL);
    filtrageGaussien(edge);
    sobelEdgeDetection(edge);
    SDL_Surface *finite_lines = drawFiniteLines(inf_lines, edge);
    SDL_FreeSurface(image1);
    SDL_FreeSurface(inf_lines);
    SDL_FreeSurface(edge);
    int maxRho = sqrt(image->w * image->w + image->h * image->h);
    for (int i = 0; i < 2 * maxRho; i++)
    {
        free(accumulator[i]);
    }
    free(accumulator);
    SDL_SaveBMP(finite_lines, "finite_lines.bmp");
    return finite_lines;
}

SDL_Surface* buildGrid(SDL_Surface* finite_lines, SDL_Surface* original_image)
{
    Line *lines = getHorizontalLines(finite_lines, 300);
    Line *vLines = getVerticalLines(finite_lines, 300);

    //copy of original
    SDL_Surface* edge2 = SDL_CreateRGBSurface(0, original_image->w, original_image->h, 32, 0, 0, 0, 0);
    SDL_BlitSurface(original_image, NULL, edge2, NULL);

    draw(lines, edge2);
    draw(vLines, edge2);

    SDL_Surface *im2 = SDL_CreateRGBSurface(0, original_image->w, original_image->h, 32, 0, 0, 0, 0);
    SDL_BlitSurface(original_image, NULL, im2, NULL);
    removeLines(lines, im2);
    removeLines(vLines, im2);
    int minLength = im2->w > im2->h ? im2->w / 2 : im2->h / 2;
    SDL_Surface *grid = extractGrid(vLines, lines, im2, minLength);
    int **sudoku = cutGrid(grid);
    solve(0, 0, sudoku);
    SDL_Surface *resolved = drawGrid(sudoku);

    //free all the memory
    SDL_FreeSurface(im2);
    SDL_FreeSurface(grid);
    SDL_FreeSurface(edge2);
    free(lines);
    free(vLines);
    for (int i = 0; i < 9; i++)
    {
        free(sudoku[i]);
    }
    free(sudoku);
    return resolved;
}