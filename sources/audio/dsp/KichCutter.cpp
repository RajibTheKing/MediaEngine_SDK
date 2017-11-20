
#include "KichCutter.h"
#include "AudioMacros.h"

KichCutter::KichCutter()
{
	m_sZeroSampleCount = MAX_AUDIO_FRAME_SAMPLE_SIZE / 8; //100
	m_sSpikeSampleCount = MAX_AUDIO_FRAME_SAMPLE_SIZE * 9 / 8; //900
	m_sThreshold = 200;
	m_bRemoveContinentalShelves = false;
	m_nDataBufSize = MAX_AUDIO_FRAME_SAMPLE_SIZE * 3; //2400
	m_nDataBufIOSize = MAX_AUDIO_FRAME_SAMPLE_SIZE; //800
	m_sTotalSpikeSampleCount = m_sSpikeSampleCount + 2 * m_sZeroSampleCount;

	m_sSilentBuf = new short[m_sZeroSampleCount]();
	m_sSilentFillBuf = new short[m_sTotalSpikeSampleCount]();
	m_sTempBuf = new short[m_sTotalSpikeSampleCount]();
	m_sDataBuf = new short[m_nDataBufSize]();
}

KichCutter::~KichCutter()
{
	if (m_sSilentBuf) delete[] m_sSilentBuf;
	if (m_sSilentFillBuf) delete[] m_sSilentFillBuf;
	if (m_sTempBuf) delete[] m_sTempBuf;
	if (m_sDataBuf) delete[] m_sDataBuf;
}