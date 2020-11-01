#include "bitmapHeader.h"

BitmapHeader::BitmapHeader(std::string filepath)
{
	std::ifstream image( filepath, std::ios::binary );
	if (image.is_open())
	{
		header = new unsigned char[BITMAP_HEADER_LENGTH];
		image.read((char*)header, BITMAP_HEADER_LENGTH);

		std::memcpy(&imageDataOffset, &header[10], sizeof(int));

		uint16_t DIBLength = imageDataOffset - BITMAP_HEADER_LENGTH;

		DIBHeader = new unsigned char[DIBLength];
		image.read((char*)DIBHeader, DIBLength);
		
		std::memcpy(&bitsPerPixel, &DIBHeader[14], sizeof(uint16_t));
		std::memcpy(&width, &DIBHeader[4], sizeof(uint32_t));
		std::memcpy(&height, &DIBHeader[8], sizeof(uint32_t));

		image.close();
	}
}