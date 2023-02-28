#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

#include "filtre.h"
#include "rotation.h"

const int INIT_WIDTH = 640;
const int INIT_HEIGHT = 400;

// Updates the display.
//
// renderer: Renderer to draw on.
// texture: Texture that contains the image.
void draw(SDL_Renderer* renderer, SDL_Texture* texture)
{
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

// Event loop that calls the relevant event handler.
//
// renderer: Renderer to draw on.
// colored: Texture that contains the colored image.
// grayscale: Texture that contains the grayscale image.
void event_loop(SDL_Renderer* renderer, SDL_Texture* colored, SDL_Texture* grayscale)
{
    SDL_Event event;
    SDL_Texture* t = colored;
    int i = 0;
    while (1)
    {
        SDL_WaitEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                return;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    draw(renderer, t);
                }
                break;
            case SDL_KEYDOWN:
                if (i==0)
                {
                    t=grayscale;
                    draw(renderer, t);
                    i=1;
                }
                else
                {
                    t = colored;
                    draw(renderer, t);
                    i=0;
                }
                break;
        }
    }
}

// Loads an image in a surface.
// The format of the surface is SDL_PIXELFORMAT_RGB888.
//
// path: Path of the image.
SDL_Surface* load_image(const char* path)
{
    SDL_Surface* image  = IMG_Load(path);
    //charge picture
    SDL_Surface* RGBimage = SDL_ConvertSurfaceFormat(image ,SDL_PIXELFORMAT_RGB888 ,0);
    //convert picture in picture format RGB
    SDL_FreeSurface(image);
    //destroy picture (first surface)
    return RGBimage;
}


// Converts a colored pixel into grayscale.
//
// pixel_color: Color of the pixel to convert in the RGB format.
// format: Format of the pixel used by the surface.
Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat* format)
{ 
     Uint8 r, g, b; 
     SDL_GetRGB(pixel_color, format, &r, &g, &b); 
     int average = 0.299*r + 0.587*g + 0.114*b; 
     r=average; 
     g=average;
     b=average; 
     Uint32 color = SDL_MapRGB(format, r, g, b); 
     return color; 
} 

//
//pixel: Color of the pixel to increase the gamma in the RGB format
//format: Format of the pixel used by the surface
Uint32 pixel_to_gamma(Uint32 pixel_color, SDL_PixelFormat* format)
{ 
     Uint8 r, g, b;
     float gamma = 2;
     SDL_GetRGB(pixel_color, format, &r, &g, &b);
     double red = r/255.;
     double green = g/255.;
     double blue = b/255.;
     r=255* pow(red,gamma);
     g=255* pow(green,gamma);
     b=255* pow(blue,gamma);
     Uint32 color = SDL_MapRGB(format, r, g, b); 
     return color; 
}

//this fonctions rotate the surface and at the same time, make it to gray and after to black and white
//
//surface : the surface that we modifie to get the final surface in blakc and white
//surface2 : the surface that we modifie before the rotation and that we copie in surface during the rotation
//theta : The angle in ° of the rotation unclockwise
void surface_traitement_rotation(SDL_Surface* surface,SDL_Surface* surface2,int theta)
{
    Uint32* pixels2 = surface2->pixels;
    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;
    for(int i=0;i<len;i++)
    {
      pixels2[i] = pixel_to_grayscale(pixels2[i],format);
    }
    rotation(surface,surface2,theta);
    Uint32* pixels = surface->pixels;
    for(int i=0;i<len;i++)
    {
      pixels[i] = pixel_to_gamma(pixels[i],format);
    }
    filtrageGaussien(surface);
    Binarisation(surface);
    SDL_UnlockSurface(surface);
}


//this fonctions make  the surface gray and after to black and white
//
//surface : the surface that we modifie to get the final surface in blakc and white
//surface2 : the surface that we modifie before the rotation and that we copie in surface during the rotation
//theta : The angle in ° of the rotation unclockwise
void surface_traitement(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;
    for(int i=0;i<len;i++)
    {
      pixels[i] = pixel_to_gamma(pixels[i],format); //increase gamma to have a better contrast
      pixels[i] = pixel_to_grayscale(pixels[i],format); // to gray and white 
    }
    filtrageGaussien(surface); //to delete isolated pixel to have a better binarisation
    Binarisation(surface); 
    SDL_UnlockSurface(surface);
}

