
#include "MediaLogger.h"
#include <vector>
#include "AudioMacros.h"
#include <unordered_map>
#include "AudioTypes.h"

// Max Size of the Informations
#define SIZE_OF_DEVICE_INFORMATION_NAME 19

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

		static const std::string m_sDeviceInformationNameForLog[SIZE_OF_DEVICE_INFORMATION_NAME];
		static const int iaDeviceInformationByteSize[SIZE_OF_DEVICE_INFORMATION_NAME];

		// Index of the Publisher[0] and VieweInCall[1] Informations
		static const int iaDeviceInformationDelay[2];
		static const int iaDeviceInformationDelayFraction[2];
		static const int iaDeviceInformationStartUpFarendBufferSize[2];
		static const int iaDeviceInformationCurrentFarendBufferSizeMax[2];
		static const int iaDeviceInformationCurrentFarendBufferSizeMin[2];
		static const int iaDeviceInformationAverageRecorderTimeDiff[2];
		static const int iaDeviceInformationIsCalling;
		static const int iaDeviceInformationCountCall;
		static const int iaDeviceInformationTotalDataSz[2];

		SharedPointer<CLockHandler> m_pAudioDeviceInfoMutex;
	};

};