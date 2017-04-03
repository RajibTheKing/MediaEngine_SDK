#include "AudioMixer.h"
#include "string.h"

AudioMixer::AudioMixer(int iNumberOfBitsPerSample, int iFrameSize) :
m_iTotalCallee(0),
m_iCalleeMaskFlag(0),
m_iNumberOfBitsPerSample(iNumberOfBitsPerSample),
m_iAudioFrameSize(iFrameSize),
m_iTotalBlock(16),
m_iCalleeFrameInfoSize(6)
{
	memset(m_iMixedData, 0, sizeof m_iMixedData);
	memset(m_uchCalleeBlockInfo, 0, sizeof m_uchCalleeBlockInfo);
}

void AudioMixer::addAudioData(unsigned char* uchCalleeAudio)
{
	int iCalleeId = uchCalleeAudio[0];
	int iMissingFlag = uchCalleeAudio[1] << 8 + uchCalleeAudio[2];
	int iOffsetForTotalCalleeAndBit = 2;

	memcpy(m_uchCalleeBlockInfo + iOffsetForTotalCalleeAndBit + m_iTotalCallee * m_iCalleeFrameInfoSize, uchCalleeAudio, m_iCalleeFrameInfoSize);

	m_iTotalCallee++;
	m_iCalleeMaskFlag |= (1 << iCalleeId);

	for (int i = 0; i < m_iAudioFrameSize; i++) 
	{
		if (!(iMissingFlag & (1 << (i / m_iTotalBlock))))
		{
			m_iMixedData[i] += ((uchCalleeAudio[i * 2] + m_iCalleeFrameInfoSize) << 8 + (uchCalleeAudio[i * 2 + 1] + m_iCalleeFrameInfoSize));
		}
	}
	return;
}
int AudioMixer::getAudioData(unsigned char* uchMixedAudioData)
{
	m_uchCalleeBlockInfo[0] = m_iTotalCallee;
	m_uchCalleeBlockInfo[1] = m_iNumberOfBitsPerSample;

	int iIndexOffset = m_iCalleeFrameInfoSize * m_iTotalCallee + 2;
	int iBitOffset = 0;

	int currentBit;
	for (int i = 0; i < m_iAudioFrameSize; i++)
	{
		currentBit = m_iNumberOfBitsPerSample;
		int value = m_iMixedData[i];
		while (currentBit) {
			int blockSize = ((8 - iBitOffset) > currentBit ? currentBit : (8 - iBitOffset));
			m_uchCalleeBlockInfo[iIndexOffset] |= ((value & ((1 << blockSize) - 1)) << iBitOffset);
			if (iBitOffset == 8) {
				iIndexOffset++;
				iBitOffset = 0;
			}
			value = value >> blockSize;
			currentBit -= blockSize;
		}
	}

	memcpy(uchMixedAudioData, m_uchCalleeBlockInfo, iIndexOffset);
	return iIndexOffset;
}

