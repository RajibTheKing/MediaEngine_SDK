
#include "MediaLogger.h"
#include <vector>
#include "AudioMacros.h"

namespace MediaSDK
{

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
		unsigned char m_ucaBuffer[DEVICE_INFORMATION_MAX_SIZE];
		int m_nBufferSize;
	};

};