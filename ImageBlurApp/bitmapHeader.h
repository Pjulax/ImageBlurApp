#pragma once
#include <Windows.h>
#include <iostream>
#include <fstream>

#define BITMAP_HEADER_LENGTH 14
#define TWOGIGABYTE 2 147 483 648

class BitmapHeader
{
public:
	unsigned char* bitmapHeader;
	unsigned char* DIBHeader;
	uint16_t imageDataOffset;
	uint16_t bitsPerPixel;
	uint32_t width;
	uint32_t height;

	BitmapHeader(std::string filepath);
};




