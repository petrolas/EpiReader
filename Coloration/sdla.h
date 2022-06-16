#ifndef SDLA_H
#define SDLA_H

#include <stdlib.h>
#include <SDL.h>

void init_sdl();
SDL_Surface* load_image(char *path);
SDL_Surface* display_image(SDL_Surface *img);
void wait_for_keypressed();
SDL_Surface* copy_image(SDL_Surface *img);
SDL_Surface* Resize(SDL_Surface *img,int a,int b);

#endif