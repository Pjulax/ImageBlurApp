#include "bitmapHeader.h"

BitmapHeader::BitmapHeader(std::string filepath)
{
	std::ifstream image( filepath, std::ios::binary );
	if (image)
	{
		bitmapHeader = new unsigned char[BITMAP_HEADER_LENGTH];
		image.read((char*)bitmapHeader, BITMAP_HEADER_LENGTH);

		std::memcpy(&imageDataOffset, &bitmapHeader[10], sizeof(int));

		uint16_t DIBLength = imageDataOffset - BITMAP_HEADER_LENGTH;

		DIBHeader = new unsigned char[DIBLength];
		image.read((char*)DIBHeader, DIBLength);
		
		std::memcpy(&bitsPerPixel, &DIBHeader[14], sizeof(uint16_t));
		std::memcpy(&width, &DIBHeader[4], sizeof(uint32_t));
		std::memcpy(&height, &DIBHeader[8], sizeof(uint32_t));

		image.close();
	}
}
