#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "qr.h"
#include "png_create.h"
#include "err.h"

void printArrayBYTEwithOffset(char* info, uint16_t length, unsigned char data[], unsigned char offset) {
	printf("[size=%d] %s", length, info);
	printf("\n\tBYTE ");
	for (uint16_t i = 0; i < length; i++) {
		printf(">%02x", data[i+offset]);
		if ((i+1) % 32 != 0) printf(",");
		if ((i+1) % 32 == 0) printf("\n\tBYTE ");
	}
	printf("\n");
}

void printArrayBYTE(char* info, uint16_t length, unsigned char data[]) {
	printArrayBYTEwithOffset(info, length, data, 0);
}



void reedSolomon(int16_t data_codewords, int16_t data_offset, unsigned char message[], int16_t error_codewords, unsigned char errorcode[], unsigned char generator[]) {
	for (int16_t i=0; i < data_codewords; i++)
		errorcode[i] = message[i + data_offset];

	for (int16_t i=data_codewords; i < error_codewords; i++) // i
		errorcode[i] = 0;
	//printArrayBYTE("init: ", error_codewords, errorcode);

	for (int16_t j=1; j <= data_codewords; j++) {
		int16_t lead_term = a_inv[errorcode[0]];
		if (errorcode[0] != 0) {
			for (int16_t i=1; i <= error_codewords; i++) {
				unsigned char temp_value = 0;
				if (i < error_codewords) temp_value = errorcode[i];
				errorcode[i-1] = temp_value ^ a[(generator[i-1] + lead_term) % 255];
			}
		} else { // polynomial division step is greatly simiplified (just a shift of all terms left) if leading coeff. is zero
			for (int16_t i=1; i <= error_codewords; i++)
				errorcode[i-1] = errorcode[i];
		}

		for (int16_t i=error_codewords+1; i <= data_codewords; i++) {
			errorcode[i-1] = errorcode[i];
		}
		//printArrayBYTE("iter: ", error_codewords, errorcode);

	}
}

//-----------------------------------------------------------------------------------------------------------------

int is_mask_applicable(int16_t row, int16_t column, unsigned char mask_number) {
	switch (mask_number) {
		case 0: return ((row + column) % 2 == 0);
		case 1: return ((row % 2) == 0);
		case 2: return ((column % 3) == 0);
		case 3: return ((row + column) % 3 == 0);
		case 4: return ((((row / 2) + (column / 3)) % 2) == 0); //double check floor
		case 5: return ((((row * column) % 2) + ((row * column) % 3)) == 0);
		case 6: return ((((row * column) % 2) + ((row * column) % 3)) % 2 == 0);
		case 7: return ((((row + column) % 2) + ((row * column) % 3)) % 2 == 0);
	}
	return 0;
}

