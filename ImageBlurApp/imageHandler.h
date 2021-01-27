#pragma once
#include <cmath>
#include <Windows.h>
#include "bitmapHeader.h"


class ImageHandler {
	//for file loading
	std::string inputFilepath;
	BitmapHeader bitmapHeader;
	unsigned char* inputPixelArray;
	uint32_t pixelArrayStartHeight;
	uint32_t pixelArrayHeight;

	//for file saving
	std::string outputFilepath;
	unsigned char* outputPixelArray;
	bool isHeaderSaved;
	uint16_t paddingNum;

private:
	uint32_t* histogramCalc(unsigned char* pixelArray);

public:
	ImageHandler(std::string inputFilepath, std::string outputFilepath);
	~ImageHandler();
	void calculateFragmentSize();
	// Saves V1 Bitmap Header to file
	bool saveHeader();

	bool saveImagePart(uint32_t startIndx, uint32_t endIndx);
	bool loadImagePart();
	void blurImageDLLCPP();
	uint32_t* inputHistogramCalc();
	uint32_t* outputHistogramCalc();
};