#include "AudioDumper.h"
namespace MediaSDK
{
	CAudioDumper::CAudioDumper(std::string filePath, std::string fileName, bool enable)
	{
		std::string filePathwithFileName = filePath + fileName;
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