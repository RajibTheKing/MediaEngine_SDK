#ifndef _AUDIO_MUX_HEADER_H_
#define _AUDIO_MUX_HEADER_H_

#include <vector>

class MuxHeader
{
public:
	MuxHeader(long long llCalleeId, int iFrameNumber, std::vector<std::pair<int, int>>&vMissingBlocks);
	MuxHeader();
	~MuxHeader();

	int getMuxHeader(unsigned char *uchData);
	void writeValue(unsigned char *uchData, int &iIndexOffset, int ilengthInByte, long long llValue);
	int genMissingMask(std::vector<std::pair<int, int>> &vMissingBlocks);
	long long getCalleeId();
	int getMissingMask();
	int getFrameNo();
private:
	int m_iHeaderSizeInByte;
	int m_iMissingMaskSizeInByte;
	int m_iFrameNumberSizeInByte;
	int m_iCalleIdSizeInByte;

	long long m_llCalleeId;
	int m_iMissingMask;
	int m_iFrameNumber;
};

#endif