void parseMessage(char* filename, const char* freetext, unsigned char test_vector[]) {

	unsigned char message[1666] = {0};  // 244 valid up to VERSION 13-Q, 1666 valid up to VERSION 40-Q
	int16_t message_length = strlen(freetext);
	printf("INFO: message=[%s]\n", freetext);
	printf("INFO: len of message=%d\n", message_length);
	//printArrayBYTE("unencoded input", message_length, (unsigned char*)&freetext[0]);

	int16_t qr_version = -1;
	for (int16_t i=0; i < 40; i++) {
		int16_t capacity = codeword_parameters[i][1]*codeword_parameters[i][2] + codeword_parameters[i][3]*codeword_parameters[i][4] - 2;
		if (i > 8) capacity--; 

		if (message_length <= capacity) {
			qr_version = i;
			printf("INFO: selected QR Version %d\n", qr_version+1);
			break;
		}
	}
	if (qr_version < 0) {
		printf("ERROR: Unable to find QR version capable of encoding input message.  Sorry, try again.\n");
		return;
	}

	unsigned char* message_parameters = codeword_parameters[qr_version];
	int message_index = 0;

	message[message_index] = 64; // "0100" Byte Encoding
	if (qr_version > 8) { // QR Verisons 10+ for Byte data encoding represent length as 16-bits
		message_index++;
		message[message_index++] = ((message_length & 5888) >> 4) | ((message_length & 240) >> 4);
	} else { // QR Verisons 1 through 9 for Byte data encoding represent length as 8-bits
		message[message_index++] |= ((message_length & 240) >> 4);
	}
	message[message_index++] = ((message_length & 15) << 4) | ((freetext[0] & 240) >> 4);

	for (int16_t i=0; i < message_length; i++)		
		message[message_index++] = ((freetext[i] & 15) << 4) | ((freetext[i+1] & 240) >> 4);

	{
		unsigned char pad[] = {236, 17};
		uint16_t pad_index = 0;
		uint16_t needed_pad_bytes = 
			(message_parameters[1] * message_parameters[2])
			+ (message_parameters[3] * message_parameters[4])
			- message_index;
		printf("INFO: needed pad bytes: %d\n", needed_pad_bytes);
		for (uint16_t i=0; i < needed_pad_bytes; i++) {
			message[message_index++] = pad[pad_index];
			pad_index ^= 1;
		}
	}


	int16_t error_codewords = message_parameters[0];

	unsigned char errorcode[30] = {0}; // 30 is highest EC count for Q-quality; 25 is highest dataword count for Q-quality

	int16_t total_blocks = message_parameters[1] + message_parameters[3];
	unsigned char interleaved_output[3706] = {0}; // 532 valid up to VERSION 13-Q; 3706 valid up to VERSION 40-Q

	int16_t message_offset = 0;
	int16_t block_number = 0;
	for (int16_t groups=0; groups < 2; groups++) {
		int16_t num_blocks = message_parameters[groups*2+1];
		int16_t data_codewords = message_parameters[groups*2+2];

		for (int16_t blocks=0; blocks < num_blocks; blocks++) {
			reedSolomon(data_codewords, message_offset, message, error_codewords, errorcode, &gen_poly[gen_offset[message_parameters[0]-13]]);


			int16_t interleaved_output_offset = block_number;
			for (int16_t i=0; i < data_codewords; i++) {
				interleaved_output[interleaved_output_offset] = message[i + message_offset];

				if (i+1 < message_parameters[2]) // { 18, 2, 15, 2, 16}
					interleaved_output_offset += message_parameters[1];
				if (i+1 < message_parameters[4])
					interleaved_output_offset += message_parameters[3];
			}

			interleaved_output_offset = message_parameters[1] * message_parameters[2] + message_parameters[3] * message_parameters[4] + block_number;
			for (int16_t i=0; i < error_codewords; i++) {
				interleaved_output[interleaved_output_offset] = errorcode[i];
				interleaved_output_offset += total_blocks;
			}

			message_offset += data_codewords;
			block_number++;
		}
		//printArrayBYTE("output: ", 346, interleaved_output);
	}

	int16_t output_size = 
		(message_parameters[1] * message_parameters[2]) + (message_parameters[3] * message_parameters[4]) // total data codewords
		+ ((message_parameters[1] + message_parameters[3])) * message_parameters[0]; // total error codewords
	printf("INFO: total output_size=%d bytes\n", output_size);

	if (test_vector != 0) {
		for (int16_t i=0; i < output_size; i++) {
			if (interleaved_output[i] != test_vector[i]) {
				printf("\aERROR: TEST FAILED!  Index=%d\n", i);
				printArrayBYTE("output: ", output_size, interleaved_output);
				return;
			}
		}
		printf("INFO: TEST PASSED!\n");
	}
	//printArrayBYTE("output: ", output_size, interleaved_output);

	int16_t max_pixels = (qr_version*4)+21;
	printf("INFO: pixel size=%d x %d\n", max_pixels, max_pixels);

        unsigned char **image = malloc(max_pixels * sizeof(unsigned char*)); //rows
        if (image == 0) {
                printf("\a!!! ERROR !!! Out of memory during first malloc\n");
                return;
        }
        for (int i=0; i < max_pixels; i++) {
                image[i] = malloc(max_pixels); //columns
                if (image[i] == 0) {
                        printf("\a!!! ERROR !!! Out of memory during second malloc\n");
                        return;
                }
        }

	for (int16_t i=0; i < max_pixels; i++) {
		for (int16_t j=0; j < max_pixels; j++) {
			image[i][j] = 255; // set all pixels to white
		}
	}

	// add the three finder pattern modules to the qr code
	int16_t finder_pattern = (qr_version*4)+14;
	for (int16_t i = 0; i < 7; i++) {
		image[0][i] = 0; //top left module
		image[6][i] = 0;
		image[0][finder_pattern+i] = 0; //top right module
		image[6][finder_pattern+i] = 0;
		image[finder_pattern][i] = 0; //bottom left module
		image[max_pixels-1][i] = 0;
	}
	for (int16_t i = 1; i < 6; i++) {
		image[i][0] = 0; //top left module
		image[i][6] = 0;
		image[i][finder_pattern] = 0; //top right module
		image[i][max_pixels-1] = 0;
		image[finder_pattern+i][0] = 0; //bottom left module
		image[finder_pattern+i][6] = 0;
	}
	for (int16_t i = 2; i < 5; i++) {
		for (int16_t j = 0; j < 3; j++) {
			image[2+j][i] = 0;
			image[2+j][i+finder_pattern] = 0;
			image[finder_pattern+2+j][i] = 0;
		}
	}

	//insert alignment patterns
	if (qr_version > 0) { // no pattern for QR Version 1 (our qr_verison == 0)
		unsigned char center[7] = {0};
		center[0] = 6;
		for (int16_t i=1; i < 7; i++)
			center[i] = message_parameters[5+i];

		for (int16_t i=0; i < 7; i++) {
			for (int16_t j=0; j < 7; j++) {
				if ((center[i] != 0) && (center[j] != 0)) {
					//printf("coord=(%d,%d)\n", center[i], center[j]);
					if (image[center[i]][center[j]] == 255) { //only add if bit is currently white
						image[center[i]][center[j]] = 0;
						for (int16_t k=0; k < 5; k++) {
							image[center[i]-2][center[j]-2+k] = 0;
							image[center[i]+2][center[j]-2+k] = 0;
						}
						for (int16_t k=0; k < 3; k++) {
							image[center[i]-1+k][center[j]-2] = 0;
							image[center[i]-1+k][center[j]+2] = 0;
						}
					}
				}
			}
		}

	}

	//adding timing patterns
	for (int16_t i=8; i < max_pixels - 8; i+=2) {
		image[6][i] = 0;
		image[i][6] = 0;
	}

	//add the "dark module"
	image[(qr_version * 4)+13][8] = 0;

	unsigned char mask_number = 1;
	printf("INFO: using mask %d\n", mask_number);

	//apply mask format info
	{
		int16_t mask = mask_info[mask_number];
		int16_t skip = 0;
		for (int16_t i=0; i < 8; i++) {
			if (i == 6) skip=1;
			if ((mask & 1) > 0) {
				image[8][max_pixels-i-1] = 0;
				image[i+skip][8] = 0;
			}
			mask = mask >> 1;
		}

		skip = 0;
		for (int16_t i=0; i < 7; i++) {
			if (i == 1) skip= -1;
			if ((mask & 1) > 0) {
				image[max_pixels-7+i][8] = 0;
				image[8][7-i+skip] = 0;
			}
			mask = mask >> 1;
		}
	}

	if (qr_version > 5) { 
		int16_t offset = (qr_version-6)*3;
		for (int i=0; i < 3; i++) {
			unsigned char ver = version_info[offset+i];
			for (int j=0; j < 2; j++) {
				for (int k=0; k < 3; k++) {
					if ((ver & 1) > 0) {
						image[0+j+(i*2)][max_pixels-11+k] = 0;
						image[max_pixels-11+k][0+j+(i*2)] = 0;
					}
					ver = ver >> 1;
				}
			}
		}
	}

	//data fill
	int16_t y = max_pixels-1;
	int16_t x = max_pixels-1;
	int16_t dir = -1;

	int16_t primary_bits = output_size * 8;
	int16_t remainder_bits = message_parameters[5];

	printf("INFO: primary bits=%d  remainder bits=%d\n", primary_bits, remainder_bits);

	unsigned char working_byte = 0;
	int16_t interleaved_index = -1;

	for (int i=0; i < primary_bits + remainder_bits; i++) {

		if (image[y][x] == 0) { // check for alignment marker hit
			if (image[y][x-1] == 0) //hit alignment marker head=-on, skip over it
				y = y + dir*5;
			else {  // hit left-hand edge of alignment marker, handle special case
				x = x - 1;
				for (int j=0; j < 5; j++) {
					if (y != 6) { //skip over horitzonal timing line
						if (i % 8 == 0) { working_byte = interleaved_output[++interleaved_index]; } else { working_byte = working_byte << 1; }
						if ((working_byte & 128) > 0) image[y][x] = 0;
						if (is_mask_applicable(y, x, mask_number)) image[y][x]=~image[y][x];
	
						i++;
					}
					y = y + dir;
				}
				x = x + 1;
			}
		}

		if (i < primary_bits) {
			if (i % 8 == 0) { working_byte = interleaved_output[++interleaved_index]; } else { working_byte = working_byte << 1; }
			if ((working_byte & 128) > 0) image[y][x] = 0;
		}
		if (is_mask_applicable(y, x, mask_number)) image[y][x]=~image[y][x]; // handle masking for primary or remainder bit

		i++;
		x = x - 1;

		if (i < primary_bits) {
			if (i % 8 == 0) { working_byte = interleaved_output[++interleaved_index]; } else { working_byte = working_byte << 1; }
			if ((working_byte & 128) > 0) image[y][x] = 0;
		}
		if (is_mask_applicable(y, x, mask_number)) image[y][x]=~image[y][x];

		y = y + dir;
		x = x + 1;

		if (((x < 9) && (y == 8)) || ((x > max_pixels-8) && (y == 8)) || (y < 0)) { // hit top-left or top-right finder patterns
			dir = +1;
			y = y + 1;
			x = x - 2;
		} else if ((qr_version > 5) && (x == max_pixels-9) && (y==6)) { //skip over top 3x6 version info block for large QR Version>=7
			i++; //because of the position of this in if/then rats nest, need to advance bit count, since i wasn't inc'ed yet after the second write
			dir = +1;
			y = 0;
			x = x - 3;
			for (int j=0; j < 6; j++) {
				if (i % 8 == 0) { working_byte = interleaved_output[++interleaved_index]; } else { working_byte = working_byte << 1; }
				if ((working_byte & 128) > 0) image[y][x] = 0;
				if (is_mask_applicable(y, x, mask_number)) image[y][x]=~image[y][x];

				i++;
				y = y + dir;
			}
			i--; //because of the position of this in if/then rats nest, need to take back an i, since i gets inc'ed by the overall for loop at the end
				// i tried moving the block around, but ran into a segfault, and didn't have time to investigate too closely
			x = x + 1;
		} else if ((x == 10) && (y == max_pixels)) { //hit bottom row around "dark module"
			dir = -1;
			y = max_pixels - 9;
			x = x - 2;
		} else if (y == max_pixels) { // hit bottom row
			dir = -1;
			y = max_pixels - 1;
			x = x - 2;
		} else if ((x < 10) && (y > max_pixels-9)) { //hit bottom-left finder pattern (near dark module)
			dir = -1;
			y = max_pixels - 9;
			x = x - 2;
		} else if ((qr_version > 5) && (x < 7) && (y > max_pixels-12)) { //skip over bottom 6x3 version info block for large QR Version>=7
			dir = -1;
			y = max_pixels - 12;
			x = x -2;
		}

		if (y == 6) //skip vertical timing lines
			y += dir;
		else if (x == 6) //skip horizontal timing line
			x = x-1;
	}

	//-----------------------------------

	png_create(max_pixels, max_pixels, image, filename, 4);
	printf("INFO: filename=[%s]\n\n", filename);

	//deallocate memory
	for (int i=0; i < max_pixels; i++)
		free(image[i]);
	free(image);

}

