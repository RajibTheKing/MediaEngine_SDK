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

void AudioMixer::reset(int iNumberOfBitsPerSample, int iFrameSize) // sample size ..
{
	m_iTotalCallee = 0;
	m_iCalleeMaskFlag = 0;
	m_iNumberOfBitsPerSample = iNumberOfBitsPerSample; //
	m_iAudioFrameSize = iFrameSize; //
	m_iTotalBlock = 16;
	m_iCalleeFrameInfoSize = 6;

	memset(m_iMixedData, 0, sizeof m_iMixedData);
	memset(m_uchCalleeBlockInfo, 0, sizeof m_uchCalleeBlockInfo);
	return;
}


int AudioMixer::readValue(unsigned char *uchByteArray, int &iIndexOffset, int &iBitOffset, int iReadBitLength)
{
	int iResult = 0;
	int iTotalBitToRead = iReadBitLength;

	int iBlockSize;
	int iBlockMask;
	int iBlockValue;

	while (iTotalBitToRead)
	{
		iBlockSize = (8 - iBitOffset) > iTotalBitToRead ? iTotalBitToRead : (8 - iBitOffset);
		iBlockMask = (1 << iBlockSize) - 1;
		iBlockMask <<= iBitOffset;
		iBlockValue = (int)uchByteArray[iIndexOffset] & iBlockMask;
		iBlockValue >>= iBitOffset;
		iResult = iResult | (iBlockValue << (iReadBitLength - iTotalBitToRead));

		iTotalBitToRead -= iBlockSize;
		iBitOffset += iBlockSize;
		if (iBitOffset == 8)
		{
			iBitOffset = 0;
			iIndexOffset += 1;
		}
	}

	if (iResult & (1 << (iReadBitLength - 1)))
	{
		iResult ^= ((1 << iReadBitLength) - 1);
		iResult += 1;
		iResult *= -1;
	}
	return iResult;
}
void AudioMixer::writeValue(unsigned char *uchByteArray, int &iIndexOffset, int &iBitOffset, int iWriteBitLength, int iValue)
{
	int iTotalBitToWrite = iWriteBitLength;

	if (iValue > ((1 << iWriteBitLength) - 1))
		iValue = ((1 << iWriteBitLength) - 1);
	
	if (iValue < (-1 * (1 << iWriteBitLength)))
		iValue = -1 * (1 << iWriteBitLength);

	if (iValue < 0) {
		iValue *= -1;
		iValue ^= ((1 << iWriteBitLength) - 1);
		iValue += 1;
	}

	int iBlockSize;
	int iBlockMask;
	int iBlockValue;

	while (iTotalBitToWrite)
	{
		iBlockSize = (8 - iBitOffset) > iTotalBitToWrite ? iTotalBitToWrite : (8 - iBitOffset);
		iBlockMask = (1 << iBlockSize) - 1;
		//iBlockMask <<= (iTotalBitToWrite - iBlockSize);
		iBlockValue = iValue & iBlockMask;
		iValue >>= iBlockSize;
		//iBlockValue >>= (iTotalBitToWrite - iBlockSize);
		iBlockValue <<= iBitOffset;
		// prep to reset block size of iIndex
		//iBlockMask >>= (iTotalBitToWrite - iBlockSize);
		iBlockMask <<= iBitOffset;
		iBlockMask = 0xFF ^ iBlockMask;
		// reset ..
		uchByteArray[iIndexOffset] = uchByteArray[iIndexOffset] & iBlockMask;
		// set
		uchByteArray[iIndexOffset] = uchByteArray[iIndexOffset] | iBlockValue;

		iBitOffset += iBlockSize;
		iTotalBitToWrite -= iBlockSize;

		if (iBitOffset == 8) {
			iBitOffset = 0;
			iIndexOffset++;
		}
	}
	return;
}

