#include "AudioPacketizer.h"

#include "AudioCallSession.h"
#include "AudioPacketHeader.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "CommonElementsBucket.h"
#include "Size.h"


AudioPacketizer::AudioPacketizer(CAudioCallSession* audioCallSession, CCommonElementsBucket* pCommonElementsBucket): 
m_pAudioCallSession(audioCallSession),
m_pCommonElementsBucket(pCommonElementsBucket)
{	
	m_nHeaderLength = m_AudioPacketHeader.GetHeaderSize();
	m_nHeaderLengthWithMediaByte = m_nHeaderLength + 1;
	m_nMaxDataSyzeInEachBlock = MAX_AUDIO_PACKET_SIZE - m_nHeaderLengthWithMediaByte;
}

AudioPacketizer::~AudioPacketizer()
{	
}

void AudioPacketizer::Packetize(bool bShouldPacketize, unsigned char* uchData, int nDataLength, int nFrameNumber, int packetType, int networkType, int version, long long llRelativeTime, int channel,
	int iPrevRecvdSlotID, int nReceivedPacketsInPrevSlot, long long llFriendID) {
	
	int nNumberOfBlocks = (nDataLength + m_nMaxDataSyzeInEachBlock - 1) / m_nMaxDataSyzeInEachBlock;
	int nBlockOffset = 0;	
	int nMediaByteSize = 1, nCurrentBlockLength;

	int iSlotID = nFrameNumber / AUDIO_SLOT_SIZE;
	iSlotID %= m_AudioPacketHeader.GetFieldCapacity(INF_SLOTNUMBER);

	for (int iBlockNumber = 0; iBlockNumber < nNumberOfBlocks; iBlockNumber++ ) {

		nCurrentBlockLength = min(m_nMaxDataSyzeInEachBlock, nDataLength - nBlockOffset);
		/*(unsigned char* header, int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
			int numPacketRecv, int channel, int version, long long timestamp, int iBlockNumber, int nTotalBlocksInThisFrame, int nBlockOffset);*/
		m_AudioPacketHeader.SetHeaderAllInByteArray(m_uchAudioBlock + nMediaByteSize, packetType, m_nHeaderLength, networkType, iSlotID, nFrameNumber,
			nCurrentBlockLength, iPrevRecvdSlotID, nReceivedPacketsInPrevSlot, channel, version, llRelativeTime, iBlockNumber, nNumberOfBlocks, nBlockOffset, nDataLength);
			
		memcpy(m_uchAudioBlock + m_nHeaderLengthWithMediaByte, uchData + nBlockOffset, nCurrentBlockLength);
		m_uchAudioBlock[0] = AUDIO_PACKET_MEDIA_TYPE;

		nBlockOffset += nCurrentBlockLength;
		m_pCommonElementsBucket->SendFunctionPointer(llFriendID, MEDIA_TYPE_LIVE_CALL_AUDIO, m_uchAudioBlock, nCurrentBlockLength + m_nHeaderLengthWithMediaByte, 0);
	}
}