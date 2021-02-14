#pragma once
#include <cmath>
#include <Windows.h>
#include "bitmapHeader.h"
#include "PerformanceCounter.h"
#include <vector>
#include <thread>

class ImageHandler {
	//for file loading
	std::string inputFilepath;
	BitmapHeader bitmapHeader;
	unsigned char* inputPixelArray;
	
	//for file saving
	std::string outputFilepath;
	unsigned char* outputPixelArray;
	bool isHeaderSaved;
	uint16_t paddingNum;

	//for image processing
	//for image dividing
	std::vector<uint32_t> pixelArrayStartHeight;
	std::vector<uint32_t> pixelArrayHeight;
	//for threads dividing
	std::vector<uint32_t> threadPixelArrayStartHeight;
	std::vector<uint32_t> threadPixelArrayHeight;
	//time counter
	PerformanceCounter performanceCounter;
	//ram memory
	MEMORYSTATUSEX memStatEx;
	DWORDLONG memsize;
	DWORDLONG maxProgramMemUse;

private:
	uint32_t* histogramCalc(unsigned char* pixelArray);

public:
	ImageHandler(std::string inputFilepath, std::string outputFilepath);
	~ImageHandler();
	std::string processImage(const int threadsCount, uint32_t* inputHistogramBGRTab, uint32_t* outputHistogramBGRTab, std::string libDLLSuffix);
	int calculateFragmentCountAndSize();
	// Saves V1 Bitmap Header to file
	bool saveHeader();

	bool saveImagePart(int fragmentIndex);
	bool loadImagePart(int fragmentIndex);
	void blurImageDLLCPP(int fragmentIndex);
	void blurImageDLLASM(int fragmentIndex);
	void inputHistogramCalc(uint32_t* inputHistogramBGRTab, int fragmentIndex);
	void outputHistogramCalc(uint32_t* outputHistogramBGRTab, int fragmentIndex);
};