void AudioMixer::addAudioData(unsigned char* uchCalleeAudio)
{
	int iCalleeId = uchCalleeAudio[0];
	int iMissingFlag = (uchCalleeAudio[1] << 8) + uchCalleeAudio[2];
	int iOffsetForTotalCalleeAndBit = 2;

	int iAudioSamplePerBlock = m_iAudioFrameSize / m_iTotalBlock;

	memcpy(m_uchCalleeBlockInfo + iOffsetForTotalCalleeAndBit + m_iTotalCallee * m_iCalleeFrameInfoSize, uchCalleeAudio, m_iCalleeFrameInfoSize);

	m_iTotalCallee++;
	m_iCalleeMaskFlag |= (1 << iCalleeId);

	int iIndexOffset = m_iCalleeFrameInfoSize;
	int iBitOffset = 0;

	for (int i = 0; i < m_iAudioFrameSize; i++) 
	{
		if (!(iMissingFlag & (1 << (i / iAudioSamplePerBlock))))
		{
			int value = readValue(uchCalleeAudio, iIndexOffset, iBitOffset, 16);
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

	for (int i = 0; i < m_iAudioFrameSize; i++)
	{
		writeValue(m_uchCalleeBlockInfo, iIndexOffset, iBitOffset, m_iNumberOfBitsPerSample, m_iMixedData[i]);
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

	int iBitOffsetForCallee = 0;
	int iIndexOffsetForCallee = 0;

	int iIndexOffsetForMixed = iTotalCallee * m_iCalleeFrameInfoSize + 2;
	int iBitOffsetForMixed = 0;

	int iBitOffsetForPlayAudio = 0;
	int iIndexOffsetForPlayAudio = 0;

	int iMissingBlocks = -1;
	int iSamplePerBlock = m_iAudioFrameSize / m_iTotalBlock;


	/// Find Callee info block for Callee designated by calleeId
	for (int i = 0; i < iTotalCallee; i++) {
		if (uchMixedAudioData[2 + (i * m_iCalleeFrameInfoSize)] == calleeId)
		{
			iMissingBlocks = ((int)uchMixedAudioData[2 + (i * m_iCalleeFrameInfoSize) + 1] << 8) + (int)uchMixedAudioData[2 + (i * m_iCalleeFrameInfoSize) + 2];
			break;
		}
	}
	
	for (int i = 0; i < m_iAudioFrameSize; i++)
	{
		int calleeValue = readValue(uchCalleeAudioData, iIndexOffsetForCallee, iBitOffsetForCallee, 16);
		int mixedValue = readValue(uchMixedAudioData, iIndexOffsetForMixed, iBitOffsetForMixed, iBitPerSample);

		if (!(iMissingBlocks & (1 << (i / iSamplePerBlock))))
		{
			mixedValue -= calleeValue;
		}

		writeValue(uchAudioDataToPlay, iIndexOffsetForPlayAudio, iBitOffsetForPlayAudio, 16, mixedValue);
	}

	return m_iAudioFrameSize * 2;
}

void AudioMixer::genCalleeChunkHeader(unsigned char* uchDestinaton, int iStartIndex, int iEndIndex, int iCalleeId, int iFrameNumber, int iFrameSize, int iTotalBlock, std::vector<std::pair<int, int>> &vMissingBlocks)
{
	int samplesPerBlocks = (iEndIndex - iStartIndex + 1) / iTotalBlock;

	int IndexOffset = 0;
	int BitOffset = 0;
	
	int iMissingFlag = 0;
	int iMissingVectorIndex = 0;

	uchDestinaton[IndexOffset++] = iCalleeId;

	for (int i = 0; i < (iEndIndex - iStartIndex + 1); i++)
	{
		while (iMissingVectorIndex < (int)vMissingBlocks.size() && vMissingBlocks[iMissingVectorIndex].second < (iStartIndex+i))
		{
			iMissingVectorIndex++;
		}

		if ((iMissingVectorIndex < (int)vMissingBlocks.size()) && ((iStartIndex + i) >= vMissingBlocks[iMissingVectorIndex].first)) {
			iMissingFlag |= (1 << (i / samplesPerBlocks));
		}
	}

	uchDestinaton[IndexOffset++] = (iMissingFlag & ((1 << 8) - 1));
	iMissingFlag >>= 8;
	uchDestinaton[IndexOffset++] = (iMissingFlag & ((1 << 8) - 1));

	uchDestinaton[IndexOffset++] = (iFrameNumber & ((1 << 8) - 1));
	iFrameNumber >>= 8;
	uchDestinaton[IndexOffset++] = (iFrameNumber & ((1 << 8) - 1));
	iFrameNumber >>= 8;
	uchDestinaton[IndexOffset++] = (iFrameNumber & ((1 << 8) - 1));
	iFrameNumber >>= 8;
	return;
}

int AudioMixer::GetAudioFrameByParsingMixHeader(unsigned char *uchByteArray, int nUserId){
	int nNumberOfUsers = uchByteArray[0];
	int nBlock = uchByteArray[1];	
	for (int i = 0; i < nNumberOfUsers; i++)
	{
		int nId = uchByteArray[2 + i * 6];
		int Index = 2 + i * 6 + 3;
		int Offset = 0;
		int bitLen = 24;
		int nFrameNumber = readValue(uchByteArray, Index , Offset, bitLen);
		if (nId == nUserId)
		{
			return nFrameNumber;
		}
	}
	return -1;
}