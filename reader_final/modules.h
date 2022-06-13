#ifndef QRMODULES_H
#define QRMODULES_H

#include "scanner.h"

void next_bit (scanner_t* scanner);
void skip_bits(scanner_t* scanner, size_t n);

byte mask      (byte m, size_t i, size_t j);
int  mask_grade(scanner_t* scanner, byte m);
void mask_apply(scanner_t* scanner, byte m);

byte get_codeword(scanner_t* scanner);

#endif
