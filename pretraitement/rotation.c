#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>

#include "filtre.h"
#include "rotation.h"
#include "../grid_detection/grid_detection.h"

#define M_PI 3.14159265

Uint8 valueG(Uint32 pixel_color,SDL_PixelFormat* format)
{
  //get value of the pixel (only work if in gray because r = g = b)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  return g;
}

Uint8 valueR(Uint32 pixel_color,SDL_PixelFormat* format)
{
  //get value of the pixel (only work if in gray because r = g = b)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  return r;
}

Uint8 valueB(Uint32 pixel_color,SDL_PixelFormat* format)
{
  //get value of the pixel (only work if in gray because r = g = b)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  return b;
}

Uint32 ChangeBlueValue(Uint32 pixel_color,Uint8 blue,SDL_PixelFormat* format)
{
  //(r=g=b because gray format)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  b=blue;
  Uint32 color = SDL_MapRGB(format, r, g, b); 
  return color;
}

Uint32 ChangeRedValue(Uint32 pixel_color,Uint8 red,SDL_PixelFormat* format)
{
  //(r=g=b because gray format)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  r=red;
  Uint32 color = SDL_MapRGB(format, r, g, b); 
  return color;
}

Uint32 ChangeGreenValue(Uint32 pixel_color,Uint8 green,SDL_PixelFormat* format)
{
  //(r=g=b because gray format)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  g=green;
  Uint32 color = SDL_MapRGB(format, r, g, b); 
  return color;
}

// Function that performs bilinear interpolation to determine the pixel
// value of a new pixel.
float bilinearly_interpolate_green(int top, int bottom, int left, int right,
			     float horizontal_position, float vertical_position,
			      Uint32* pixels,int w,SDL_PixelFormat* format) 
			      
{
  // Determine the values of the corners.
  Uint8 top_left = valueG(pixels[top*w+left],format);
  Uint8 top_right = valueG(pixels[top*w+right],format);
  Uint8 bottom_left = valueG(pixels[bottom*w+left],format);
  Uint8 bottom_right = valueG(pixels[bottom*w+right],format);

    // Figure out "how far" the output pixel being considered is
    // between *_left and *_right.
    float horizontal_progress = horizontal_position -
        (float) left;
    float vertical_progress = vertical_position -
        (float) top;

    // Combine top_left and top_right into one large, horizontal
    // block.
    float top_block = top_left + horizontal_progress
        * (top_right - top_left);

    // Combine bottom_left and bottom_right into one large, horizontal
    // block.
    float bottom_block = bottom_left +
        horizontal_progress
        * (bottom_right - bottom_left);

    // Combine the top_block and bottom_block using vertical
    // interpolation and return as the resulting pixel.
    return top_block + vertical_progress * (bottom_block - top_block);
}

// Function that performs bilinear interpolation to determine the pixel
// value of a new pixel.
float bilinearly_interpolate_red(int top, int bottom, int left, int right,
			     float horizontal_position, float vertical_position,
			      Uint32* pixels,int w,SDL_PixelFormat* format) 
			      
{
  // Determine the values of the corners.
  Uint8 top_left = valueR(pixels[top*w+left],format);
  Uint8 top_right = valueR(pixels[top*w+right],format);
  Uint8 bottom_left = valueR(pixels[bottom*w+left],format);
  Uint8 bottom_right = valueR(pixels[bottom*w+right],format);

    // Figure out "how far" the output pixel being considered is
    // between *_left and *_right.
    float horizontal_progress = horizontal_position -
        (float) left;
    float vertical_progress = vertical_position -
        (float) top;

    // Combine top_left and top_right into one large, horizontal
    // block.
    float top_block = top_left + horizontal_progress
        * (top_right - top_left);

    // Combine bottom_left and bottom_right into one large, horizontal
    // block.
    float bottom_block = bottom_left +
        horizontal_progress
        * (bottom_right - bottom_left);

    // Combine the top_block and bottom_block using vertical
    // interpolation and return as the resulting pixel.
    return top_block + vertical_progress * (bottom_block - top_block);
}

