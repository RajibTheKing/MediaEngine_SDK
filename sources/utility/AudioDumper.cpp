#include "AudioDumper.h"

namespace MediaSDK
{
#define DATE_TIME_LENGTH 10
	CAudioDumper::CAudioDumper(std::string fileName, bool enable)
	{
#ifdef PCM_DUMP
		std::string Directory = Tools::GetCurrentDirectoryAny();
		char prefix[DATE_TIME_LENGTH];
		Tools::GetTime(prefix);
		std::string filePathwithFileName = Directory + std::string(prefix) + "_" + fileName;

		if (enable) dumpfile = fopen(filePathwithFileName.c_str(), "wb");
#endif
	}

	CAudioDumper::~CAudioDumper()
	{
#ifdef PCM_DUMP
		if (dumpfile) fclose(dumpfile);
#endif
	}

	void CAudioDumper::WriteDump(void* audioData, int typeSize, int dataSize)
	{
#ifdef PCM_DUMP
		if (dumpfile) fwrite(audioData, typeSize, dataSize, dumpfile);
#endif
	}
}