
#include "MediaLogger.h"
#include "AudioMacros.h"
#include <vector>

namespace MediaSDK
{
	// Byte size of different value
	#define ByteSizeDelay 2
	#define ByteSizeDelayFraction 1
	#define ByteSizeFarendSize 1
	#define ByteSizeAverageTimeDiff 1
	#define ByteSizeIsCalling 1
	#define ByteSizeCountCall 1

	// Types of different value
	#define DEVICE_INFORMATION_DELAY 0
	#define DEVICE_INFORMATION_DELAY_FRACTION 1
	#define DEVICE_INFORMATION_STARTUP_FAREND_BUFFER_SIZE 2
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MAX 3
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MIN 4
	#define DEVICE_INFORMATION_AVERAGE_RECORDER_TIME_DIFF 5
	#define DEVICE_INFORMATION_IS_CALLING 6
	#define DEVICE_INFORMATION_COUNT_CALL 7

	#define DEVICE_INFORMATION_DELAY_CALLEE 100
	#define DEVICE_INFORMATION_DELAY_FRACTION_CALLEE 101
	#define DEVICE_INFORMATION_STARTUP_FAREND_BUFFER_SIZE_CALLEE 102
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MAX_CALLEE 103
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MIN_CALLEE 104
	#define DEVICE_INFORMATION_AVERAGE_RECORDER_TIME_DIFF_CALLEE 105
	
	
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