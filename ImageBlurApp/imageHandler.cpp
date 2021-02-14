#include "imageHandler.h"

ImageHandler::ImageHandler(std::string inputFilepath, std::string outputFilepath) :
	bitmapHeader(inputFilepath)
{
	this->inputFilepath = inputFilepath;
	this->outputFilepath = outputFilepath;

	this->inputPixelArray = nullptr;
	this->outputPixelArray = nullptr;
	this->pixelArrayHeight = { 0 };
	this->pixelArrayStartHeight = { 0 };
	this->paddingNum = uint16_t(4 - ((bitmapHeader.width * 3) % 4));
	this->isHeaderSaved = false;
	this->performanceCounter = PerformanceCounter();
	this->memStatEx.dwLength = sizeof(memStatEx);
	GlobalMemoryStatusEx(&memStatEx);
	this->memsize = memStatEx.ullTotalPhys;
	this->maxProgramMemUse = memsize / 3; // for test u can use values like 1324000	// set to maximum 33% memory use for one table - we need two pixel tables
}

ImageHandler::~ImageHandler()
{
	delete[] this->inputPixelArray;
	delete[] this->outputPixelArray;
}

std::string ImageHandler::processImage(const int threadsCount, uint32_t* inputHistogramBGRTab, uint32_t* outputHistogramBGRTab, std::string libDLLSuffix) {

	auto blurImageFun = libDLLSuffix == "Cpp" ? &ImageHandler::blurImageDLLCPP : &ImageHandler::blurImageDLLASM;
	saveHeader();
	int fragmentsCount = calculateFragmentCountAndSize();
	for (int fragmentIndex = 0; fragmentIndex < fragmentsCount; fragmentIndex++) {
		loadImagePart(fragmentIndex);

		// podziel na w�tki tutaj i zr�b Blur + inputHistogram + outputHistogram + saveImagePart(start w w�tku, koniec w w�tku)
		// tu powinno by� wybierz C++ lub ASM zale�nie od flagi czy co�
		std::vector<std::thread> threadVector;
		uint32_t threadHeight = floor(pixelArrayHeight[fragmentIndex] / threadsCount);
		uint32_t heightRemainder = pixelArrayHeight[fragmentIndex];
		

		performanceCounter.startCounting();
		
		for (int threadIndex = 0; threadIndex < threadsCount-1; threadIndex++) {
			heightRemainder -= threadHeight;
			threadPixelArrayStartHeight.push_back(threadIndex * threadHeight);
			threadPixelArrayHeight.push_back(threadHeight);
			std::thread th(blurImageFun, this, threadIndex);
			threadVector.push_back(std::move(th));
		}
		threadPixelArrayStartHeight.push_back((threadsCount - 1) * threadHeight);
		threadPixelArrayHeight.push_back(heightRemainder);
		std::thread th(blurImageFun, this, threadsCount - 1);
		threadVector.push_back(std::move(th));

		for (std::thread& th : threadVector)
			th.join();
		
		performanceCounter.stopCounting();

		inputHistogramCalc(inputHistogramBGRTab, fragmentIndex);
		outputHistogramCalc(outputHistogramBGRTab, fragmentIndex);
		saveImagePart(fragmentIndex);
	}
	return performanceCounter.calculateTime();
}

int ImageHandler::calculateFragmentCountAndSize() {
	uint64_t bmpSize = this->bitmapHeader.width * 3 * this->bitmapHeader.height;
	uint64_t bmpWidth = this->bitmapHeader.width * 3;
	uint32_t rowsAmount = 0; // ilo�� wierszy w fragmentach sta�ej warto�ci 
	int fragmentRatioCeiled = 1; // ilo�� fragment�w
	
	this->pixelArrayHeight.clear(); // czy�cimy wektory na wielko�ci i poczatki tablic
	this->pixelArrayStartHeight.clear();

	if (bmpSize > this->maxProgramMemUse) { // je�li bmp jest wi�kszy ni� ilo�� u�ywalnego przez nas ramu
		rowsAmount = floor(static_cast<double>(maxProgramMemUse) / bmpWidth); // ilo�� wierszy to pami�� ram / szeroko�� wiersza = maksymalna wysoko�� fragmentu obrazu zaokr�glona w d�
		fragmentRatioCeiled = ceil(static_cast<double>(this->bitmapHeader.height) / rowsAmount); // ilo�� fragment�w zaokr�glona w g�r�, ostatni fragment leci po tym co zosta�o, a nie ma wysoko�ci r�wnej rowsAmount

		for (int i = 0; i < fragmentRatioCeiled - 1; i++) { // zapisujemy do wektora wszystkie sta�e bloki obrazu
			this->pixelArrayHeight.push_back(rowsAmount);
			this->pixelArrayStartHeight.push_back(rowsAmount * i);
		}
		this->pixelArrayHeight.push_back(this->bitmapHeader.height - (rowsAmount * (fragmentRatioCeiled - 1))); // zapisujemy do wektora resztkowy ostatni blok obrazu
		this->pixelArrayStartHeight.push_back(rowsAmount * (fragmentRatioCeiled - 1)); // punkt startu w obrazie dla resztkowego fragmentu obrazu
		return fragmentRatioCeiled; // zwracamy ilo�� fragment�w
	}
	else {
		this->pixelArrayHeight.push_back(this->bitmapHeader.height); // zapisujemy ��czn� wielko�� obrazu
		this->pixelArrayStartHeight.push_back(0); // zapisujemy �e startujemy od 0
		return 1; // obraz to ca�y fragment wi�c jest 1
	}
}

