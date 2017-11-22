
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
	#define DEVICE_INFORMATION_DELAY_PUBLISHER 1
	#define DEVICE_INFORMATION_DELAY_FRACTION_PUBLISHER 3
	#define DEVICE_INFORMATION_STARTUP_FAREND_BUFFER_SIZE_PUBLISHER 5
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MAX_PUBLISHER 7
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MIN_PUBLISHER 9
	#define DEVICE_INFORMATION_AVERAGE_RECORDER_TIME_DIFF_PUBLISHER 11
	#define DEVICE_INFORMATION_IS_CALLING_PUBLISHER 13
	#define DEVICE_INFORMATION_COUNT_CALL_PUBLISHER 15

	#define DEVICE_INFORMATION_DELAY_CALLEE 2
	#define DEVICE_INFORMATION_DELAY_FRACTION_CALLEE 4
	#define DEVICE_INFORMATION_STARTUP_FAREND_BUFFER_SIZE_CALLEE 6
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MAX_CALLEE 8
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MIN_CALLEE 10
	#define DEVICE_INFORMATION_AVERAGE_RECORDER_TIME_DIFF_CALLEE 12

	

	class AudioDeviceInformation
	{

	public:
		AudioDeviceInformation();
		~AudioDeviceInformation();
		void Reset();
		void SetInformation(int nInfoSize, int nInfoType, unsigned long long ullInfo);
		int GetInformation(unsigned char* ucaInfo);
		std::vector < std::pair < int, long long > > ParseInformation(unsigned char *ucaInfo, int len);

		// Name of the Informations:
		const int iSzOfm_sDeviceInformationNameForLog = 16;
		std::string m_sDeviceInformationNameForLog[16] = { "", "[DI] [P] Delay", "[DI] [V] Delay", "[DI] [P] Delay Fraction", "[DI] [V] Delay Fraction", "[DI] [P] Start Up Farend Buffer Size",
			"[DI] [V] Start Up Farend Buffer Size", "[DI] [P] Current Farend Buffer Size Max", "[DI] [V] Current Farend Buffer Size Max", "[DI] [P]  Current Farend Buffer Size Min",
			"[DI] [V]  Current Farend Buffer Size Min", "[DI] [P] Average Recorder Time Difference", "[DI] [V] Average Recorder Time Difference", "[DI] [P] Is Call in Live", "",
			"[DI] [P] Call Counter" };

	private:
		unsigned char m_ucaBuffer[400];
		int m_nBufferSize;
		SharedPointer<CLockHandler> m_pAudioDeviceInformationLocker;
	};

};