
#include "MediaLogger.h"
#include <vector>
#include "AudioMacros.h"
#include <unordered_map>
#include "AudioTypes.h"

namespace MediaSDK
{
	class AudioDeviceInformation : public DeviceInformationInterface
	{

	public:
		AudioDeviceInformation(int EntityType);
		~AudioDeviceInformation() { };

		void ResetAll();
		void ResetVaryingData(int end = 2);
		void SetDeviceInformationOfViewerInCall(unsigned char *ucaInfo, int len);
		int GetChunk(unsigned char* ucaInfo, int inCall = 0);
		void GenerateReport(unsigned char *ucaInfo, int len);
		
		void CallStarted();
		void UpdateStartingBufferSize(long long Sz);
		void UpdateCurrentBufferSize(long long Sz);
		void UpdateOnDataArrive(int Sz);
		void UpdateEchoDelay(int delay, int delayFraction);
		
	private:
		int m_id;
		long long m_llTimeDiff, m_llTotalDataSz, m_llCallCount, m_llLastTime;
		std::unordered_map < int, long long > m_umDeviceInfo;

		unsigned char m_ucaStatViewerInCall[DEVICE_INFORMATION_MAX_SIZE];
		int m_iStatInfoOfViewerInCallLen;

		SharedPointer<CLockHandler> m_pAudioDeviceInfoMutex;
	};

};