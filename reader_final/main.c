#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "pbm.h"
#include "decoder.h"
#include "image2qr.h"

int main(int argc, char** argv)
{
	scanner_t scanner;
	scanner.c = 0;
	scanner.verbosity = 0;
	
	if(argc < 2)
	{
		fprintf(stderr, "You must specify a file\n");
		exit(1);
	}
	const char* data = argv[1];
	
	display((char*)data);
	SDL_Surface* img = load_image((char*)data);
	binary(img);
	image2qr(img);
	int a = SDL_SaveBMP(img, "examples/test2.bmp");
	if(a==-1)
		errx(1, "error in saving");
		
	SDL_FreeSurface(img);
	display("examples/test2.bmp");
	
	FILE* f = data ? fopen(argv[1], "r") : stdin;
	if (!f)
	{
		fprintf(stderr, "Could not open file '%s'\n", data);
		exit(1);
	}
	load_pbm(&scanner, f);
	qrc_decode(&scanner);

	free(scanner.d);
	return 0;
}
