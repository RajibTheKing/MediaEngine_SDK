
#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>

#include "AudioVideoEngineDefinitions.h"

#if defined(TARGET_OS_WINDOWS_PHONE) || defined (_DESKTOP_C_SHARP_) || defined (_WIN32)
#include <windows.h>
#elif defined(TARGET_OS_IPHONE) || defined(__ANDROID__) || defined(TARGET_IPHONE_SIMULATOR)
#include <unistd.h>
#endif

class Tools;

using namespace std;

class Tools
{

public:

	Tools();
	~Tools();

	void SOSleep(int nSleepTimeout);
	static LongLong CurrentTimestamp();
	static std::string DoubleToString(double dConvertingValue);
	static int StringToIntegerConvert(std::string strConvertingString);
	static std::string IntegertoStringConvert(int nConvertingNumber);
	static std::string LongLongtoStringConvert(LongLong number);
	static std::string LongLongToString(LongLong llConvertingValue);

	void WriteToFile(short* saDataToWriteToFile, int nLength);
	void WriteToFile(unsigned char* ucaDataToWriteToFile, int nLength);
	unsigned long long GetTotalSystemMemory();
	unsigned long long GetAvailableSystemMemory();


private:

	FILE *m_filePointerToWriteShortData, *m_filePointerToWriteByteData;
};

#define funcline(...) CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, std::string(__func__) + " : " + Tools::IntegertoStringConvert(__LINE__) + ":" + std::string(__VA_ARGS__));

#endif