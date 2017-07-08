#include "AudioMixer.h"
#include <string.h>
#include <algorithm>
#include "MuxHeader.h"
#include "AudioMacros.h"
#include "LogPrinter.h"


namespace MediaSDK
{

	AudioMixer::AudioMixer(int iNumberOfBitsPerSample, int iFrameSize) :
		m_iTotalCallee(0),
		m_iCalleeMaskFlag(0),
		m_iNumberOfBitsPerSample(iNumberOfBitsPerSample),
		m_iAudioFrameSize(iFrameSize),
		m_iTotalBlock(16),
		m_iCalleeFrameInfoSize(AUDIO_MUX_HEADER_LENGHT),
		m_iCalleeIdLengthInByte(8),
		m_iMissingMaskLengthInByte(2),
		m_iFrameNumberLengthInByte(4)
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
		m_iCalleeFrameInfoSize = 14;

		memset(m_iMixedData, 0, sizeof m_iMixedData);
		memset(m_uchCalleeBlockInfo, 0, sizeof m_uchCalleeBlockInfo);
		return;
	}


	long long AudioMixer::readValue(unsigned char *uchByteArray, int &iIndexOffset, int &iBitOffset, int iReadBitLength)
	{
		long long iResult = 0;
		int iTotalBitToRead = iReadBitLength;

		int iBlockSize;
		long long iBlockMask;
		long long iBlockValue;

		while (iTotalBitToRead)
		{
			iBlockSize = (8 - iBitOffset) > iTotalBitToRead ? iTotalBitToRead : (8 - iBitOffset);
			iBlockMask = (1LL << iBlockSize) - 1;
			iBlockMask <<= iBitOffset;
			iBlockValue = (long long)uchByteArray[iIndexOffset] & iBlockMask;
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

		if (iResult & (1LL << (iReadBitLength - 1)))
		{
			iResult ^= ((1LL << iReadBitLength) - 1LL);
			iResult += 1;
			iResult *= -1;
		}
		return iResult;
	}


	long long AudioMixer::getMax(int bitLength) {
		return (1LL << (bitLength - 1)) - 1LL;
	}

	long long AudioMixer::getMin(int bitLength) {
		long long value = (1LL << (bitLength - 1)) - 1LL;
		value *= -1;
		value -= 1;
		return value;
	}

	void AudioMixer::writeValue(unsigned char *uchByteArray, int &iIndexOffset, int &iBitOffset, int iWriteBitLength, long long iValue)
	{
		int iTotalBitToWrite = iWriteBitLength;

		if (iValue > getMax(iWriteBitLength))
			iValue = getMax(iWriteBitLength);

		if (iValue < 0) {
			if (iValue < getMin(iWriteBitLength))
				iValue = getMin(iWriteBitLength);
		}

		if (iValue < 0) {
			iValue *= -1LL;
			iValue ^= ((1LL << iWriteBitLength) - 1LL);
			iValue += 1LL;
		}

		int iBlockSize;
		long long iBlockMask;
		long long iBlockValue;

		while (iTotalBitToWrite)
		{
			iBlockSize = (8 - iBitOffset) > iTotalBitToWrite ? iTotalBitToWrite : (8 - iBitOffset);
			iBlockMask = (1LL << iBlockSize) - 1;
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

	void AudioMixer::addAudioData(unsigned char* uchCalleeAudio, MuxHeader &header)
	{
		int indexOffset = 0;
		int bitOffset = 0;
		long long iCalleeId = header.getCalleeId();
		long long iMissingFlag = header.getMissingMask();
		long long FrameNo = header.getFrameNo();

		LOG18("#18@# ADD AUDIO OF CALLEE %lld WITH FRAME %lld", iCalleeId, FrameNo);
		int iOffsetForTotalCalleeAndBit = 2;

		int iAudioSamplePerBlock = m_iAudioFrameSize / m_iTotalBlock;

		//memcpy(m_uchCalleeBlockInfo + iOffsetForTotalCalleeAndBit + m_iTotalCallee * m_iCalleeFrameInfoSize, uchCalleeAudio, m_iCalleeFrameInfoSize);


		header.getMuxHeader(m_uchCalleeBlockInfo + iOffsetForTotalCalleeAndBit + m_iTotalCallee * m_iCalleeFrameInfoSize);

		m_iTotalCallee++;

		int iIndexOffset = 0;
		int iBitOffset = 0;

		for (int i = 0; i < m_iAudioFrameSize; i++)
		{
			if (!(iMissingFlag & (1LL << (i / iAudioSamplePerBlock))))
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

	int AudioMixer::removeAudioData(unsigned char* uchAudioDataToPlay, unsigned char* uchMixedAudioData, unsigned char* uchCalleeAudioData, long long calleeId, std::vector<std::pair<int, int>> &vMissingBlock)
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

		long long iMissingBlocks = -1;
		int iSamplePerBlock = m_iAudioFrameSize / m_iTotalBlock;


		/// Find Callee info block for Callee designated by calleeId
		int iIndexOffset = 2;
		int iBitOffset = 0;
		for (int i = 0; i < iTotalCallee; i++) {
			long long llCurCalleeId = readValue(uchMixedAudioData, iIndexOffset, iBitOffset, m_iCalleeIdLengthInByte * 8);
			long long llCurMissingBlocks = readValue(uchMixedAudioData, iIndexOffset, iBitOffset, m_iMissingMaskLengthInByte * 8);
			long long llCurFrameNo = readValue(uchMixedAudioData, iIndexOffset, iBitOffset, m_iFrameNumberLengthInByte * 8);

			if (llCurCalleeId == calleeId)
			{
				iMissingBlocks = llCurMissingBlocks;
				break;
			}
		}

		int nMissingIndex = 0;
		int BitPerByte = 8;
		for (int i = 0; i < m_iAudioFrameSize; i++)
		{
			long long calleeValue = readValue(uchCalleeAudioData, iIndexOffsetForCallee, iBitOffsetForCallee, 16);
			long long mixedValue = readValue(uchMixedAudioData, iIndexOffsetForMixed, iBitOffsetForMixed, iBitPerSample);

			if (!(iMissingBlocks & (1LL << (i / iSamplePerBlock))))
			{
				mixedValue -= calleeValue;
			}

			int nLeftBitPos = (i* m_iNumberOfBitsPerSample);
			int nRightBitPos = nLeftBitPos + m_iNumberOfBitsPerSample;
			int isMissing = 0;
			while (nMissingIndex < (int)vMissingBlock.size() && ((vMissingBlock[nMissingIndex].second + 1) * BitPerByte - 1) < nLeftBitPos) {
				nMissingIndex++;
			}

			if (nMissingIndex < (int)vMissingBlock.size()) {
				int iLpos = max(vMissingBlock[nMissingIndex].first * BitPerByte, nLeftBitPos);
				int iRpos = min((vMissingBlock[nMissingIndex].first + 1) * BitPerByte - 1, nRightBitPos);
				if (iLpos < iRpos) {
					isMissing = 1;
				}
			}

			if (isMissing) {
				mixedValue = 0;
			}
			writeValue(uchAudioDataToPlay, iIndexOffsetForPlayAudio, iBitOffsetForPlayAudio, 16, mixedValue);
		}

		return m_iAudioFrameSize * 2;
	}

	void AudioMixer::genCalleeChunkHeader(unsigned char* uchDestinaton, int iStartIndex, int iEndIndex, long long iCalleeId, int iFrameNumber, int iFrameSize, int iTotalBlock, std::vector<std::pair<int, int>> &vMissingBlocks)
	{
		int samplesPerBlocks = (iEndIndex - iStartIndex + 1) / iTotalBlock;

		int IndexOffset = 0;
		int BitOffset = 0;

		int iMissingFlag = 0;
		int iMissingVectorIndex = 0;

		writeValue(uchDestinaton, IndexOffset, BitOffset, m_iCalleeIdLengthInByte * 8, iCalleeId);

		for (int i = 0; i < (iEndIndex - iStartIndex + 1); i++)
		{
			while (iMissingVectorIndex < (int)vMissingBlocks.size() && vMissingBlocks[iMissingVectorIndex].second < (iStartIndex + i))
			{
				iMissingVectorIndex++;
			}

			if ((iMissingVectorIndex < (int)vMissingBlocks.size()) && ((iStartIndex + i) >= vMissingBlocks[iMissingVectorIndex].first)) {
				iMissingFlag |= (1 << (i / samplesPerBlocks));
			}
		}

		writeValue(uchDestinaton, IndexOffset, BitOffset, m_iMissingMaskLengthInByte * 8, iMissingFlag);
		writeValue(uchDestinaton, IndexOffset, BitOffset, m_iFrameNumberLengthInByte * 8, iFrameNumber);
		return;
	}

	int AudioMixer::GetAudioFrameByParsingMixHeader(unsigned char *uchByteArray, long long nUserId){
		int nNumberOfUsers = uchByteArray[0];
		int nBlock = uchByteArray[1];

		LOG18("#18@# FOUND USER %d, nBlock : %d", nNumberOfUsers, nBlock);
		int indexOffset = 2;
		int bitOffst = 0;
		for (int i = 0; i < nNumberOfUsers; i++)
		{
			long long nId = readValue(uchByteArray, indexOffset, bitOffst, m_iCalleeIdLengthInByte * 8);
			long long iMissingMask = readValue(uchByteArray, indexOffset, bitOffst, m_iMissingMaskLengthInByte * 8);
			int nFrameNumber = (int)readValue(uchByteArray, indexOffset, bitOffst, m_iFrameNumberLengthInByte * 8);
			LOG18("#18@# FOUND UID %lld nframe %d", nId, nFrameNumber);
			if (nId == nUserId)
			{
				return nFrameNumber;
			}
		}
		return -1;
	}

	void AudioMixer::Convert18BitTo16Bit(unsigned char* p18BitData, unsigned char* p16BitData, int nSampleSize)
	{
		int readIndexOffset = 0;
		int readBitOffset = 0;
		int writeIndexOffset = 0;
		int writeBitOffset = 0;
		for (int i = 0; i < nSampleSize; ++i)
		{
			writeValue(p16BitData, writeIndexOffset, writeBitOffset, 16, readValue(p18BitData, readIndexOffset, readBitOffset, 18));
		}
	}

} //namespace MediaSDK