//-----------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[]) 
{
	if (argc != 2)
	{
		errx(EXIT_FAILURE,"arg error");
	}
	//printf("\nSimple QR Code Generator 1.0 - Copyright (C) 2016 joyteq LLC - B.J. Guillot [bguillot@joyteq.com]\n");

	/*unsigned char test_vector[] = {67,246,182,70,85,246,230,247,70,66,247,118,134,7,119,86,87,118,50,194,38,134,7,6,85,242,118,151,194,7,134,50,119,38,87,16,50,86,38,236,6,22,82,17,18,198,6,236,6,199,134,17,103,146,151,236,38,6,50,17,7,236,213,87,148,235,199,204,116,159,11,96,177,5,45,60,212,173,115,202,76,24,247,182,133,147,241,124,75,59,223,157,242,33,229,200,238,106,248,134,76,40,154,27,195,255,117,129,230,172,154,209,189,82,111,17,10,2,86,163,108,131,161,163,240,32,111,120,192,178,39,133,141,236};
	unsigned char test_vector2[] = {66,87,5,38,38,82,86,6,22,134,198,151,199,50,146,7,6,70,182,247,230,118,247,86,119,194,50,6,7,151,118,50,134,16,44,59,46,193,131,150,63,211,215,226,172,171,254,106,247,75,61,110,135,128,210,135,133,15,222,65,27,151,37,178,138,243,22,100,252,163};

	
	parseMessage("v03.png", "Really knows where his towel is!", test_vector2); //V3 -- good
	parseMessage("v05.png", "google.com", test_vector); //V5 - good
*/
	parseMessage("v01.png", argv[1], 0); //V1 - good
	/*parseMessage("v02.png", "Hello World!", 0); //V2 - good

	parseMessage("v04.png", "Frood who really knows where his towel is!", 0); //V4 - good
	parseMessage("v06.png", "There\'s a frood who really knows where his towel is!  Quick brown fox.", 0); //V6 - good
	parseMessage("v07.png", "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", 0); //V7 - good
	parseMessage("v07a.png", "There\'s a frood who really knows where his towel is!  Quick brown fox jumped over.", 0); //V7 - good
	parseMessage("v07b.png", "THEREA# A FR_ODY zzz REALLY KNO:S WHERE HIS TOWEL IS!  QUICK brown fox JUMPED OVER!", 0); //V7 - good
	parseMessage("v07c.png", "therEa# A fr_OdY zzz ReAlLy KnO:s WhErE hiS ToWeL Is!  QuIcK brown fox JuMpEd over!", 0); //V7 - good
	parseMessage("v08.png", "There\'s a frood who really knows where his towel is! And ahoy the quick brown fox jumped over the rusty.", 0); //V8 - good
	parseMessage("v09.png", "There\\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty car.", 0); //V9 - good
	parseMessage("v10.png", "There\\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car.", 0); //V10 - good
	parseMessage("v10a.png", "Thereiss a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car.", 0); //V10 - good
	parseMessage("v11.png", "There\\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V11 - good

	// note: one of the QR scanning apps starting beep twice around the next few lines
	parseMessage("v12.png", "Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V12 - good
	parseMessage("v13.png", "Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V13 - good
	parseMessage("v14.png", "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", 0); //V14 - good
	parseMessage("v14a.png", "Four score and seven years ago.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V14 - good
	parseMessage("v15.png", "Four score and seven years ago our forefathers set forth upon.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V15 - good
	parseMessage("v16.png", "Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V16 - good
	parseMessage("v17.png", "Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V17 - good
	parseMessage("v18.png", "Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V18 - good
	parseMessage("v19.png", "Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V19 - good
	parseMessage("v20.png", "Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V20 - good
	parseMessage("v21.png", "Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V21 - good in QR Scanner, QR Droid ignored
	parseMessage("v22.png", "The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V22 - good in QT Scanner, not QR Droid
	parseMessage("v23.png", "Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V23 - good in QT Scanner, only in ZXing Scanner engine (Zapper nogo)
	parseMessage("v24.png", "Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V24 - good
	parseMessage("v25.png", "Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V25 - good
	parseMessage("v26.png", "Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V26 - good
	parseMessage("v27.png", "I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V27 - good
	parseMessage("v28.png", "You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V28 - good
	parseMessage("v29.png", "The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V29 - good
	parseMessage("v30.png", "Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V30 - good
	parseMessage("v31.png", "Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V31 - good
	parseMessage("v32.png", "Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V32 - good
	parseMessage("v33.png", "Just Beat It.  Just Eat It.  Michael Jackson was a singer.  Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V33 - good
	parseMessage("v34.png", "I'm running out of stuff to write in here for testing purposes.  Just Beat It.  Just Eat It.  Michael Jackson was a singer.  Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V34 - good
	parseMessage("v35.png", "Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune.  I'm running out of stuff to write in here for testing purposes.  Just Beat It.  Just Eat It.  Michael Jackson was a singer.  Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V35 - good
	parseMessage("v36.png", "A normal computer operates off of on and off bits, one and zero.  Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune.  I'm running out of stuff to write in here for testing purposes.  Just Beat It.  Just Eat It.  Michael Jackson was a singer.  Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V36 - good
	parseMessage("v37.png", "A quantum computer has qubits that can be in a superpositon of 1 and 0.  Nice.  A normal computer operates off of on and off bits, one and zero.  Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune.  I'm running out of stuff to write in here for testing purposes.  Just Beat It.  Just Eat It.  Michael Jackson was a singer.  Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V37 - good
	parseMessage("v38.png", "The Java language is nice.  It's typesafe and object-oriented.  Write once, run anywhere.  A quantum computer has qubits that can be in a superpositon of 1 and 0.  Nice.  A normal computer operates off of on and off bits, one and zero.  Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune.  I'm running out of stuff to write in here for testing purposes.  Just Beat It.  Just Eat It.  Michael Jackson was a singer.  Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V38 - good
	parseMessage("v39.png", "C is nice too.  No garbage collection, but it's relatively simple.  The Java language is nice.  It's typesafe and object-oriented.  Write once, run anywhere.  A quantum computer has qubits that can be in a superpositon of 1 and 0.  Nice.  A normal computer operates off of on and off bits, one and zero.  Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune.  I'm running out of stuff to write in here for testing purposes.  Just Beat It.  Just Eat It.  Michael Jackson was a singer.  Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V39 - good
	parseMessage("v40.png", "But the pointers can be rough.  C is nice too.  No garbage collection, but it's relatively simple.  The Java language is nice.  It's typesafe and object-oriented.  Write once, run anywhere.  A quantum computer has qubits that can be in a superpositon of 1 and 0.  Nice.  A normal computer operates off of on and off bits, one and zero.  Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune.  I'm running out of stuff to write in here for testing purposes.  Just Beat It.  Just Eat It.  Michael Jackson was a singer.  Star Trekking Across the Universe.  Only going forward 'cause we can't find reverse.  Data pretty cool too.  But Spock is better than Riker, just barely.  Captain Picard is better that Kirk.  The Corbomite Manuever.  Poker. You green-blooded Vulcan just shows no emotion.  I canna change the laws of physics, Captain.  Captain's Log, Startdate 1234.5.  It's a wonderul day.  Who ya gonna call?  Ghostbusters.  Dum da da dum da da dum dum.  Til the bitter end.  Ode to sweet joy.  Merry Christmas.  Green Eggs and Ham is wonderful wonderful wonderful.  The Cat in the Hat is so Awesome!  Astro.  The Dog.  Lost in Space.  Elroy. George.  Judy.  Jane.  Jetsons, meet the Jetsons.  Space.  Flintstones.  Meet the Flintstones!  Yadda yadda yadda, Seinfeld!  Four score and seven years ago our forefathers set forth upon this continent a new nation.  Silly rabbit.  Twix are for kids!  Whatcha gonna do?  There\'s a frood who really knows where his towel is!  And ahoy the quick brown fox jumped over the rusty stinky rotten no good for nothing car fa la la la la la do re mi.", 0); //V40
	parseMessage("v40a.png", "When in the Course of human events, it becomes necessary for one people to dissolve the political bands which have connected them with another, and to assume among the powers of the earth, the separate and equal station to which the Laws of Nature and of Nature's God entitle them, a decent respect to the opinions of mankind requires that they should declare the causes which impel them to the separation.\n\nWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.\n\nThat to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, --That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.\n\nPrudence, indeed, will dictate that Governments long established should not be changed for light and transient causes; and accordingly all experience hath shewn, that mankind are more disposed to suffer, while evils are sufferable, than to right themselves by abolishing the forms to which they are accustomed.\n\nBut when a long train of abuses and usurpations, pursuing invariably the same Object evinces a design to reduce them under absolute Despotism, it is their right, it is their duty, to throw off such Government, and to provide new Guards for their future security.", 0);
*/
}


