#include "imageHandler.h"

ImageHandler::ImageHandler(std::string inputFilepath, std::string outputFilepath) :
	bitmapHeader(inputFilepath)
{
	this->inputFilepath = inputFilepath;
	this->outputFilepath = outputFilepath;
	this->inputPixelArray = nullptr;
	this->outputPixelArray = nullptr;
	this->pixelArrayHeight = 0;
	this->pixelArrayStartHeight = 0;
	this->paddingNum = uint16_t(4 - ((bitmapHeader.width*3)%4));
	this->isHeaderSaved = false;
}

bool ImageHandler::saveHeader()
{
	std::ofstream image;
	image.open(outputFilepath, std::ofstream::binary | std::ofstream::trunc);
	if (image.is_open())
	{
		image.write((char*)bitmapHeader.header, BITMAP_HEADER_LENGTH);
		image.write((char*)bitmapHeader.DIBHeader, bitmapHeader.imageDataOffset - BITMAP_HEADER_LENGTH);
		isHeaderSaved = true;
		image.close();
		return true;
	}
	return false;
}

bool ImageHandler::saveImagePart(uint32_t startIndx, uint32_t endIndx)
{
	unsigned char* paddingChars = nullptr;
	if (paddingNum > 0) {
		paddingChars = new unsigned char[paddingNum] {0};
	}
	std::ofstream image;
	image.open(outputFilepath, std::ofstream::binary | std::ofstream::app);
	if (image.is_open())
	{
		pixelArrayHeight = bitmapHeader.height;
		for (uint32_t i = 0; i < pixelArrayHeight; i++) {
			
			// write line of pixels
			image.write((char*)(outputPixelArray + (i * bitmapHeader.width*3)), bitmapHeader.width*3);
			// write padding
			if (paddingChars != nullptr && paddingNum > 0 && paddingNum < 4) {
				
				image.write((char*)paddingChars, paddingNum);
			}
		}
		image.close();
		delete[] paddingChars;
		return true;
	}
	return false;
}

bool ImageHandler::loadImagePart()
{
	inputPixelArray = new unsigned char[(bitmapHeader.width * bitmapHeader.height * 3)];
	std::ifstream image(inputFilepath, std::ifstream::binary);
	if (image.is_open())
	{
		image.seekg(bitmapHeader.imageDataOffset);
		for (int i = 0; i < bitmapHeader.height; i++) {
			image.read((char*)inputPixelArray + (i * bitmapHeader.width*3), (bitmapHeader.width * 3));
			if(!image.eof() && paddingNum < 4)
				image.seekg(paddingNum, std::ios::cur);
		}
		image.close();
		outputPixelArray = new unsigned char[(bitmapHeader.width * bitmapHeader.height * 3)];
		return true;
	}
	return false;
}

void ImageHandler::blurImage()
{
	uint32_t temp = 0;
	int divider = 0;
	for (int line = 0; line < bitmapHeader.height; line++) {
		for (int column = 0; column < bitmapHeader.width * 3; column++) {
			temp = inputPixelArray[line * bitmapHeader.width * 3 + column];
			divider++;
			if (line > 0) {
				temp += inputPixelArray[(line - 1) * bitmapHeader.width * 3 + column];
				divider++;
				if (column > 2) {
					temp += inputPixelArray[(line - 1) * bitmapHeader.width * 3 + column - 3];
					divider++;
				}
				if (column < bitmapHeader.width * 3 - 4) {
					temp += inputPixelArray[(line - 1) * bitmapHeader.width * 3 + column + 3];
					divider++;
				}
			}
			if (column > 2) {
				temp += inputPixelArray[line * bitmapHeader.width * 3 + column - 3];
				divider++;
			}
			if (column < bitmapHeader.width * 3 - 4) {
				temp += inputPixelArray[line * bitmapHeader.width * 3 + column + 3];
				divider++;
			}
			if (line < bitmapHeader.height - 1) {
				temp += inputPixelArray[(line + 1) * bitmapHeader.width * 3 + column];
				divider++;
				if (column > 2){
					temp += inputPixelArray[(line + 1) * bitmapHeader.width * 3 + column - 3];
					divider++;
				}
				if (column < bitmapHeader.width * 3 - 4){
					temp += inputPixelArray[(line + 1) * bitmapHeader.width * 3 + column + 3];
					divider++;
				}
			}
			outputPixelArray[line * bitmapHeader.width * 3 + column] = temp/divider;
			divider = 0;
		}
	}
}

uint32_t* ImageHandler::inputHistogramCalc()
{
	return histogramCalc(this->inputPixelArray);
}

uint32_t* ImageHandler::outputHistogramCalc()
{
	return histogramCalc(this->outputPixelArray);
}

uint32_t* ImageHandler::histogramCalc(unsigned char* pixelArray)
{
	uint32_t* BGR = new uint32_t[768]{ 0 };
	if (pixelArray != nullptr) {
		for (int pixel = 0; pixel < bitmapHeader.height * bitmapHeader.width * 3; pixel+=3) {
			BGR[pixelArray[pixel]]++;
			BGR[pixelArray[pixel+1]+256]++;
			BGR[pixelArray[pixel+2]+512]++;
		}
		return BGR;
	}
	return nullptr;
}