bool ImageHandler::loadImagePart(int fragmentIndex)
{
	//Je�li nie pierwsza to zczytaj przed ostatni� lini� i przechowaj j� w temp[bitmapHeader.width]
	unsigned char* tempFirstRow = nullptr;
	if (fragmentIndex > 0 && inputPixelArray != nullptr) {
		tempFirstRow = new unsigned char[bitmapHeader.width * 3];
		for (int i = 0; i < bitmapHeader.width * 3; i++) {
			tempFirstRow[i] = inputPixelArray[pixelArrayHeight[fragmentIndex] * (bitmapHeader.width + 2) * 3 + 3 + i];
		}
		//memcpy((void *)tempFirstRow, (const void*)inputPixelArray[pixelArrayHeight[fragmentIndex] * (bitmapHeader.width + 2) * 3 + 3], bitmapHeader.width * 3);
		delete[] this->inputPixelArray; // je�li to nie pierwsze �adowanie zwolnij pami�� poprzedniej tablicy wej�ciowej
	}
	// ogarn�� zapis punktu w pliku gdzie sko�czy�em wczytywa� by cofn�� si� o linie i wczytywa� nast�pnie od niej znowu
	this->inputPixelArray = new unsigned char[(bitmapHeader.width + 2) * (pixelArrayHeight[fragmentIndex] + 2) * 3] { 0 };
	//Je�li nie pierwsza to wpisz zerow� lini� z temp 
	if (tempFirstRow != nullptr) {
		memcpy((void*)(inputPixelArray + 3), (const void*)tempFirstRow, bitmapHeader.width * 3);
		delete[] tempFirstRow;
	}
	
	std::ifstream image(inputFilepath, std::ifstream::binary);
	if (image.is_open())
	{
		uint64_t fragmentOffset = bitmapHeader.imageDataOffset;
		fragmentOffset += pixelArrayStartHeight[fragmentIndex] * (bitmapHeader.width * 3 + (paddingNum < 4 ? paddingNum : 0));

		image.seekg(fragmentOffset); // tu przechodze do odpowiedniego miejsca w pliku
		for (int i = 1; i <= pixelArrayHeight[fragmentIndex]; i++) {
			image.read((char*)inputPixelArray + (i * (bitmapHeader.width + 2) * 3) + 3, (bitmapHeader.width * 3));
			if(!image.eof() && paddingNum < 4) // pomini�cie paddingu
				image.seekg(paddingNum, std::ios::cur);
		}
		if (!image.eof() && fragmentIndex < pixelArrayStartHeight.size() - 1) {
			image.read((char*)inputPixelArray + ((pixelArrayHeight[fragmentIndex]+1) * (bitmapHeader.width + 2) * 3) + 3, (bitmapHeader.width * 3));
			delete[] outputPixelArray;// je�li to nie ostatnie �adowanie zwolnij pami�� poprzedniej tablicy wynikowej
		}
		//Je�li nie ostatnia to wczytaj kolejn� lini� jako ostatni� (nadmiar z nast�pnego fragmentu w celu jednolitego obrazu)
		image.close();
		outputPixelArray = new unsigned char[(bitmapHeader.width * bitmapHeader.height * 3)];
		return true;
	}
	return false;
}

void ImageHandler::blurImageDLLCPP(int threadFragmentIndex) {

	typedef void(CALLBACK* BLURFUNCTION)(unsigned char* inputPixelArray, unsigned char* outputPixelArray,
										const unsigned int arrayHeight, const unsigned int arrayWidth);

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
			procPtr(inputPixelArray + ((threadPixelArrayStartHeight[threadFragmentIndex]) * (bitmapHeader.width+2) * 3),
				outputPixelArray + (threadPixelArrayStartHeight[threadFragmentIndex] * bitmapHeader.width * 3),
				threadPixelArrayHeight[threadFragmentIndex], bitmapHeader.width*3);
		}

		FreeLibrary(hDLL);
	}
}

