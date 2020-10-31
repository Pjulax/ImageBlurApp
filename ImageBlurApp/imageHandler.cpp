#include "imageHandler.h"

ImageHandler::ImageHandler(std::string filepath) :
	bitmapHeader(filepath)
{
	this->pixelArray = nullptr;
	this->pixelArraySize = 0;
	this->processedOffset = this->bitmapHeader.imageDataOffset;
}
