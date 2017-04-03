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

	int iAudioSamplePerBlock = m_iAudioFrameSize / m_iTotalBlock;

	memcpy(m_uchCalleeBlockInfo + iOffsetForTotalCalleeAndBit + m_iTotalCallee * m_iCalleeFrameInfoSize, uchCalleeAudio, m_iCalleeFrameInfoSize);

	m_iTotalCallee++;
	m_iCalleeMaskFlag |= (1 << iCalleeId);

	for (int i = 0; i < m_iAudioFrameSize; i++) 
	{
		if (!(iMissingFlag & (1 << (i / iAudioSamplePerBlock))))
		{
			int value = (((int)uchCalleeAudio [i * 2 + m_iCalleeFrameInfoSize]) << 8 + ((int)uchCalleeAudio[i * 2 + 1 + m_iCalleeFrameInfoSize]));
			if (value & (1 << 15)) {
				value ^= (1 << 15);
				value *= -1;
			}
			m_iMixedData[i] += value;
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
		currentBit = m_iNumberOfBitsPerSample - 1;
		// For negative value
		int value = m_iMixedData[i];
		while (currentBit) {
			int blockSize = ((8 - iBitOffset) > currentBit ? currentBit : (8 - iBitOffset));
			int blockMask = (((1 << blockSize) - 1) << (currentBit - blockSize));

			int blockValue = (value & blockMask) >> ((currentBit - blockSize));
			m_uchCalleeBlockInfo[iIndexOffset] |= (blockValue << iBitOffset);

			if (iBitOffset == 8) {
				iIndexOffset++;
				iBitOffset = 0;
			}
			value = value >> blockSize;
			currentBit -= blockSize;
		}
		// for negative
		value = m_iMixedData[i];
		if (value < 0) {
			value *= -1;
			m_uchCalleeBlockInfo[iIndexOffset] |= (1 << iBitOffset);
			iBitOffset++;
		}
		else iBitOffset++;

		if (iBitOffset == 8) {
			iIndexOffset++;
			iBitOffset = 0;
		}
	}

	memcpy(uchMixedAudioData, m_uchCalleeBlockInfo, iIndexOffset);
	
	m_iTotalCallee = 0;
	m_iCalleeMaskFlag = 0;
	memset(m_uchCalleeBlockInfo, 0, sizeof m_uchCalleeBlockInfo);
	memset(m_iMixedData, 0, sizeof m_iMixedData);

	return iIndexOffset;
}

int AudioMixer::removeAudioData(unsigned char* uchAudioDataToPlay, unsigned char* uchMixedAudioData, unsigned char* uchCalleeAudioData, int calleeId)
{
	int iTotalCallee = uchMixedAudioData[0];
	int iBitPerSample = uchMixedAudioData[1];
	int iPlayDataIndex = 0;


	int iBitOffset = 0;
	int iIndexOffset = iTotalCallee * 6 + 2;

	int iMissingBlocks = -1;
	int iSamplePerBlock = m_iAudioFrameSize / m_iTotalBlock;

	for (int i = 0; i < iTotalCallee; i++) {
		if (uchMixedAudioData[2 + (i * m_iCalleeFrameInfoSize)] == calleeId)
		{
			iMissingBlocks = ((int)uchMixedAudioData[2 + (i * m_iCalleeFrameInfoSize) + 1] << 8) + (int)uchMixedAudioData[2 + (i * m_iCalleeFrameInfoSize) + 2];
			break;
		}
	}

	for (int i = 0; i < m_iAudioFrameSize; i++)
	{
		int calleeValue = (int)(uchCalleeAudioData[i * 2] << 8) + (int)uchCalleeAudioData[i * 2 + 1];
		if (calleeValue & (1 << 15)) {
			calleeValue ^= (1 << 15);
			calleeValue *= -1;
		}

		int mixedValue = 0;
		int totalBit = m_iNumberOfBitsPerSample - 1;

		while (totalBit) {
			int blockSize = (8 - iBitOffset) > totalBit ?  totalBit : (8 - iBitOffset);
			int blockMask = (1 << blockSize) - 1;
			blockMask <<= iBitOffset;
			mixedValue = (mixedValue << blockSize) | (uchMixedAudioData[iIndexOffset] & blockMask);

			totalBit -= blockSize;
			iBitOffset += blockSize;
			if (iBitOffset == 8)
			{
				iBitOffset = 0;
				iIndexOffset++;
			}
		}

		if (uchMixedAudioData[iIndexOffset] & (1 << iBitOffset)) {
			mixedValue *= -1;
		}

		iBitOffset++;
		if (iBitOffset == 8)
		{
			iBitOffset = 0;
			iIndexOffset++;
		}

		if (!(iMissingBlocks & (1 << (i / iSamplePerBlock))))
		{
			mixedValue -= calleeValue;
		}
		uchAudioDataToPlay[i * 2] = 0;
		uchAudioDataToPlay[i * 2 + 1] = 0;

		if (mixedValue < 0) {
			uchAudioDataToPlay[i * 2] |= (1 << 7);
		}
		int byteMask = (1 << 7) - 1;
		int byteValue = mixedValue & (byteMask << 8);
		byteMask >>= 8;
		uchAudioDataToPlay[i * 2] = byteMask;

		byteMask = (1 << 8) - 1;
		byteValue = mixedValue & byteMask;
		uchAudioDataToPlay[i * 2 + 1] = byteValue;
	}

	return m_iAudioFrameSize * 2;
}