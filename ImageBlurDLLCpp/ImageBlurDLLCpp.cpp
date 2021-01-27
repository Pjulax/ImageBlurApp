#include "pch.h"
#include <utility>
#include <iostream>
#include <limits.h>
#include "ImageBlurDLLCpp.h"

//// DLL internal state variables:
//static unsigned long long previous_;  // Previous value, if any
//static unsigned long long current_;   // Current sequence value
//static unsigned index_;               // Current seq. position

/**
* in
*/
void blur_image(unsigned char* inputPixelArray, unsigned char* outputPixelArray,
				const unsigned int arrayHeight, const unsigned int arrayWidth,
				const bool start, const bool end)
{
	uint32_t temp = 0;
	for (int row = 0; row < arrayHeight; row++) {
		for (int column = 0; column < arrayWidth; column++) {
			temp = (inputPixelArray[row * (arrayWidth + 6) + column] * 0.111)
				+ (inputPixelArray[row * (arrayWidth + 6) + column + 3] * 0.111)
				+ (inputPixelArray[row * (arrayWidth + 6) + column + 6] * 0.111)
				+ (inputPixelArray[(row + 1) * (arrayWidth + 6) + column] * 0.111)
				+ (inputPixelArray[(row + 1) * (arrayWidth + 6) + column + 3] * 0.111) // pixel on input fragment which is [row*arrayWidth + column] pixel on output fragment
				+ (inputPixelArray[(row + 1) * (arrayWidth + 6) + column + 6] * 0.111)
				+ (inputPixelArray[(row + 2) * (arrayWidth + 6) + column] * 0.111)
				+ (inputPixelArray[(row + 2) * (arrayWidth + 6) + column + 3] * 0.111)
				+ (inputPixelArray[(row + 2) * (arrayWidth + 6) + column + 6] * 0.111);
			outputPixelArray[row * arrayWidth + column] = (unsigned char) temp;
		}
	}
	
	/*
	int divider = 0;
	for (int line = 0; line < arrayHeight; line++) {
		for (int column = 0; column < arrayWidth * 3; column++) {
			temp = inputPixelArray[line * arrayWidth * 3 + column];
			divider++;
			if (line > 0) {
				temp += inputPixelArray[(line - 1) * arrayWidth * 3 + column];
				divider++;
				if (column > 2) {
					temp += inputPixelArray[(line - 1) * arrayWidth * 3 + column - 3];
					divider++;
				}
				if (column < arrayWidth * 3 - 4) {
					temp += inputPixelArray[(line - 1) * arrayWidth * 3 + column + 3];
					divider++;
				}
			}
			if (column > 2) {
				temp += inputPixelArray[line * arrayWidth * 3 + column - 3];
				divider++;
			}
			if (column < arrayWidth * 3 - 4) {
				temp += inputPixelArray[line * arrayWidth * 3 + column + 3];
				divider++;
			}
			if (line < arrayHeight - 1) {
				temp += inputPixelArray[(line + 1) * arrayWidth * 3 + column];
				divider++;
				if (column > 2) {
					temp += inputPixelArray[(line + 1) * arrayWidth * 3 + column - 3];
					divider++;
				}
				if (column < arrayWidth * 3 - 4) {
					temp += inputPixelArray[(line + 1) * arrayWidth * 3 + column + 3];
					divider++;
				}
			}
			outputPixelArray[line * arrayWidth * 3 + column] = temp / divider;
			divider = 0;
		}
	}*/


}