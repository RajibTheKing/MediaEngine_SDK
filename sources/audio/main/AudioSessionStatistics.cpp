#include "AudioSessionStatistics.h"
#include "Tools.h"
#include "AudioTypes.h"
#include "InterfaceOfAudioVideoEngine.h"
#include <string>
#include <algorithm>

namespace MediaSDK
{
	// Name of the Informations:
	const std::string AudioSessionStatistics::m_sSessionStatNameForLog[SIZE_OF_SESSION_STATISTICS_NAME] = {
		"",											//0
		"Delay",									//1
		"Delay",									//2
		"Delay Fraction",							//3
		"Delay Fraction",							//4
		"Start Up Farend Buffer Size",				//5
		"Start Up Farend Buffer Size",				//6
		"Current Max",								//7	
		"Current Max",								//8
		"Min",										//9
		"Min",										//10
		"Average Time Difference",					//11
		"Average Time Difference",					//12
		"Call in Live",								//13
		"",											//14
		"Call Count",								//15
		"",											//16						
		"Total Data Size",							//17
		"Total Data Size"							//18
	};

	// Byte Size of the Informations
	const int AudioSessionStatistics::m_iaSessionStatByteSize[SIZE_OF_SESSION_STATISTICS_NAME] = {
		-1,											//0
		2,											//1
		2,											//2
		1,											//3
		1,											//4
		1,											//5
		1,											//6
		1,											//7
		1,											//8
		1,											//9
		1,											//10
		1,											//11
		1,											//12
		1,											//13
		-1,											//14
		1,											//15
		-1,											//16
		4,											//17
		4											//18
	};

	// Index of the Publisher[0] and VieweInCall[1] Informations
	const int AudioSessionStatistics::m_iaSessionStatDelay[2] = { 1, 2 };
	const int AudioSessionStatistics::m_iaSessionStatDelayFraction[2] = { 3, 4 };
	const int AudioSessionStatistics::m_iaSessionStatStartUpFarendBufferSize[2] = { 5, 6 };
	const int AudioSessionStatistics::m_iaSessionStatCurrentFarendBufferSizeMax[2] = { 7, 8 };
	const int AudioSessionStatistics::m_iaSessionStatCurrentFarendBufferSizeMin[2] = { 9, 10 };
	const int AudioSessionStatistics::m_iaSessionStatAverageRecorderTimeDiff[2] = { 11, 12 };
	const int AudioSessionStatistics::m_iaSessionStatIsCalling = 13;
	const int AudioSessionStatistics::m_iaSessionStatCountCall = 15;
	const int AudioSessionStatistics::m_iaSessionStatTotalDataSz[2] = { 17, 18 };

	AudioSessionStatistics::AudioSessionStatistics(int EntityType)
	{
		if (EntityType == ENTITY_TYPE_PUBLISHER_CALLER || EntityType == ENTITY_TYPE_PUBLISHER)
			m_id = 0;
		else m_id = 1;

		m_llCallCount = 0;
		m_llTimeDiff = 0;
		m_llTotalDataSz = 0;
		m_umSessionStat[m_iaSessionStatCountCall] = 0;
		ResetAll();

		m_pAudioSessionStatMutex.reset(new CLockHandler);
	}

	void AudioSessionStatistics::ResetAll()
	{
		m_umSessionStat.clear();
		for (int i = 0; i < 2; i++)
		{
			m_umSessionStat[m_iaSessionStatDelay[i]] = SHRT_MAX;
			m_umSessionStat[m_iaSessionStatDelayFraction[i]] = 255;
			m_umSessionStat[m_iaSessionStatCurrentFarendBufferSizeMax[i]] = SHRT_MIN;
			m_umSessionStat[m_iaSessionStatCurrentFarendBufferSizeMin[i]] = SHRT_MAX;
			m_umSessionStat[m_iaSessionStatAverageRecorderTimeDiff[i]] = 0;
			m_umSessionStat[m_iaSessionStatTotalDataSz[i]] = 0;
		}
		m_llLastTime = -1;
	}

	void AudioSessionStatistics::ResetVaryingData(int end)
	{
		m_llTimeDiff = 0;
		m_llTotalDataSz = 0;
		if (end > 2) end = 2;
		for (int i = 0; i < end; i++)
		{
			m_umSessionStat[m_iaSessionStatCurrentFarendBufferSizeMax[i]] = SHRT_MIN;
			m_umSessionStat[m_iaSessionStatCurrentFarendBufferSizeMin[i]] = SHRT_MAX;
			m_umSessionStat[m_iaSessionStatAverageRecorderTimeDiff[i]] = 0;
			m_umSessionStat[m_iaSessionStatTotalDataSz[i]] = 0;
		}
	}

	void AudioSessionStatistics::SetSessionStatisticsOfViewerInCall(unsigned char *ucaInfo, int len)
	{
		BaseMediaLocker lock(*m_pAudioSessionStatMutex);
		memcpy(m_ucaStatViewerInCall, ucaInfo, len);
		m_iStatInfoOfViewerInCallLen = len;
	}

