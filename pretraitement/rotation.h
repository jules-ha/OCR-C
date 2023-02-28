#ifndef ROTATION_H
#define ROTATION_H

#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "filtre.h"
#include "../grid_detection/grid_detection.h"

Uint8 valueG(Uint32 pixel_color,SDL_PixelFormat* format);
Uint8 valueR(Uint32 pixel_color,SDL_PixelFormat* format);
Uint8 valueB(Uint32 pixel_color,SDL_PixelFormat* format);
Uint32 ChangeBlueValue(Uint32 pixel_color,Uint8 blue,SDL_PixelFormat* format);
Uint32 ChangeRedValue(Uint32 pixel_color,Uint8 red,SDL_PixelFormat* format);
Uint32 ChangeGreenValue(Uint32 pixel_color,Uint8 green,SDL_PixelFormat* format);
float bilinearly_interpolate_green(int top, int bottom, int left, int right,
			     float horizontal_position, float vertical_position,
			      Uint32* pixels,int w,SDL_PixelFormat* format);
float bilinearly_interpolate_red(int top, int bottom, int left, int right,
			     float horizontal_position, float vertical_position,
			      Uint32* pixels,int w,SDL_PixelFormat* format) ;
float bilinearly_interpolate_blue(int top, int bottom, int left, int right,
			     float horizontal_position, float vertical_position,
			      Uint32* pixels,int w,SDL_PixelFormat* format) ;
void rotation_Color(SDL_Surface* surface1 ,SDL_Surface* surface2, int theta);
Uint32 ChangeValueToBlack(Uint32 pixel_color,SDL_PixelFormat* format);
Uint32 ChangeValueToBlue(Uint32 pixel_color,SDL_PixelFormat* format);
void rotation_binaire(SDL_Surface* surface1 ,SDL_Surface* surface2, int theta);
float bilinearly_interpolate (int top, int bottom, int left, int right,
			     float horizontal_position, float vertical_position,
			      Uint32* pixels,int w,SDL_PixelFormat* format) ;
void rotation(SDL_Surface* surface1 ,SDL_Surface* surface2, int theta);
void copy(SDL_Surface* surface1 ,SDL_Surface* surface2);
int Angle_detection(SDL_Surface* surface1 ,SDL_Surface* surface2);
void Rotation_automatique(SDL_Surface* surface1 ,SDL_Surface* surface2,SDL_Surface* surface3);
#endif
