// MathLibrary.cpp : Defines the exported functions for the DLL.
#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <utility>
#include <iostream>
#include <limits.h>
#include "ImageBlurDLLCpp.h"

// DLL internal state variables:
static unsigned long long previous_;  // Previous value, if any
static unsigned long long current_;   // Current sequence value
static unsigned index_;               // Current seq. position

// Initialize a Fibonacci relation sequence
// such that F(0) = a, F(1) = b.
// This function must be called before any other function.
void blur_image(unsigned char* inputPixelArray, unsigned char* outputPixelArray,
				const unsigned int arrayHeight, const unsigned int arrayWidth,
				const bool start, const bool end)
{
	uint32_t temp = 0;
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
	}
}

// Produce the next value in the sequence.
// Returns true on success, false on overflow.
bool fibonacci_next()
{
    // check to see if we'd overflow result or position
    if ((ULLONG_MAX - previous_ < current_) ||
        (UINT_MAX == index_))
    {
        return false;
    }

    // Special case when index == 0, just return b value
    if (index_ > 0)
    {
        // otherwise, calculate next sequence value
        previous_ += current_;
    }
    std::swap(current_, previous_);
    ++index_;
    return true;
}

// Get the current value in the sequence.
unsigned long long fibonacci_current()
{
    return current_;
}

// Get the current index position in the sequence.
unsigned int fibonacci_index()
{
    return index_;
}