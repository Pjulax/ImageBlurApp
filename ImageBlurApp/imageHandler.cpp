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
	this->paddingNum = uint16_t(4 - ((bitmapHeader.width * 3) % 4));
	this->isHeaderSaved = false;
}

ImageHandler::~ImageHandler()
{
	delete[] this->inputPixelArray;
	delete[] this->outputPixelArray;
}

void ImageHandler::calculateFragmentSize() {

}

// poprawiæ na wczytywanie kolejnych fragmentów obrazu
bool ImageHandler::loadImagePart()
{
	// ogarn¹æ zapis punktu w pliku gdzie skoñczy³em wczytywaæ by cofn¹æ siê o linie i wczytywaæ nastêpnie od niej znowu
	inputPixelArray = new unsigned char[(bitmapHeader.width + 2) * (bitmapHeader.height + 2) * 3] { 0 };
	std::ifstream image(inputFilepath, std::ifstream::binary);
	if (image.is_open())
	{
		image.seekg(bitmapHeader.imageDataOffset); // tu przechodze do odpowiedniego miejsca w pliku
		for (int i = 1; i <= bitmapHeader.height; i++) {
			image.read((char*)inputPixelArray + (i * (bitmapHeader.width + 2) * 3) + 3, (bitmapHeader.width * 3));
			if(!image.eof() && paddingNum < 4) // pominiêcie paddingu
				image.seekg(paddingNum, std::ios::cur);
		}
		image.close();
		outputPixelArray = new unsigned char[(bitmapHeader.width * bitmapHeader.height * 3)];
		return true;
	}
	return false;
}

void ImageHandler::blurImageDLLCPP() {

	typedef void(CALLBACK* BLURFUNCTION)(unsigned char* inputPixelArray, unsigned char* outputPixelArray,
										const unsigned int arrayHeight, const unsigned int arrayWidth,
										const bool start, const bool end);

	HINSTANCE hDLL;     // Handle to DLL
	BLURFUNCTION procPtr;    // Function pointer

	hDLL = LoadLibraryA("ImageBlurDLLCpp");
	if (hDLL != NULL)
	{
		procPtr = (BLURFUNCTION)GetProcAddress(hDLL, "blur_image");
		if (!procPtr)
		{
			// handle the error
			FreeLibrary(hDLL);
			// here is place to some expection
		}
		else
		{
			// call the function
			procPtr(inputPixelArray, outputPixelArray, bitmapHeader.height, bitmapHeader.width*3, true, true);
		}

		FreeLibrary(hDLL);
	}
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
			image.write((char*)(outputPixelArray + (i * bitmapHeader.width * 3)), bitmapHeader.width * 3);
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

/*
1. Wczytuje header
2. Sprawdzam ile razy musze czytaæ obraz (PIXEL_ARRAY_SIZE_MAX / width) to height mo¿liwa na raz, dzielimy przez 3 bo 3 kolory
3. Sprawdzamy thready ile ustawione
4. Wczytujemy kawa³ek dodaj¹c:
	a) Gdy to pocz¹tek - width+2 czarnych pikseli na pocz¹tku
	b) Zawsze - +1 na pocz¹tku, +1 na koñcu wiersza
	c) Gdy to koniec - width+2 czarnych pikseli na koñcu	
5. Rozdzielamy ka¿d¹ partiê na thready, uruchamiamy i ³¹czymy
6. Zapisujemy fragment
7. Z ka¿dego fragmentu zbieramy histogram z Input i Output Array
8. 



*/