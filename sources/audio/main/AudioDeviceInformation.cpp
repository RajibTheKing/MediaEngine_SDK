#include "AudioDeviceInformation.h"

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
			m_ucaBuffer[i] = (unsigned char)((ullInfo >> (nInfoSize - 1 - i) * 8) & 0xFFu);
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
		for (i = 0; i < len; ++i)
		{
			int nInfoSize = ucaInfo[i++];
			int nInfoType = ucaInfo[i++];

			int j = 1;
			long long val = 0;
			for (; j <= nInfoSize; j++, i++)
			{
				val = ucaInfo[i];
				val = val << 8;
			}
			v.push_back(std::make_pair(nInfoType, val));
		}
		return v;
	}
};