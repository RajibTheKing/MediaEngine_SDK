
#include "MediaLogger.h"
#include "AudioMacros.h"
#include < vector >

namespace MediaSDK
{

	#define ByteSizeDelay 1
	#define ByteSizeDelayFraction 1
	#define ByteSizeFarendSize 1
	#define ByteSizeAverageTimeDiff 1

	class AudioDeviceInformation
	{

	public:
		AudioDeviceInformation();
		~AudioDeviceInformation();
		void Reset();
		void SetInformation(int nInfoSize, int nInfoType, unsigned long long ullInfo);
		int GetInformation(unsigned char* ucaInfo);
		std::vector < std::pair < int, long long > > ParseInformation(unsigned char *ucaInfo, int len);

	private:

		unsigned char m_ucaBuffer[400];
		int m_nBufferSize;
	};

};