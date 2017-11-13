#include "AudioDumper.h"

namespace MediaSDK
{
#define DATE_TIME_LENGTH 10
	CAudioDumper::CAudioDumper(std::string fileName, bool enable)
	{
		std::string Directory = Tools::GetCurrentDirectoryAny();
		char prefix[DATE_TIME_LENGTH];
		Tools::GetTime(prefix);
		std::string filePathwithFileName = Directory + std::string(prefix) + "_" + fileName;

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