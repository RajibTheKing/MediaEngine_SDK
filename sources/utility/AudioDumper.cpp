#include "AudioDumper.h"

namespace MediaSDK
{
	CAudioDumper::CAudioDumper(std::string fileName, bool enable)
	{
		std::string filePath;
#if defined(__ANDROID__)
		std::string filePath = "/sdcard/";
#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
		std::string filePath = std::string(getenv("HOME")) + "/Documents/";
#elif defined(DESKTOP_C_SHARP)
		std::string filePath = "";
#endif
		char prefix[512];
		MediaLogger *loggerInstance = NULL;
		loggerInstance->GetDateTime(prefix);
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