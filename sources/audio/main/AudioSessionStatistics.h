
#include "MediaLogger.h"
#include <vector>
#include "AudioMacros.h"
#include <unordered_map>
#include "AudioTypes.h"

// Max Size of the Informations
#define SIZE_OF_SESSION_STATISTICS_NAME 19

namespace MediaSDK
{
	class AudioSessionStatistics : public SessionStatisticsInterface
	{

	public:
		AudioSessionStatistics(int EntityType);
		~AudioSessionStatistics() { };

		void ResetAll();
		void ResetVaryingData(int end = 2);
		void SetSessionStatisticsOfViewerInCall(unsigned char *ucaInfo, int len);
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
		std::unordered_map < int, long long > m_umSessionStat;

		unsigned char m_ucaStatViewerInCall[SESSION_STATISTICS_MAX_SIZE];
		int m_iStatInfoOfViewerInCallLen;

		static const std::string m_sSessionStatNameForLog[SIZE_OF_SESSION_STATISTICS_NAME];
		static const int m_iaSessionStatByteSize[SIZE_OF_SESSION_STATISTICS_NAME];

		// Index of the Publisher[0] and VieweInCall[1] Informations
		static const int m_iaSessionStatDelay[2];
		static const int m_iaSessionStatDelayFraction[2];
		static const int m_iaSessionStatStartUpFarendBufferSize[2];
		static const int m_iaSessionStatCurrentFarendBufferSizeMax[2];
		static const int m_iaSessionStatCurrentFarendBufferSizeMin[2];
		static const int m_iaSessionStatAverageRecorderTimeDiff[2];
		static const int m_iaSessionStatIsCalling;
		static const int m_iaSessionStatCountCall;
		static const int m_iaSessionStatTotalDataSz[2];

		SharedPointer<CLockHandler> m_pAudioSessionStatMutex;
	};

};