// Function that performs bilinear interpolation to determine the pixel
// value of a new pixel.
float bilinearly_interpolate_blue(int top, int bottom, int left, int right,
			     float horizontal_position, float vertical_position,
			      Uint32* pixels,int w,SDL_PixelFormat* format) 
			      
{
  // Determine the values of the corners.
  Uint8 top_left = valueB(pixels[top*w+left],format);
  Uint8 top_right = valueB(pixels[top*w+right],format);
  Uint8 bottom_left = valueB(pixels[bottom*w+left],format);
  Uint8 bottom_right = valueB(pixels[bottom*w+right],format);

    // Figure out "how far" the output pixel being considered is
    // between *_left and *_right.
    float horizontal_progress = horizontal_position -
        (float) left;
    float vertical_progress = vertical_position -
        (float) top;

    // Combine top_left and top_right into one large, horizontal
    // block.
    float top_block = top_left + horizontal_progress
        * (top_right - top_left);

    // Combine bottom_left and bottom_right into one large, horizontal
    // block.
    float bottom_block = bottom_left +
        horizontal_progress
        * (bottom_right - bottom_left);

    // Combine the top_block and bottom_block using vertical
    // interpolation and return as the resulting pixel.
    return top_block + vertical_progress * (bottom_block - top_block);
}









void rotation_Color(SDL_Surface* surface1 ,SDL_Surface* surface2, int theta)
{
  //Calcul angle in radian
  float angle = (float)(theta)*(float)(M_PI)/180.;
  //initialise value used
  SDL_PixelFormat* format = surface1->format;

  //get width and height of the picture to now the center
  
  int w = surface1->w;
  int h = surface1->h;

  //initialise list of the pixel in the picture
  
  Uint32* pixels1 = surface1->pixels;
  Uint32* pixels2 = surface2->pixels;

  //initialise center value
  
  int vertical_center = floor(w/2.0);
  int horizontal_center = floor(h/2.0);

     int i, j, k;
    for (i = 0; i < h; ++i) {
        for (j = 0; j < w; ++j) {
            //   Figure out how rotated we want the image.
            float vertical_position = (float) cos(angle) *
                (i - vertical_center) + sin(angle) * (j - horizontal_center)
                + vertical_center;
            float horizontal_position = (float) -sin(angle) *
                (i - vertical_center) + cos(angle) * (j - horizontal_center)
                + horizontal_center;

            // Figure out the four locations (and then, four pixels)
            // that we must interpolate from the original image.
            int top = floor(vertical_position);
            int bottom = top + 1;
            int left = floor(horizontal_position);
            int right = left + 1;

            // Check if any of the four locations are invalid. If they are,
            // skip interpolating this pixel. Otherwise, interpolate the
            // pixel according to the dimensions set above and set the
            // resulting pixel.
            if (top >= 0 && bottom < h && left >= 0 && right < w ) {
                for (k = 0; k < 3; k++) {
                    float interpolatedR = bilinearly_interpolate_red(top, bottom,
                        left, right, horizontal_position, vertical_position,
								pixels2,w,format);
		    float interpolatedG = bilinearly_interpolate_green(top, bottom,
                        left, right, horizontal_position, vertical_position,
								pixels2,w,format);
		    float interpolatedB = bilinearly_interpolate_blue(top, bottom,
                        left, right, horizontal_position, vertical_position,
								pixels2,w,format);
		    pixels1[i*w+j] = ChangeRedValue(pixels1[i*w+j],(int)(interpolatedR),format);
		    pixels1[i*w+j] = ChangeBlueValue(pixels1[i*w+j],(int)(interpolatedB),format);
		    pixels1[i*w+j] = ChangeGreenValue(pixels1[i*w+j],(int)(interpolatedG),format);
                }
            }
        }
    }
}

