
#ifndef AUDIODUMP_H
#define AUDIODUMP_H
#include <string>
#include "MediaLogger.h"
#include "Tools.h"

namespace MediaSDK
{
	class CAudioDumper
	{
	public:
		CAudioDumper(std::string fileName, bool enable);
		~CAudioDumper();
		void WriteDump(void* audioData, int typeSize, int dataSize);

	private:
		FILE *dumpfile = nullptr;
	};
} //namespace MediaSDK
#endif // !AUDIODUMP_H