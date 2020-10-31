#pragma once
#include "bitmapHeader.h"

class ImageHandler {
	BitmapHeader bitmapHeader;
	unsigned char* pixelArray;
	uint32_t pixelArraySize;
	uint64_t processedOffset;

public:
	ImageHandler(std::string filepath);
};