Uint32 ChangeValueToBlack(Uint32 pixel_color,SDL_PixelFormat* format)
{
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  r=0;
  g=0;
  b=0;
  Uint32 color = SDL_MapRGB(format, r, g, b); 
  return color;
}
Uint32 ChangeValueToBlue(Uint32 pixel_color,SDL_PixelFormat* format)
{
  //(r=g=b because gray format)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  r=0;
  g=0;
  b=255;
  Uint32 color = SDL_MapRGB(format, r, g, b); 
  return color;
}

// Function that performs bilinear interpolation to determine the pixel
// value of a new pixel.
float bilinearly_interpolate (int top, int bottom, int left, int right,
			     float horizontal_position, float vertical_position,
			      Uint32* pixels,int w,SDL_PixelFormat* format) 
			      
{
  // Determine the values of the corners.
  Uint8 top_left = value(pixels[top*w+left],format);
  Uint8 top_right = value(pixels[top*w+right],format);
  Uint8 bottom_left = value(pixels[bottom*w+left],format);
  Uint8 bottom_right = value(pixels[bottom*w+right],format);

    // Figure out "how far" the output pixel being considered is
    // between *_left and *_right.
    float horizontal_progress = horizontal_position -
        (float) left;
    float vertical_progress = vertical_position -
        (float) top;

    // Combine top_left and top_right into one large, horizontal
    // block.
    float top_block = top_left + horizontal_progress
        * (top_right - top_left);

    // Combine bottom_left and bottom_right into one large, horizontal
    // block.
    float bottom_block = bottom_left +
        horizontal_progress
        * (bottom_right - bottom_left);

    // Combine the top_block and bottom_block using vertical
    // interpolation and return as the resulting pixel.
    return top_block + vertical_progress * (bottom_block - top_block);
}


void rotation(SDL_Surface* surface1 ,SDL_Surface* surface2, int theta)
{
  //Calcul angle in radian
  float angle = (float)(theta)*(float)(M_PI)/180.;
  //initialise value used
  SDL_PixelFormat* format = surface1->format;

  //get width and height of the picture to now the center
  
  int w = surface1->w;
  int h = surface1->h;

  //initialise list of the pixel in the picture
  
  Uint32* pixels1 = surface1->pixels;
  Uint32* pixels2 = surface2->pixels;

  //initialise center value
  
  int vertical_center = floor(w/2.0);
  int horizontal_center = floor(h/2.0);

     int i, j, k;
    for (i = 0; i < h; ++i) {
        for (j = 0; j < w; ++j) {
            //   Figure out how rotated we want the image.
            float vertical_position = (float) cos(angle) *
                (i - vertical_center) + sin(angle) * (j - horizontal_center)
                + vertical_center;
            float horizontal_position = (float) -sin(angle) *
                (i - vertical_center) + cos(angle) * (j - horizontal_center)
                + horizontal_center;

            // Figure out the four locations (and then, four pixels)
            // that we must interpolate from the original image.
            int top = floor(vertical_position);
            int bottom = top + 1;
            int left = floor(horizontal_position);
            int right = left + 1;

            // Check if any of the four locations are invalid. If they are,
            // skip interpolating this pixel. Otherwise, interpolate the
            // pixel according to the dimensions set above and set the
            // resulting pixel.
            if (top >= 0 && bottom < h && left >= 0 && right < w ) {
                for (k = 0; k < 3; k++) {
                    float interpolated = bilinearly_interpolate(top, bottom,
                        left, right, horizontal_position, vertical_position,
								pixels2,w,format);
		    pixels1[i*w+j] = ChangeValue(pixels1[i*w+j],(int)(interpolated),format);
                }
            }
        }
    }
}

