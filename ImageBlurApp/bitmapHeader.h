#pragma once
#include <Windows.h>
#include <iostream>
#include <fstream>

#define BITMAP_HEADER_LENGTH 14
#define ONE_GIGABYTE 1 073 741 824
#define PIXEL_ARRAY_SIZE_MAX 357 913 941 // 1/3 z 1 GB zaokr¹glona w dó³ do ca³oœci

class BitmapHeader
{
public:
	unsigned char* header;
	unsigned char* DIBHeader;
	uint16_t imageDataOffset;
	uint16_t bitsPerPixel;
	uint32_t width;
	uint32_t height;

	BitmapHeader(std::string filepath);
};