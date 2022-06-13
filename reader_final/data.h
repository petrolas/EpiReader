#ifndef DATA_H
#define DATA_H

// 7 by 7 matrix representing the
// finder pattern found in each corner
extern const unsigned char pattern_finder[7][7];

// 5 by 5 matrix representing the
// alignment pattern
extern const unsigned char pattern_alignment[5][5];

// 45-byte long character set for
// the alphanumeric encoding mode
extern const char* charset_alpha;

// version to version range translation
extern const unsigned char version_range[41];

// Error Correction Level is handled as L..H = 0..3
// but they are coded as 1, 0, 3, 2
// yes, it is absurd, but see Table 25
extern const unsigned char code_to_ecl[4];
extern const unsigned char ecl_to_code[4];

// number of bits used to encode the segment length
extern const unsigned char lenbits[5][3]; // [encoding][version range]

// format and version information decoding data
extern const unsigned long bch_format_gen;
extern const unsigned long bch_format_mask;
extern const unsigned long bch_version_gen;
extern const unsigned long bch_version_mask;

// position of alignment patterns for each version
// the v-th row contains the list of coordinates
// the positions are all combinations of this coordinates
// except for the three corner (finder pattern overlaps)
extern const unsigned char pattern_alignment_pos[41][8];

struct BlockRun {
    unsigned int n_blocks;
    unsigned int total_codewords_per_block;
    unsigned int data_codewords_per_block;
};

struct TwoBlockRuns {
    struct BlockRun first;
    struct BlockRun second;
};

// row at index (4 * version + error_correction_level) describes the two runs of blocks
extern const struct TwoBlockRuns block_sizes[164];

#endif
