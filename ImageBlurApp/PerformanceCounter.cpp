#include "PerformanceCounter.h"

PerformanceCounter::PerformanceCounter()
{
	// Pobieramy iloœæ taktów zegara na sekundê (czêstotliwoœæ) i inicjalizujemy zmienne
	QueryPerformanceFrequency(&TemporaryStorage);
	this->Frequency = TemporaryStorage.QuadPart;
	this->StartingTime = 0;
	this->EndingTime = 0;
	this->ElapsedMicroseconds = 0;
	this->FormattedTime = std::string();
}

// funkcja zapisuje wartoœæ pocz¹tkow¹ taktów
bool PerformanceCounter::startCounting()
{
	if (QueryPerformanceCounter(&TemporaryStorage)) {
		StartingTime = TemporaryStorage.QuadPart;
		return true;
	}
	return false;
}

// funkcja zapisuje wartoœæ koñcow¹ taktów
bool PerformanceCounter::stopCounting()
{
	if (QueryPerformanceCounter(&TemporaryStorage)) {
		EndingTime = TemporaryStorage.QuadPart;
		return true;
	}
	return false;
}

// funkcja oblicza wartoœæ godzin, minut, sekund, milisekund i mikrosekund,
// a nastêpnie zamienia na string w formacie HH:MM:SS:mmm:uuu
bool PerformanceCounter::calculateTime()
{
	ElapsedMicroseconds = EndingTime - StartingTime;
	ElapsedMicroseconds *= 1000000;
	ElapsedMicroseconds /= Frequency;
	if (ElapsedMicroseconds >= 0) {
		long long hour, min, sec, milisec, microsec, lastRest;
		hour = (ElapsedMicroseconds - (ElapsedMicroseconds % 3600000000)) / 3600000000;
		min = ((ElapsedMicroseconds % 3600000000) - (ElapsedMicroseconds % 60000000)) / 60000000;
		sec = ((ElapsedMicroseconds % 60000000) - (ElapsedMicroseconds % 1000000)) / 1000000;
		milisec = ((ElapsedMicroseconds % 1000000) - (ElapsedMicroseconds % 1000)) / 1000;
		microsec = ElapsedMicroseconds % 1000;

		std::stringstream ss;
		ss << std::setw(2) << std::setfill('0') << std::to_string(hour);
		std::string sHour = ss.str();
		ss.str(std::string());
		ss << std::setw(2) << std::setfill('0') << std::to_string(min);
		std::string sMin = ss.str();
		ss.str(std::string());
		ss << std::setw(2) << std::setfill('0') << std::to_string(sec);
		std::string sSec = ss.str();
		ss.str(std::string());
		ss << std::setw(3) << std::setfill('0') << std::to_string(milisec);
		std::string sMilisec = ss.str();
		ss.str(std::string());
		ss << std::setw(3) << std::setfill('0') << std::to_string(microsec);
		std::string sMicrosec = ss.str();
		ss.str(std::string());

		FormattedTime = "Czas: " + sHour + ":" + sMin + ":" + sSec + ":" + sMilisec + ":" + sMicrosec;

		return true;
	}
	return false;
}

std::string PerformanceCounter::getTime()
{
	return FormattedTime;
}
