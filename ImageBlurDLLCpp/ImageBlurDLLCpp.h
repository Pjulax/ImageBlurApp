// MathLibrary.h - Contains declarations of math functions
#pragma once

#ifdef IMAGEBLURDLLCPP_EXPORTS
#define IMAGEBLURDLLCPP_API __declspec(dllexport)
#else
#define IMAGEBLURDLLCPP_API __declspec(dllimport)
#endif

// The Fibonacci recurrence relation describes a sequence F
// where F(n) is { n = 0, a
//               { n = 1, b
//               { n > 1, F(n-2) + F(n-1)
// for some initial integral values a and b.
// If the sequence is initialized F(0) = 1, F(1) = 1,
// then this relation produces the well-known Fibonacci
// sequence: 1, 1, 2, 3, 5, 8, 13, 21, 34, ...

// Initialize a Fibonacci relation sequence
// such that F(0) = a, F(1) = b.
// This function must be called before any other function.
extern "C" IMAGEBLURDLLCPP_API void blur_image(unsigned char* inputPixelArray, unsigned char* outputPixelArray, 
												const unsigned int arrayHeight, const unsigned int arrayWidth);

// Produce the next value in the sequence.
// Returns true on success and updates current value and index;
// false on overflow, leaves current value and index unchanged.
extern "C" IMAGEBLURDLLCPP_API bool fibonacci_next();

// Get the current value in the sequence.
extern "C" IMAGEBLURDLLCPP_API unsigned long long fibonacci_current();

// Get the position of the current value in the sequence.
extern "C" IMAGEBLURDLLCPP_API unsigned int fibonacci_index();