void ImageHandler::blurImageDLLASM(int threadFragmentIndex) {

	typedef void(CALLBACK* BLURFUNCTION)(unsigned char* inputPixelArray, unsigned char* outputPixelArray,
		const unsigned int arrayHeight, const unsigned int arrayWidth);

	HINSTANCE hDLL;     // Handle to DLL
	BLURFUNCTION procPtr;    // Function pointer

	hDLL = LoadLibraryA("ImageBlurDLLAsm");
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
			procPtr(inputPixelArray + ((threadPixelArrayStartHeight[threadFragmentIndex]) * (bitmapHeader.width + 2) * 3),
				outputPixelArray + (threadPixelArrayStartHeight[threadFragmentIndex] * bitmapHeader.width * 3),
				threadPixelArrayHeight[threadFragmentIndex], bitmapHeader.width * 3);
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

bool ImageHandler::saveImagePart(int fragmentIndex)
{
	unsigned char* paddingChars = nullptr;
	if (paddingNum > 0) {
		paddingChars = new unsigned char[paddingNum] {0};
	}
	std::ofstream image;
	image.open(outputFilepath, std::ofstream::binary | std::ofstream::app);
	if (image.is_open())
	{
		for (uint32_t i = 0; i < pixelArrayHeight[fragmentIndex]; i++) {

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


void ImageHandler::inputHistogramCalc(uint32_t* inputHistogramBGRTab, int fragmentIndex)
{
	if (this->inputPixelArray != nullptr) {
		for (int pixelHeight = 1; pixelHeight <= this->pixelArrayHeight[fragmentIndex] + 1; pixelHeight++) {
			for (int pixelWidth = 3; pixelWidth < (bitmapHeader.width + 1) * 3; pixelWidth += 3) {
				inputHistogramBGRTab[this->inputPixelArray[(pixelHeight * bitmapHeader.width + 2) * 3 + pixelWidth]]++;
				inputHistogramBGRTab[this->inputPixelArray[(pixelHeight * bitmapHeader.width + 2) * 3 + pixelWidth + 1] + 256]++;
				inputHistogramBGRTab[this->inputPixelArray[(pixelHeight * bitmapHeader.width + 2) * 3 + pixelWidth + 2] + 512]++;

			}
		}
		//for (int pixel = bitmapHeader.width + 3; pixel < (this->pixelArrayHeight[0] + 1) * (bitmapHeader.width+2) * 3; pixel += 3) {
		//	inputHistogramBGRTab[this->inputPixelArray[pixel]]++;
		//	inputHistogramBGRTab[this->inputPixelArray[pixel + 1] + 256]++;
		//	inputHistogramBGRTab[this->inputPixelArray[pixel + 2] + 512]++;
		//}

	}
}

void ImageHandler::outputHistogramCalc(uint32_t* outputHistogramBGRTab, int fragmentIndex)
{
	for (int pixel = 0; pixel < this->pixelArrayHeight[fragmentIndex] * bitmapHeader.width * 3; pixel += 3) {
		outputHistogramBGRTab[this->outputPixelArray[pixel]]++;
		outputHistogramBGRTab[this->outputPixelArray[pixel + 1] + 256]++;
		outputHistogramBGRTab[this->outputPixelArray[pixel + 2] + 512]++;
	}
}


//uint32_t* ImageHandler::histogramCalc(unsigned char* pixelArray)
//{
//	uint32_t* BGR = new uint32_t[768]{ 0 };
//	if (pixelArray != nullptr) {
//		for (int pixel = 0; pixel < bitmapHeader.height * bitmapHeader.width * 3; pixel+=3) {
//			BGR[pixelArray[pixel]]++;
//			BGR[pixelArray[pixel+1]+256]++;
//			BGR[pixelArray[pixel+2]+512]++;
//		}
//		return BGR;
//	}
//	return nullptr;
//}

/*
1. Wczytuje header
2. Sprawdzam ile razy musze czyta� obraz (maxMemoryUse / width*3 ) to height mo�liwa na raz, dzielimy przez 3 bo 3 kolory
3. Sprawdzamy thready ile ustawione
4. Wczytujemy kawa�ek dodaj�c:
	a) Gdy to pocz�tek - width+2 czarnych pikseli na pocz�tku
	b) Zawsze - +1 na pocz�tku, +1 na ko�cu wiersza
	c) Gdy to koniec - width+2 czarnych pikseli na ko�cu
5. Rozdzielamy ka�d� parti� na thready, uruchamiamy i ��czymy
6. Zapisujemy fragment
7. Z ka�dego fragmentu zbieramy histogram z Input i Output Array
8. 



*/