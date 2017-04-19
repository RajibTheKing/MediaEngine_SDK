#include "MuxHeader.h"

MuxHeader::MuxHeader(long long llCalleeId, int iFrameNumber, std::vector<std::pair<int, int>> &vMissingBlocks) :
m_llCalleeId(m_llCalleeId),
m_iMissingMask(genMissingMask(vMissingBlocks)),
m_iFrameNumber(iFrameNumber)
{
	// right now static
	m_iHeaderSizeInByte = 14;
	m_iMissingMaskSizeInByte = 2;
	m_iFrameNumberSizeInByte = 4;
	m_iCalleIdSizeInByte = 8;
}

MuxHeader::~MuxHeader()
{
}

MuxHeader::MuxHeader()
{
}


long long MuxHeader::getCalleeId()
{
	return m_llCalleeId;
}
int MuxHeader::getMissingMask()
{
	return m_iMissingMask;
}
int MuxHeader::getFrameNo()
{
	return m_iFrameNumber;
}

void MuxHeader::writeValue(unsigned char *uchData,int &iIndexOffset, int ilengthInByte, long long llValue)
{
	for (int i = 0; i < ilengthInByte; i++) {
		int value = (int)(llValue & ((1LL << 8) - 1LL));
		uchData[iIndexOffset] = llValue;
		iIndexOffset++;
	}
	return;
}

int MuxHeader::getMuxHeader(unsigned char *uchData)
{
	int indexOffset = 0;
	writeValue(uchData, indexOffset, m_iCalleIdSizeInByte, m_llCalleeId);
	writeValue(uchData, indexOffset, m_iMissingMaskSizeInByte, m_iMissingMask);
	writeValue(uchData, indexOffset, m_iFrameNumberSizeInByte, m_iFrameNumber);
	return indexOffset;
}

int MuxHeader::genMissingMask(std::vector<std::pair<int, int>> &vMissingBlocks)
{
	/** 
		These things are static right now.
		assume that every frame is started at index 0 and end at index of 1599 containing a length of 1600 Byte
		and total number of blocks is 16 => total 2 Byte
	*/
	
	int iMissingVectorIndex = 0;
	int iStartIndex = 0;
	int iEndIndex = 1599;
	int iTotalBlocks = 16;

	int iMissingFlag = 0;

	int samplesPerBlocks = (iEndIndex - iStartIndex + 1) / iTotalBlocks;

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

	return iMissingFlag;
}