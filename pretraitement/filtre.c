#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

#include "rotation.h"
#include "filtre.h"

//Return value of a gray pixel
//
//Pixel_color : value of the pixel
//format : format of the pixel
Uint8 value(Uint32 pixel_color,SDL_PixelFormat* format)
{
  //get value of the pixel (only work if in gray because r = g = b)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  return g;
}


//change the value of the pixel in gray and return the new value in gray
//
//Pixel_color : value of the pixel
//val : new value for the r,g and b componant of the pixel
//format : format of the pixel
Uint32 ChangeValue(Uint32 pixel_color, Uint8 val,SDL_PixelFormat* format)
{
  //(r=g=b because gray format)
  Uint8 r, g, b;
  SDL_GetRGB(pixel_color, format, &r, &g, &b);
  r=val;
  g=val;
  b=val;
  Uint32 color = SDL_MapRGB(format, r, g, b); 
  return color;
}


//Applie GaussianFilter to delete the isolated pixel
//
//surface : the surface which we apply GaussianFilter
void filtrageGaussien(SDL_Surface* surface)
{
  Uint32* pixels = surface->pixels;
  SDL_PixelFormat* format = surface->format;
  int w = surface->w;
  int h = surface->h;
  //copie the pixels to a new temporary list to have the new value in terms of the old value
  int* pixelsTab=malloc(sizeof(int)*h*w);
  for (int k =0;k<h*w;k++)
    {	  
      *(pixelsTab+k)=value(pixels[k],format);
    }
  //for each pixels we applied the GaussienFilter
  for (int k =0;k<h*w;k++)
    {
      int val = 0;
      if (k%w<w-1 && k%w>1 && k/w<h-1 && k/w>1)
	{
	  //Value to get the average value with the GaussienFilter
	  val+=*(pixelsTab+k)*4;
	  val+=*(pixelsTab+k+1)*2;
	  val+=*(pixelsTab+k-1)*2;
	  val+=*(pixelsTab+k+w)*2;
	  val+=*(pixelsTab+k-w)*2;
	  val+=*(pixelsTab+k-w+1);
	  val+=*(pixelsTab+k-w-1);
	  val+=*(pixelsTab+k+w+1);
	  val+=*(pixelsTab+k+w-1);
	  val=val/16;
	  pixels[k]=ChangeValue(pixels[k],val,format);
	}
    }
  free(pixelsTab);
}


//Get average value of pixels list
//
//pixels : the list to get the average
//len : the length of the list
int moyenne(int* pixels,int len)
{
  long int sum = 0;
  
   for (int i =0;i<len;i++)
    {
      sum+=pixels[i];
    }
  return (int) (sum/len);
}

//Get standard deviation value of pixels list
//
//pixels : the list to get the standard deviation
//len : the length of the list
//moyenne : the average of the list
float ecart_type(int* pixels,int len,int moyenne)
{
  float sum = 0;
  
  for (int i =0;i<len;i++)
    {
      sum+=pow(abs(pixels[i]-moyenne),2);
    }
  return sqrtf(sum/len);
}

//Get Variance value of pixels list
//
//pixels : the list to get the Variance
//len : the length of the list
//moyenne : the average of the list
float variance(int* pixels,int len,int moyenne)
{
  float sum = 0;
  
  for (int i =0;i<len;i++)
    {
      sum+=pow(abs(pixels[i]-moyenne),2);
    }
  return sum/(float)(len);
}


//Change value of a pixels depending on the treshold
//
//pixel_color : value of the pixel_color which have to be in gray
//format : format of the pixel
//seuil : treshold the change un black or white
Uint32 pixel_to_blackandwhite(Uint32 pixel_color, SDL_PixelFormat* format,int seuil)
{ 
     Uint8 r, g, b;
     int average=255;
     SDL_GetRGB(pixel_color, format, &r, &g, &b); 
     if (r<seuil)
       average = 0;
     r=average; 
     g=average;
     b=average; 
     Uint32 color = SDL_MapRGB(format, r, g, b); 
     return color; 
} 

