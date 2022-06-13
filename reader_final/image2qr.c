#include <string.h>
#include <err.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "image2qr.h"

SDL_Surface *IMG_Load(const char *file);


void init_sdl()
{
	if(SDL_Init(SDL_INIT_VIDEO)==-1)
		errx(1, "Could not initialize SDL: %s.\n",SDL_GetError());
}

SDL_Surface* load_image(char *path)
{
	SDL_Surface *img;
	img = IMG_Load(path);
	if(!img)
		errx(3, "Error loading image : %s",path);
	
	return img;
}

SDL_Surface* display_image(SDL_Surface *img)
{
	SDL_Surface *screen;
	screen = SDL_SetVideoMode(img->w, img->h, 0,SDL_SWSURFACE|SDL_ANYFORMAT);
	if(screen == NULL)
		errx(1, "couldn't set %dx%d video mode: %s\n", img->w, img->h, SDL_GetError());
	
	if(SDL_BlitSurface(img, NULL, screen, NULL)<0)
		warnx("BlitSurface error: %s\n", SDL_GetError());
	
	SDL_UpdateRect(screen, 0,0,img->w, img->h);
	return screen;
}

void wait_for_keypressed()
{
	SDL_Event event;
	do
	{
		SDL_PollEvent(&event);
	}while(event.type != SDL_KEYDOWN);
	
	do
	{
		SDL_PollEvent(&event);
	}while(event.type != SDL_KEYUP);
}

void SDL_FreeSurface(SDL_Surface *surface);

void display(char* path)
{
	SDL_Surface* image_surface;
	SDL_Surface* screen_surface;
	
	init_sdl();
	
	image_surface = load_image(path);
	screen_surface = display_image(image_surface);
	
	wait_for_keypressed();
	
	SDL_FreeSurface(image_surface);
	SDL_FreeSurface(screen_surface);
}

static inline Uint8* pixelref(SDL_Surface *surf, unsigned x, unsigned y)
{
	int bpp = surf->format->BytesPerPixel;
	return (Uint8*)surf->pixels + y * surf->pitch + x*bpp;
}

Uint32 getpixel(SDL_Surface * surface, unsigned x, unsigned y)
{
	Uint8 *p = pixelref(surface, x, y);
	switch(surface->format->BytesPerPixel){
		case 1:
			return *p;
		case 2:
			return *(Uint16 *)p;
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		case 4:
			return *(Uint32 *)p;
	}
	return 0;
}

void putpixel(SDL_Surface *surface, unsigned x, unsigned y, Uint32 pixel)
{
	Uint8* p = pixelref(surface, x, y);
	switch(surface->format->BytesPerPixel){
		case 1:
			*p = pixel;
			break;
		case 2:
			*(Uint16 *)p = pixel;
			break;
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			{
				p[0] = (pixel>>16)&0xff;
				p[1] = (pixel>>8)&0xff;
				p[2] = pixel & 0xff;
			}
			else
			{
				p[0] = pixel & 0xff;
				p[1] = (pixel>>8)&0xff;
				p[2] = (pixel>>16)&0xff;
			}
			break;
		case 4:
			*(Uint32 *)p = pixel;
			break;
	}
}

void binary(SDL_Surface *img)
{
	Uint32 pixel;
	Uint8 r, g, b;
	int w, h;
	w = img->w;
	h = img->h;
	
	for(int i = 0; i<w; i++)
	{
		for(int j = 0; j<h; j++)
		{
			pixel = getpixel(img, i, j);
			SDL_GetRGB(pixel, img->format, &r, &g, &b);
			if(r>=127 && g>=127 && b>=127)
			{
				r = 255;
				g = 255;
				b = 255;

			}else
			{
				r = 0;
				g = 0;
				b = 0;
			}
			pixel = SDL_MapRGB(img->format, r, g, b);
			putpixel(img, i, j, pixel);
		}
	}
}

void image2qr(SDL_Surface *img)
{
	Uint32 pixel;
	Uint8 r, g, b;
	int blockw=0;
	int blockh=0;
	int w, h;
	w = img->w;
	h = img->h;
	
	for(int i = 0; i<w; i++)
	{
		pixel = getpixel(img, i, 0);
		SDL_GetRGB(pixel, img->format, &r, &g, &b);
		if(r==255 && g==255 && b==255)
		{
			blockw++;
		}else
			break;
	}
	
	for(int i = 0; i<h; i++)
	{
		pixel = getpixel(img, blockw/2, i);
		SDL_GetRGB(pixel, img->format, &r, &g, &b);
		if(r==255 && g==255 && b==255)
		{
			blockh++;
		}else
			break;
	}
	printf("%i\n", img->h);
	printf("%i\n", img->h/blockh);
	
}
