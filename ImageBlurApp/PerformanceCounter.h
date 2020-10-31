#pragma once
#define NOMINMAX
#include <Windows.h>
#include <sstream>
#include <iomanip>

class PerformanceCounter {
	LARGE_INTEGER TemporaryStorage;
	long long StartingTime;
	long long EndingTime;
	long long ElapsedMicroseconds;
	long long Frequency;
	std::string FormattedTime;
public:
	PerformanceCounter();
	bool startCounting();
	bool stopCounting();
	bool calculateTime();
	std::string getTime();
};