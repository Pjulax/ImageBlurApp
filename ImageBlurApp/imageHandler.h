#pragma once
#include <cmath>
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
	bool saveHeader();
	bool saveImagePart(uint32_t startIndx, uint32_t endIndx);
	bool loadImagePart();
	void blurImage();
	uint32_t* inputHistogramCalc();
	uint32_t* outputHistogramCalc();
};