void rotation_binaire(SDL_Surface* surface1 ,SDL_Surface* surface2, int theta)
{
  //Calcul angle in radian
  float angle = (float)(theta)*(float)(M_PI)/180.;
  //initialise value used
  SDL_PixelFormat* format = surface1->format;

  //get width and height of the picture to now the center
  
  int w = surface1->w;
  int h = surface1->h;

  //initialise list of the pixel in the picture
  
  Uint32* pixels1 = surface1->pixels;
  Uint32* pixels2 = surface2->pixels;

  //initialise center value
  
  int vertical_center = floor(w/2.0);
  int horizontal_center = floor(h/2.0);

     int i, j, k;
    for (i = 0; i < h; ++i) {
        for (j = 0; j < w; ++j) {
            //   Figure out how rotated we want the image.
            float vertical_position = (float) cos(angle) *
                (i - vertical_center) + sin(angle) * (j - horizontal_center)
                + vertical_center;
            float horizontal_position = (float) -sin(angle) *
                (i - vertical_center) + cos(angle) * (j - horizontal_center)
                + horizontal_center;

            // Figure out the four locations (and then, four pixels)
            // that we must interpolate from the original image.
            int top = floor(vertical_position);
            int bottom = top + 1;
            int left = floor(horizontal_position);
            int right = left + 1;

            // Check if any of the four locations are invalid. If they are,
            // skip interpolating this pixel. Otherwise, interpolate the
            // pixel according to the dimensions set above and set the
            // resulting pixel.
            if (top >= 0 && bottom < h && left >= 0 && right < w ) {
                for (k = 0; k < 3; k++) {
                    float interpolated = bilinearly_interpolate(top, bottom,
                        left, right, horizontal_position, vertical_position,
								pixels2,w,format);
		    if (interpolated>128.)
		      pixels1[i*w+j] = ChangeValueToBlue(pixels1[i*w+j],format);
		    else
		      pixels1[i*w+j] = ChangeValueToBlack(pixels1[i*w+j],format);
		      
                }
            }
        }
    }
}

//detecte angle for rotation
int Angle_detection(SDL_Surface* surface1 ,SDL_Surface* surface2)
{

  int w = surface1->w;
  int h = surface1->h;
  SDL_PixelFormat* format = surface1->format;
  
  Uint32* pixels1 = surface1->pixels;

  for (int i = 0; i < h; ++i)
    {
    for (int j = 0; j < w; ++j)
     {
       pixels1[i*w+j]=ChangeValue(pixels1[i*w+j],0,format);
    }
  }
  
  int angle = 0;
  int numberLine = 0;
  int i = -45;
  while (i<45)
    {
      rotation_binaire(surface1 ,surface2, i);
      Line* hori = getHorizontalLines(surface1, 300);
      Line* verti = getVerticalLines(surface1, 300);
      int lengthHori = 0;
      int lengthVerti = 0;
      while (hori[lengthHori].x1 > -1)
	lengthHori ++;
      while (verti[lengthVerti].x1 > -1)
	lengthVerti++;
      int total = lengthVerti+lengthHori;
      if(total>numberLine)
	{
	  angle=i;
	  numberLine=total;
	}
      if (total==0)
	i+=8;
      else if (total < 20)
	i+=2;
      free(hori);
      free(verti);
      i++;
    }
  return angle;
}

//made all the rotation automatique, surface1 and 2 will be the same at the end
void Rotation_automatique(SDL_Surface* surface1 ,SDL_Surface* surface2,SDL_Surface* surface3)
{
  int theta = Angle_detection(surface1 ,surface2);
  int w = surface1->w;
  int h = surface1->h;
  SDL_PixelFormat* format = surface1->format;
  
  Uint32* pixels1 = surface1->pixels;

  for (int i = 0; i < h; ++i)
    {
    for (int j = 0; j < w; ++j)
     {
       pixels1[i*w+j]=ChangeValue(pixels1[i*w+j],255,format);
    }
  }
  
  rotation_Color(surface1 ,surface3,theta);
  SDL_SaveBMP(surface1, "rotationautomatiquecouleur.bmp");
}
