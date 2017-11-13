#include "AudioDumper.h"

namespace MediaSDK
{
#define DATE_TIME_LENGTH 10
	CAudioDumper::CAudioDumper(std::string fileName, bool enable)
	{
		std::string filePath;
#if defined(__ANDROID__)
		filePath = "/sdcard/";
#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
		filePath = std::string(getenv("HOME")) + "/Documents/";
#elif defined(DESKTOP_C_SHARP)
		filePath = "";
#endif
		char prefix[DATE_TIME_LENGTH];
		Tools::GetDateTime(prefix);
		std::string filePathwithFileName = filePath + std::string(prefix) + fileName;
		if (enable) dumpfile = fopen(filePathwithFileName.c_str(), "wb");
	}

	CAudioDumper::~CAudioDumper()
	{
		if (dumpfile) fclose(dumpfile);
	}

	void CAudioDumper::WriteDump(void* audioData, int typeSize, int dataSize, bool enable)
	{
		if (enable) fwrite(audioData, typeSize, dataSize, dumpfile);
	}
}