	int AudioSessionStatistics::GetChunk(unsigned char* ucaInfo, int inCall)
	{
		BaseMediaLocker lock(*m_pAudioSessionStatMutex);

		if (inCall)
		{
			m_umSessionStat[m_iaSessionStatIsCalling] = 1;
		}
		else
		{
			m_umSessionStat[m_iaSessionStatIsCalling] = 0;
		}

		if (m_id == 0)
		{
			m_umSessionStat[m_iaSessionStatCountCall] = m_llCallCount;
		}

		m_umSessionStat[m_iaSessionStatAverageRecorderTimeDiff[m_id]] = m_llTimeDiff;
		m_umSessionStat[m_iaSessionStatTotalDataSz[m_id]] = m_llTotalDataSz;

		int lenTillNow = 0;
		for (int i = 0; i < SIZE_OF_SESSION_STATISTICS_NAME; i++)
		{
			if (i % 2 == m_id) continue;
			if (m_iaSessionStatByteSize[i] == -1) continue;
			if (m_umSessionStat.find(i) == m_umSessionStat.end()) continue;

			long long val = m_umSessionStat[i];
			if (i == 11 || i == 12)
			{
				val /= SESSION_STATISTICS_PACKET_INTERVAL;
			}
			int len = m_iaSessionStatByteSize[i];
			int type = i;

			ucaInfo[lenTillNow++] = len;
			ucaInfo[lenTillNow++] = type;
			for (int j = 0; j < len; ++j, lenTillNow++)
			{
				ucaInfo[lenTillNow] = (unsigned char)((val >> (len - 1 - j) * 8) & 0xFFu);
			}
		}

		// Set Viewer In Call data only for Publisher
		if (m_id == 0)
		{
			memcpy(ucaInfo + lenTillNow, m_ucaStatViewerInCall, m_iStatInfoOfViewerInCallLen);
			lenTillNow += m_iStatInfoOfViewerInCallLen;
		}

		return lenTillNow;
	}

	void AudioSessionStatistics::GenerateReport(unsigned char *ucaInfo, int len)
	{
		int i;
		std::vector < std::pair < int, long long > > v;
		for (i = 0; i < len;)
		{
			int nInfoSize = ucaInfo[i++];
			int nInfoType = ucaInfo[i++];

			long long val = 0;
			for (int j = 1; j <= nInfoSize; ++j, ++i)
			{
				val = val | ucaInfo[i];
				val = val << 8;
			}
			val = val >> 8;

			v.push_back(std::make_pair(nInfoType, val));
		}

		// For printing the Information Purpose
		std::string sLogPrint = "";
		for (int i = 0; i < (int)v.size(); i++)
		{
			int type = v[i].first;
			long long value = v[i].second;
			if (type % 2 == 1)
			{
				if (type < SIZE_OF_SESSION_STATISTICS_NAME)
					sLogPrint = sLogPrint + " " + m_sSessionStatNameForLog[type] + ": " + Tools::LongLongToString(value);
				else
					sLogPrint = sLogPrint + " " + Tools::LongLongToString((long long)type) + ": " + Tools::LongLongToString(value);
			}
		}

		MediaLog(LOG_DEBUG, "[ADE] Publisher Info Size: %d", (int)sLogPrint.size());

		if (sLogPrint.size() > 0) MediaLog(LOG_DEBUG, "[ADE] Publisher Info -%s", sLogPrint.c_str());

		sLogPrint = "";
		for (int i = 0; i < (int)v.size(); i++)
		{
			int type = v[i].first;
			long long value = v[i].second;
			if (type % 2 == 0)
			{
				if (type < SIZE_OF_SESSION_STATISTICS_NAME)
					sLogPrint = sLogPrint + " " + m_sSessionStatNameForLog[type] + ": " + Tools::LongLongToString(value);
				else
					sLogPrint = sLogPrint + " " + Tools::LongLongToString((long long)type) + ": " + Tools::LongLongToString(value);
			}
		}
		MediaLog(LOG_DEBUG, "[ADE] Viewer Info Size: %d", (int)sLogPrint.size())
			if (sLogPrint.size() > 0) MediaLog(LOG_DEBUG, "[ADE] Callee Info -%s", sLogPrint.c_str());
	}

	void AudioSessionStatistics::CallStarted()
	{
		m_llCallCount++;
	}

	void AudioSessionStatistics::UpdateStartingBufferSize(long long Sz)
	{
		m_umSessionStat[m_iaSessionStatStartUpFarendBufferSize[m_id]] = Sz;
	}

	void AudioSessionStatistics::UpdateCurrentBufferSize(long long Sz)
	{
		long long mx = (std::max)(m_umSessionStat[m_iaSessionStatCurrentFarendBufferSizeMax[m_id]], Sz);
		long long mn = (std::min)(m_umSessionStat[m_iaSessionStatCurrentFarendBufferSizeMin[m_id]], Sz);
		m_umSessionStat[m_iaSessionStatCurrentFarendBufferSizeMax[m_id]] = mx;
		m_umSessionStat[m_iaSessionStatCurrentFarendBufferSizeMin[m_id]] = mn;
	}

	void AudioSessionStatistics::UpdateOnDataArrive(int Sz)
	{
		long long llCurrentTime = Tools::CurrentTimestamp();

		if (m_llLastTime == -1)
		{
			m_llLastTime = llCurrentTime;
		}

		long long llTimeDiff = llCurrentTime - m_llLastTime;

		m_llTimeDiff += llTimeDiff;
		m_llTotalDataSz += Sz;

		m_llLastTime = llCurrentTime;
	}

	void AudioSessionStatistics::UpdateEchoDelay(int delay, int delayFraction)
	{
		m_umSessionStat[m_iaSessionStatDelay[m_id]] = delay;
		m_umSessionStat[m_iaSessionStatDelayFraction[m_id]] = delayFraction;
	}

};