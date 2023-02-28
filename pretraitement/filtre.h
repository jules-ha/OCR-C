#ifndef FILTRE_H
#define FILTRE_H

Uint8 value(Uint32 pixel_color,SDL_PixelFormat* format);
Uint32 ChangeValue(Uint32 pixel_color, Uint8 val,SDL_PixelFormat* format);
void filtrageGaussien(SDL_Surface* surface);
int moyenne (int* pixels,int len);
float ecart_type(int* pixels,int len,int moyenne);
float variance(int* pixels,int len,int moyenne);
Uint32 pixel_to_blackandwhite(Uint32 pixel_color, SDL_PixelFormat* format,int seuil);
Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat* format);
Uint32 pixel_to_gamma(Uint32 pixel_color, SDL_PixelFormat* format);
void surface_traitement_rotation(SDL_Surface* surface,SDL_Surface* surface2,int theta);
void Binarisation(SDL_Surface* surface);
void surface_traitement(SDL_Surface* surface);
void rotationSurface(SDL_Surface* surface,SDL_Surface* surface2,int theta);
#endif