int main(int argc, char** argv)
{
  /*// Checks the number of arguments.
    if (!(argc == 2 || argc==3))
        errx(EXIT_FAILURE, "Usage: image-file (angle)");

    // - Initialize the SDL.
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    // - Create a window.
    SDL_Window* window = SDL_CreateWindow("OCR", 0, 0, INIT_WIDTH, INIT_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    // - Create a renderer.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    // - Create a surface from the colored image.
    SDL_Surface* surface  = load_image(argv[1]);
    if (surface == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    // - Create a texture from the colored surface.
    SDL_Texture* textureCouleur = SDL_CreateTextureFromSurface(renderer,surface);*/

    SDL_Surface *image = load_image(argv[1]);
    if (image == NULL)
    {
        errx(1, "IMG_Load: %s", IMG_GetError());
    }
    
    // Produce an edge image
    filtrageGaussien(image);
    //GaussianFilter(image);
    sobelEdgeDetection(image);
    SDL_SaveBMP(image, "edge_image.bmp");
    
    int **accumulator = houghTransform(image);
    SDL_Surface* inf_lines = drawLines(image, accumulator);
    SDL_SaveBMP(inf_lines, "lines.bmp");

    SDL_Surface* edge = load_image(path);
    filtrageGaussien(edge);
    sobelEdgeDetection(edge);

    SDL_Surface* finite_lines = drawFiniteLines(inf_lines , edge);
    SDL_SaveBMP(finite_lines, "finite_lines.bmp");

    SDL_Surface* edge2 = load_image(path);
    filtrageGaussien(edge2);
    sobelEdgeDetection(edge2);
    Line* lines = getHorizontalLines(finite_lines, 300);
    
    draw(lines, edge2);
    Line* vLines = getVerticalLines(finite_lines, 300);
    draw(vLines, edge2);
    SDL_SaveBMP(edge2, "v_lines.bmp");

    SDL_Surface* im2 = load_image(path);
    //filtrageGaussien(im2);
    removeLines(lines, im2);
    removeLines(vLines, im2);
    SDL_SaveBMP(im2, "removedLines.bmp");
    int minLength = im2->w > im2->h ? im2->w / 2 : im2->h / 2;
    SDL_Surface* grid = extractGrid(vLines, lines, im2, minLength);
    SDL_SaveBMP(grid, "grid.bmp");
    int** sudoku = cutGrid(grid);
    printf("BEFORE SOLVING\n");
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            printf("%d", sudoku[i][j]);
            if ((j + 1) % 3 == 0)
            {
                printf(" ");
            }
        }
        if ((i + 1) % 3 == 0)
        {
            printf("\n");
        }
        printf("\n");
    }
    solve(0, 0, sudoku);
    printf("AFTER SOLVING\n");
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            printf("%d", sudoku[i][j]);
            if ((j + 1) % 3 == 0)
            {
                printf(" ");
            }
        }
        if ((i + 1) % 3 == 0)
        {
            printf("\n");
        }
        printf("\n");
    }

    // free the accumulator
    int maxRho = sqrt(image->w * image->w + image->h * image->h);
    for (int i = 0; i < 2 * maxRho; i++)
    {
        free(accumulator[i]);
    }
    free(accumulator);

    for (int i = 0; i < 9; i++)
    {
        free(sudoku[i]);
    }
    free(sudoku);

    free(lines);
    free(vLines);

    SDL_FreeSurface(im2);
    SDL_FreeSurface(grid);
    SDL_FreeSurface(edge2);
    SDL_FreeSurface(edge);
    SDL_FreeSurface(image);
    SDL_FreeSurface(inf_lines);
    SDL_FreeSurface(finite_lines);
    // SDL_Quit();
    return 0;























    /*
    // if we have 3 argument that mean we have to rotate with the 3 argument being an angle
    if(argc==3)
      {
	SDL_Surface* surface2  = load_image(argv[1]); //create 2 surface
	if (surface == NULL)
	  errx(EXIT_FAILURE, "%s", SDL_GetError());
	int theta = 0;  // get angle value (string to int)
	for (int i =0;argv[2][i]!=0;i++)
	  theta=theta*10+(argv[2][i]-'0');
	surface_traitement_rotation(surface,surface2,theta);
	SDL_FreeSurface(surface2);// free surface2 because now useless
      }
    else
      surface_traitement(surface); // just 2 argument and do a normal traitement

    // - Create a new texture from the pretraitement.
    SDL_Texture* pretraitement = SDL_CreateTextureFromSurface(renderer,surface);

    // - Resize the window according to the size of the image.
    int w, h;
    if (SDL_QueryTexture(textureCouleur, NULL, NULL, &w, &h) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    // - Free the surface.
    SDL_FreeSurface(surface);

    // - Dispatch the events.
    event_loop(renderer,textureCouleur,pretraitement);
    // - Destroy the objet
    SDL_DestroyTexture(textureCouleur);
    SDL_DestroyTexture(pretraitement);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;*/
}
