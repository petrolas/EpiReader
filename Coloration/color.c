#include <err.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "pixel_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include "sdla.h"

void init_sdl()
{
    // Init only the video part.
    // If it fails, die with an error message.
    if(SDL_Init(SDL_INIT_VIDEO) == -1)
        errx(1,"Could not initialize SDL: %s.\n", SDL_GetError());
}

SDL_Surface* rotozoomSurface(SDL_Surface* image, double angle, double zoom, double smoth);
SDL_Surface* load_image(char *path)
{
    SDL_Surface *img;

    // Load an image using SDL_image with format detection.
    // If it fails, die with an error message.
    img = IMG_Load(path);
    if (!img)
        errx(3, "can't load %s: %s", path, IMG_GetError());

    return img;
}

SDL_Surface* display_image(SDL_Surface *img)
{
    SDL_Surface *screen;

    // Set the window to the same size as the image
    screen = SDL_SetVideoMode(img->w, img->h, 0, SDL_SWSURFACE|SDL_ANYFORMAT);
    if (screen == NULL)
    {
        // error management
        errx(1, "Couldn't set %dx%d video mode: %s\n",
                img->w, img->h, SDL_GetError());
    }

    // Blit onto the screen surface
    if(SDL_BlitSurface(img, NULL, screen, NULL) < 0)
        warnx("BlitSurface error: %s\n", SDL_GetError());

    // Update the screen
    SDL_UpdateRect(screen, 0, 0, img->w, img->h);

    // return the screen for further uses
    return screen;
}


void wait_for_keypressed()
{
    SDL_Event event;

    // Wait for a key to be down.
    do
    {
        SDL_PollEvent(&event);
    } while(event.type != SDL_KEYDOWN);

    // Wait for a key to be up.
    do
    {
        SDL_PollEvent(&event);
    } while(event.type != SDL_KEYUP);
}

void SDL_FreeSurface(SDL_Surface *surface);

void clean(SDL_Surface *image_surface, Uint8 Red, Uint8 Green, Uint8 Blue)
{
    Uint32 pixel;
    Uint32 testedpixel;
    Uint8 r, g, b;
    pixel = SDL_MapRGB(image_surface -> format, Red, Green, Blue);
    for (int i = 0; i < image_surface->w; i++)
    {
        for (int j = 0; j < image_surface -> h; j++)
        {
            testedpixel = get_pixel(image_surface, i, j);
            SDL_GetRGB(testedpixel, image_surface->format, &r, &g, &b);
            if((0.3*r + 0.59*g + 0.11*b)-127 < 0)
            {
                put_pixel(image_surface, i, j, pixel);
            }
                
	    }

	}
}

int main(int argc, char *argv[]) 
{
	if (argc != 3)
	{
		errx(EXIT_FAILURE,"arg error");
	}
    SDL_Surface* image_surface;
    Uint8 Blue;
    Uint8 Red;
    Uint8 Green;
    for(int i = 0; i < 6; i+=2)
    {
	int num = argv[1][i];
	int num2 = argv[1][i+1];
	
	if (num > 57)
	{
		num -= 87;
	}
	else
	{	
		num -= 48;
	}
	if (num2 > 57)
	{
		num2 -= 87;
	}
	else
	{	
		num2 -= 48;
	}
	num = (num*16) + num2;
	
	if (i == 0)
	{
		Red = num;
	}
	else if (i == 2)
	{
		Green = num;
	}
	else
	{
		Blue = num;
	}
	
    } 

    init_sdl();

    image_surface = load_image(argv[1]);
    
    //image_surface = rotozoomSurface(image_surface, 0, (100/(image_surface->w)), 1);

    clean(image_surface,Red,Green,Blue);
    
    SDL_SaveBMP(image_surface,"img.png");
    
    SDL_FreeSurface(image_surface);
    return 0;
}



