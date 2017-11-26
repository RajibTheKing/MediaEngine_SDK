#include "AudioDeviceInformation.h"
#include "Tools.h"
#include "AudioTypes.h"

namespace MediaSDK
{
	AudioDeviceInformation::AudioDeviceInformation()
	{
		m_nBufferSize = 0;
	}


	AudioDeviceInformation::~AudioDeviceInformation()
	{
	}

	void AudioDeviceInformation::Reset()
	{
		m_nBufferSize = 0;
	}

	void AudioDeviceInformation::SetInformation(int nInfoSize, int nInfoType, unsigned long long ullInfo)
	{
		m_ucaBuffer[m_nBufferSize] = nInfoSize;
		m_nBufferSize++;

		m_ucaBuffer[m_nBufferSize] = nInfoType;
		m_nBufferSize++;

		int i = 0;
		for (i = 0; i < nInfoSize; ++i, ++m_nBufferSize)
		{
			m_ucaBuffer[m_nBufferSize] = (unsigned char)((ullInfo >> (nInfoSize - 1 - i) * 8) & 0xFFu);
		}
	}

	int AudioDeviceInformation::GetInformation(unsigned char* ucaInfo)
	{
		for (int i = 0; i < m_nBufferSize; ++i)
		{
			ucaInfo[i] = m_ucaBuffer[i];
		}
		return m_nBufferSize;
	}

	std::vector < std::pair < int, long long > > AudioDeviceInformation::ParseInformation(unsigned char *ucaInfo, int len)
	{
		int i;
		std::vector < std::pair < int, long long > > v;
		for (i = 0; i < len;)
		{
			int nInfoSize = ucaInfo[i++];
			int nInfoType = ucaInfo[i++];

			long long val = 0;
			for (int j=1; j <= nInfoSize; ++j, ++i)
			{
				val = val | ucaInfo[i];
				val = val << 8;
			}
			val = val >> 8;

			v.push_back(std::make_pair(nInfoType, val));
		}

		// For printing the Information Purpose
		std::string sLogPrint = "";
		for (int i = 0; i < v.size(); i++)
		{
			int type = v[i].first;
			long long value = v[i].second;
			if (type % 2 == 1)
			{
				if (type < iSzOfm_sDeviceInformationNameForLog)
					sLogPrint = sLogPrint + " " + m_sDeviceInformationNameForLog[type] + ": " + Tools::LongLongToString(value);
				else
					sLogPrint = sLogPrint + " " + Tools::LongLongToString((long long) type) +": " + Tools::LongLongToString(value);
			}
		}
		MediaLog(LOG_DEBUG, "[ADE] Publisher Info Size: %d", (int)sLogPrint.size())
		if (sLogPrint.size() > 0) MediaLog(LOG_DEBUG, "[ADE] Publisher Info -%s", sLogPrint.c_str());

		sLogPrint = "";
		for (int i = 0; i < v.size(); i++)
		{
			int type = v[i].first;
			long long value = v[i].second;
			if (type % 2 == 0)
			{
				if (type < iSzOfm_sDeviceInformationNameForLog)
					sLogPrint = sLogPrint + " " + m_sDeviceInformationNameForLog[type] + ": " + Tools::LongLongToString(value);
				else
					sLogPrint = sLogPrint + " " + Tools::LongLongToString((long long) type) +": " + Tools::LongLongToString(value);
			}
		}
		MediaLog(LOG_DEBUG, "[ADE] Viewer Info Size: %d", (int)sLogPrint.size())
		if (sLogPrint.size() > 0) MediaLog(LOG_DEBUG, "[ADE] Callee Info -%s", sLogPrint.c_str());

		return v;
	}
};