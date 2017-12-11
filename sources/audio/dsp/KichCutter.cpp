
#include "KichCutter.h"
#include "AudioMacros.h"
#include "string.h"
#include "stdlib.h"
#include "MediaLogger.h"
namespace MediaSDK
{
	CKichCutter::CKichCutter()
	{
		m_sZeroSampleCount = MAX_AUDIO_FRAME_SAMPLE_SIZE / 8; //100
		m_sSpikeSampleCount = MAX_AUDIO_FRAME_SAMPLE_SIZE * 9 / 8; //900
		m_sThreshold = 200;
		m_bRemoveContinentalShelves = false;
		m_nFramesInBuffer = 3;
		m_nDataBufSize = MAX_AUDIO_FRAME_SAMPLE_SIZE * m_nFramesInBuffer; //2400
		m_nDataBufIOSize = MAX_AUDIO_FRAME_SAMPLE_SIZE; //800
		m_sTotalSpikeSampleCount = m_sSpikeSampleCount + 2 * m_sZeroSampleCount; //1100
		m_nDataBufCopySize = m_nDataBufSize - m_nDataBufIOSize; //1600

		m_sSilentBuf = new short[m_sZeroSampleCount]();
		m_sSilentFillBuf = new short[m_sTotalSpikeSampleCount]();
		m_sTempBuf = new short[m_sTotalSpikeSampleCount]();
		m_sDataBuf = new short[m_nDataBufSize]();
		m_nFlagBuf = new int[m_nFramesInBuffer]();
	}

	CKichCutter::~CKichCutter()
	{
		if (m_sSilentBuf) delete[] m_sSilentBuf;
		if (m_sSilentFillBuf) delete[] m_sSilentFillBuf;
		if (m_sTempBuf) delete[] m_sTempBuf;
		if (m_sDataBuf) delete[] m_sDataBuf;
		if (m_nFlagBuf) delete[] m_nFlagBuf;
	}

	int CKichCutter::Despike(short *sBuffer, int nFlags)
	{
		MediaLog(LOG_DEBUG, "[KC] Kich Cutter despike, flag: %d", nFlags);
		memcpy(m_sDataBuf, m_sDataBuf + m_nDataBufIOSize, m_nDataBufCopySize * sizeof(short));//enqueue sBuffer
		memcpy(m_sDataBuf + m_nDataBufCopySize, sBuffer, m_nDataBufIOSize * sizeof(short));

		memcpy(m_nFlagBuf, m_nFlagBuf + 1, (m_nFramesInBuffer - 1) * sizeof(int));//enqueue nFlags
		m_nFlagBuf[m_nFramesInBuffer - 1] = nFlags;

		for (int i = 0; i < m_nDataBufSize - m_sTotalSpikeSampleCount; i++)
		{
			memcpy(m_sTempBuf, m_sDataBuf + i, sizeof(short) * m_sTotalSpikeSampleCount);
			for (int j = 0; j < m_sSpikeSampleCount; j++)
			{
				if (abs(m_sTempBuf[j]) < m_sThreshold)
				{
					m_sTempBuf[j] = 0;
				}
			}
			if (m_bRemoveContinentalShelves)
			{
				memset(m_sTempBuf + m_sTotalSpikeSampleCount - m_sZeroSampleCount, 0, m_sZeroSampleCount * sizeof(short));
			}

			bool bLeadingZeroFound = false, bTrailingZeroFound = false;

			if (memcmp(m_sTempBuf, m_sSilentBuf, m_sZeroSampleCount * sizeof(short)) == 0)
			{
				bLeadingZeroFound = true;
			}

			if (!bLeadingZeroFound) continue;

			int j = 1;
			for (; j <= m_sSpikeSampleCount; j++)
			{
				if (memcmp(m_sTempBuf + m_sZeroSampleCount + j, m_sSilentBuf, m_sZeroSampleCount * sizeof(short)) == 0)
				{
					bTrailingZeroFound = true;
					break;
				}
			}

			if (bLeadingZeroFound && bTrailingZeroFound)
			{
				memcpy(m_sDataBuf + i, m_sSilentFillBuf, sizeof(short) * (2 * m_sZeroSampleCount + j));
				i = i + m_sZeroSampleCount + j;
			}

		}

		memcpy(sBuffer, m_sDataBuf, m_nDataBufIOSize * sizeof(short));
		return m_nFlagBuf[0];
	}
}