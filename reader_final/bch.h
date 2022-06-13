#ifndef BCH_H
#define BCH_H

typedef signed long bch_t;

bch_t bch_check (bch_t g, bch_t code);
bch_t bch_encode(bch_t g, bch_t code);
bch_t bch_decode(bch_t g, bch_t code);

#endif
