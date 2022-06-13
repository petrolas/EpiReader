#include "blocks.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data.h"
#include "modules.h"
#include "rs.h"


static void get_block(scanner_t* scanner);

static void get_block(scanner_t* scanner)
{
	// get block information
	struct TwoBlockRuns runs = block_sizes[4 * scanner->v + scanner->c];

	// current block
	size_t cur = scanner->block_cur;

	// find the data size of the current block
	size_t codewords_per_block = cur < runs.first.n_blocks ? runs.first.data_codewords_per_block : runs.second.data_codewords_per_block;
	scanner->block_dataw = codewords_per_block;

	// rewind to start of symbol
	scanner->i = scanner->s - 1;
	scanner->j = scanner->s - 1;


	// NOTE: in a symbol, all the blocks have the same number of error
	//       correction codewords but the first series of blocks can
	//       have one data codeword less

	byte nblocks = runs.first.n_blocks + runs.second.n_blocks;



	// BEGIN read data
	// the next section handles the interleaving of data codewords

	// handling the minimal number of codewords
	// n is either codewords_per_block-1 (first blocks) or codewords_per_block-2 (last ones)
	for (size_t i = 0; i < runs.first.data_codewords_per_block; i++)
	{
		skip_bits(scanner, cur * 8);
		scanner->block_data[i] = get_codeword(scanner);
		skip_bits(scanner, (nblocks-cur-1) * 8);
	}

	// interleaving specific to the second series of blocks
	if (cur < runs.first.n_blocks) // first kind of block
	{
		skip_bits(scanner, runs.second.n_blocks * 8);
	}
	else // second kind
	{
		skip_bits(scanner, (cur - runs.first.n_blocks) * 8);
		scanner->block_data[codewords_per_block - 1] = get_codeword(scanner);
		skip_bits(scanner, (nblocks-cur - 1) * 8);
	}

	// END read data



	// the module pointer is now at the end of all the data and at
	// the beginning of the interleaved error correction codewords



	// BEGIN read correction
	// this section handles the interleaving of error correction codewords

	// same for both types of blocks
	size_t n_errwords = runs.first.total_codewords_per_block - runs.first.data_codewords_per_block;
	for (size_t i = 0; i < n_errwords; i++)
	{
		skip_bits(scanner, cur * 8);
		scanner->block_data[codewords_per_block+i] = get_codeword(scanner);
		skip_bits(scanner, (nblocks-cur-1) * 8);
	}

	// END read correction



	// apply Reed-Solomon error correction
	if (rs_decode(codewords_per_block+n_errwords, scanner->block_data, n_errwords) != 0)
	{
		fprintf(stderr, "Could not correct errors\n");
		exit(1);
	}


	scanner->block_cur = cur+1;
	scanner->block_curbyte = 0;
	scanner->block_curbit = 0;
}

unsigned int get_bits(scanner_t* scanner, size_t n)
{
	if (!scanner->block_cur)
		get_block(scanner);

	// this bit-by-bit buffer reading is an abomination
	unsigned int res = 0;
	while (n--)
	{
		if (scanner->block_curbyte >= scanner->block_dataw)
			get_block(scanner);

		size_t B = scanner->block_curbyte;
		size_t b = scanner->block_curbit;
		res *= 2;
		res += (scanner->block_data[B] >> (7 - b)) & 1;

		scanner->block_curbit++;
		if (scanner->block_curbit >= 8)
		{
			scanner->block_curbyte++;
			scanner->block_curbit = 0;
		}
	}
	return res;
}
