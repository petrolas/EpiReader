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

	unsigned char message[1666] = {0};  
	int16_t message_length = strlen(freetext);
	printf("INFO: message=[%s]\n", freetext);
	printf("INFO: len of message=%d\n", message_length);
	

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
	if (qr_version > 8) { 
		message_index++;
		message[message_index++] = ((message_length & 5888) >> 4) | ((message_length & 240) >> 4);
	} else { 
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

	unsigned char errorcode[30] = {0}; 

	int16_t total_blocks = message_parameters[1] + message_parameters[3];
	unsigned char interleaved_output[3706] = {0}; 

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
		} else if ((qr_version > 5) && (x == max_pixels-9) && (y==6)) { 
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
	parseMessage("v01.png", argv[1], 0); 

}


