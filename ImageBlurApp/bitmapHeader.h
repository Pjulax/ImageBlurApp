#pragma once
#include <iostream>
#include <fstream>

#define BITMAP_HEADER_LENGTH 14
#define ONE_GIGABYTE 1 073 741 826 // trzykrotnoœæ PIXEL_ARRAY_SIZE_MAX
#define PIXEL_ARRAY_SIZE_MAX 357 913 942 // 1/3 z 1 GB zaokr¹glona w górê do ca³oœci

// class containing elementary data for bitmap header V1
class BitmapHeader
{
public:
	unsigned char* header;
	unsigned char* DIBHeader;
	uint16_t imageDataOffset;
	uint16_t bitsPerPixel;
	uint32_t width;
	uint32_t height;
	// It loads from file Bitmap Header and DIB Bitmap Header, saves offset to pixels data, bits per pixel, width and height of image
	BitmapHeader(std::string filepath);
};