//Change the surface to be in black and white
//
//surface : surface to binarise
void Binarisation(SDL_Surface* surface)
{
  //initiaise value
  SDL_PixelFormat* format = surface->format;
  Uint32* pixels = surface->pixels;
  int w =  surface->w ;
  int h =  surface->h ;
  
  int zone = 17;
  //max and min value in gray of the surface
  int maxValue=0;
  int minValue=255;
  //average value of the pixel of the surface
  int MoyenneGlobal=0;
  for (int i=0;i<w*h;i++)
    {
      int valuetempo = value(pixels[i],format);
      MoyenneGlobal+=valuetempo;
      if (valuetempo<minValue)
	minValue=valuetempo;
      if (valuetempo>maxValue)
	maxValue=valuetempo;
    }
  MoyenneGlobal=MoyenneGlobal/(w*h);

  //variable to multiplie the formula 
  float k = 0.40;
  printf("kinitial=%f \n",k);
  if (MoyenneGlobal>200) //if the picture is dusky
     k = 0.36;
  if(MoyenneGlobal<160)//if the picture is clear
    k=0.84;
  if (maxValue-minValue<150) //if the picture have low contrast
    {
      k = k-(((float)(maxValue)+(float)(minValue))/(255.+(float)(minValue)))+0.08;
      printf("LowContrast");
    }
  else if (maxValue-minValue<250)// if the picture have high contrast
    {
      k=k-(((float)(maxValue)+(float)(minValue))/(255.+(float)(minValue)))+0.6;
      printf("HighContrast");
    }
  else
    {
      k=k-(((float)(maxValue)+(float)(minValue))/(255.+(float)(minValue)))+0.85;
    }
  printf("Kfinal=%f \n",k);
  for (int i = zone;i<h-zone;i++)
     {
       for(int j = zone;j<w-zone;j++)
	 {
	   int pixelsTempo[zone*zone];
	   for (int x=i-zone;x<zone;x++)
	     {
	       for(int y=j-zone;y<zone;y++)
		 {
		   pixelsTempo[x*zone+y]=value(pixels[i*zone*w+(j*zone)+x*w+y],format);
		 }
	     }
	   int Moyenne= moyenne(pixelsTempo,zone*zone); //average of the window near the pixel we need to find the value
	   float Ecart_type = ecart_type(pixelsTempo,zone*zone,Moyenne); //standard deviation of the window near the pixel we need to find the value
	   float Variance = 125.; //constant
	   int seuil_local = Moyenne*(1-(k*(1-Ecart_type/Variance))); //apply formula to have the local treshold (Sauvola formula)
	   pixels[i*w+j]=ChangeValue(pixels[i*w+j],pixel_to_blackandwhite(pixels[i*w+j], format, seuil_local),format); 
	   // get the new value of the pixel

	 }
     }
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
    SDL_SaveBMP(surface, "grayscale.bmp");
    rotation(surface,surface2,theta);
    SDL_SaveBMP(surface, "rotation.bmp");
    Uint32* pixels = surface->pixels;
    for(int i=0;i<len;i++)
    {
      pixels[i] = pixel_to_gamma(pixels[i],format);
    }
    filtrageGaussien(surface);
    Binarisation(surface);
    SDL_SaveBMP(surface, "binarisation.bmp");
    SDL_UnlockSurface(surface);
}

//this fonctions rotate the surface
//
//surface : the surface with rotation
//surface2 : the surface that we copie in surface during the rotation
//theta : The angle in ° of the rotation unclockwise
void rotationSurface(SDL_Surface* surface,SDL_Surface* surface2,int theta)
{
    rotation(surface,surface2,theta);
    SDL_SaveBMP(surface, "rotation.bmp");
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
    SDL_SaveBMP(surface, "grayscale.bmp");
    filtrageGaussien(surface); //to delete isolated pixel to have a better binarisation
    Binarisation(surface); 
    SDL_SaveBMP(surface, "Binarisation.bmp");
    SDL_UnlockSurface(surface);
}
