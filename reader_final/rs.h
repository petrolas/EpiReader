#ifndef RS_H
#define RS_H

#include "scanner.h"

typedef struct poly poly_t;

// NOTE: for 'synd' and 'pos', the poly_t structure is used as an array and
//       the 'd' parameter is actually the size of this array (the degree of
//       a polynomial is the number of coefficients minus one)
struct poly
{
	size_t d;    // degree
	byte c[512]; // coefficients
};

byte rs_decode(size_t n_data, byte* data, byte n_sym);

#endif
