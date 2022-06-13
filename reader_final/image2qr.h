#ifndef IMAGE2QR_H
#define IMAGE2QR_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

SDL_Surface *IMG_Load(const char *file);
void image2qr(SDL_Surface *img);
void init_sdl();
SDL_Surface* load_image(char *path);
SDL_Surface* display_image(SDL_Surface *img);
void wait_for_keypressed();
void SDL_FreeSurface(SDL_Surface *surface);
void display(char* path);
void binary(SDL_Surface *img);

#endif
