#include "AudioPacketizer.h"
#include "LogPrinter.h"
#include "Tools.h"

AudioPacketizer::AudioPacketizer()
{
	m_AudioPacketHeader = AudioPacketHeader::GetInstance(HEADER_COMMON);
	m_nHeaderLength = m_AudioPacketHeader->GetHeaderSize();

	m_nHeaderLengthWithMediaByte = m_nHeaderLength + 1;
	m_nMaxDataSyzeInEachBlock = MAX_AUDIO_PACKET_SIZE - m_nHeaderLengthWithMediaByte;
}

AudioPacketizer::~AudioPacketizer()
{
	//delete m_AudioPacketHeader;
}

void AudioPacketizer::Packetize(unsigned char* uchData, const AudioHeaderFields& headerParams, OnDataReadyCallback callback)
{

	int nNumberOfBlocks = ( headerParams.frameLength /*nDataLength*/ + m_nMaxDataSyzeInEachBlock - 1) / m_nMaxDataSyzeInEachBlock;
	int nBlockOffset = 0;	
	int nMediaByteSize = 1, nCurrentBlockLength;

	int iSlotID = headerParams.packetNumber /*nFrameNumber*/ / AUDIO_SLOT_SIZE;
	
	iSlotID %= m_AudioPacketHeader->GetFieldCapacity(INF_SLOTNUMBER);
	LOGT("##NF###XXP@#@#MARUF INIT PACKETING .... data Len = %d numBlock %d, SlotId %d", headerParams.frameLength /*nDataLength*/, nNumberOfBlocks, iSlotID);
	for (int iBlockNumber = 0; iBlockNumber < nNumberOfBlocks; iBlockNumber++ ) {

		nCurrentBlockLength = min(m_nMaxDataSyzeInEachBlock, headerParams.frameLength /*nDataLength*/ - nBlockOffset);
		/*(unsigned char* header, int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
			int numPacketRecv, int channel, int version, long long timestamp, int iBlockNumber, int nTotalBlocksInThisFrame, int nBlockOffset);

		m_AudioPacketHeader->SetHeaderAllInByteArray(m_uchAudioBlock + nMediaByteSize, packetType, m_nHeaderLength, networkType, iSlotID, nFrameNumber,
			nCurrentBlockLength, iPrevRecvdSlotID, nReceivedPacketsInPrevSlot, channel, version, llRelativeTime, iBlockNumber, nNumberOfBlocks, nBlockOffset, nDataLength); //*/

		m_AudioPacketHeader->SetHeaderAllInByteArray(m_uchAudioBlock + nMediaByteSize, headerParams);
			
		memcpy(m_uchAudioBlock + m_nHeaderLengthWithMediaByte, uchData + nBlockOffset, nCurrentBlockLength);
		m_uchAudioBlock[0] = AUDIO_PACKET_MEDIA_TYPE;

		nBlockOffset += nCurrentBlockLength;
		HITLER("XXP@#@#MARUF PACKETING .... %d", iBlockNumber);

		callback(m_uchAudioBlock, nCurrentBlockLength + m_nHeaderLengthWithMediaByte);

		Tools::SOSleep(3);
		HITLER("XXP@#@#MARUF PACKETING SENT.... %d", iBlockNumber);
	}

	HITLER("XXP@#@#MARUF PACKETING ENDS.");
}

