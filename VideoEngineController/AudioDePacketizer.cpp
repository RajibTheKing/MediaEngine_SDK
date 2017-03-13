#include "AudioDePacketizer.h"

AudioDePacketizer::AudioDePacketizer() : m_iBlockOkayFlag(0), m_iPreviousPacketNumber(-1)
{
	m_iAudioHeaderLength = m_pAudioPacketHeader.GetHeaderSize();
}

AudioDePacketizer::~AudioDePacketizer()
{
	m_iBlockOkayFlag = 0;
}

bool AudioDePacketizer::dePacketize(unsigned char* uchBlock) {
	m_pAudioPacketHeader.CopyHeaderToInformation(uchBlock);

	int iBlockNo = m_pAudioPacketHeader.GetInformation(INF_PACKET_BLOCK_NUMBER);
	int iTotalBlock = m_pAudioPacketHeader.GetInformation(INF_TOTAL_PACKET_BLOCKS);
	int iPacketNumber = m_pAudioPacketHeader.GetInformation(INF_PACKETNUMBER);
	int iBlockOffset = m_pAudioPacketHeader.GetInformation(INF_BLOCK_OFFSET);
	int iBlockLength = m_pAudioPacketHeader.GetInformation(INF_BLOCK_LENGTH);

	if (m_iPreviousPacketNumber == -1) {
		m_iPreviousPacketNumber = iPacketNumber;
		m_iBlockOkayFlag |= (1 << iBlockNo);
		memcpy(m_uchAudioStorageBuffer + iBlockOffset, uchBlock + m_iAudioHeaderLength, iBlockLength);
		if ((1 << iTotalBlock) == (m_iBlockOkayFlag + 1)) return true;
	}
	else {
		if (m_iPreviousPacketNumber == iPacketNumber) {
			m_iBlockOkayFlag |= (1 << iBlockNo);
			memcpy(m_uchAudioStorageBuffer + iBlockOffset, uchBlock + m_iAudioHeaderLength, iBlockLength);
			if ((1 << iTotalBlock) == (m_iBlockOkayFlag + 1)) return true;
		}
		else if(m_iPreviousPacketNumber < iPacketNumber){
			m_iBlockOkayFlag = 0;
			m_iPreviousPacketNumber = iPacketNumber;
			m_iBlockOkayFlag |= (1 << iBlockNo);
			memcpy(m_uchAudioStorageBuffer + iBlockOffset, uchBlock + m_iAudioHeaderLength, iBlockLength);
		}
		return false;
	}
}