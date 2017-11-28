#include "LiveAudioParserForCallee.h"
#include "LogPrinter.h"
#include "AudioPacketHeader.h"
#include "ThreadTools.h"
#include "AudioMacros.h"
#include "MediaLogger.h"
#include "AudioHeaderLive.h"
#include "AudioDeviceInformation.h"
#include <vector> 

namespace MediaSDK
{

	CLiveAudioParserForCallee::CLiveAudioParserForCallee(std::vector<LiveAudioDecodingQueue*> vAudioFarEndBufferVector){
		m_vAudioFarEndBufferVector = vAudioFarEndBufferVector;
		m_pLiveReceiverMutex.reset(new CLockHandler);
		m_bIsCurrentlyParsingAudioData = false;
		m_bIsRoleChanging = false;

		m_llLastProcessedFrameNo = -1;

		m_pAudioPacketHeader = AudioPacketHeader::GetInstance(HEADER_LIVE);
		m_pAudioDeviceInformation = new AudioDeviceInformation();
	}

	CLiveAudioParserForCallee::~CLiveAudioParserForCallee(){
		SHARED_PTR_DELETE(m_pLiveReceiverMutex);
		//delete m_pAudioPacketHeader;
	}

	void CLiveAudioParserForCallee::SetRoleChanging(bool bFlah){
		m_bIsRoleChanging = bFlah;
	}

	bool CLiveAudioParserForCallee::GetRoleChanging(){
		return m_bIsRoleChanging;
	}

	bool CLiveAudioParserForCallee::IsParsingAudioData(){
		return m_bIsCurrentlyParsingAudioData;
	}

	void CLiveAudioParserForCallee::GenMissingBlock(unsigned char* uchAudioData, int nFrameLeftRange, int nFrameRightRange, std::vector<std::pair<int, int>>&vMissingBlocks, std::vector<std::pair<int, int>>&vCurrentFrameMissingBlock)
	{
		int iMediaByteHeaderLength = 1; /**Media Byte Length = 1 Byte**/
		m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + iMediaByteHeaderLength);
		int validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_LIVE_HEADERLENGTH);
		// add muxed header lenght with audio header length. 
		if (uchAudioData[nFrameLeftRange + iMediaByteHeaderLength] == AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED) {
			int totalCallee = uchAudioData[nFrameLeftRange + iMediaByteHeaderLength + validHeaderLength];
			validHeaderLength += (totalCallee * AUDIO_MUX_HEADER_LENGHT + 2); /**INFO FOR TOTAL CALLEE AND SAMPLE BIT SIZE LENGTH 2 BYTE**/
			MediaLog(LOG_CODE_TRACE, "[FE][LAPC][GMB]  TOTAL CALEE %d and VALID HEADER LENGTH %d", totalCallee, validHeaderLength);
		}
		// get audio data left range
		int nAudioDataWithoutHeaderRightRange = nFrameRightRange;
		int nAudioDataWithoutHeaderLeftRange = nFrameLeftRange + iMediaByteHeaderLength + validHeaderLength;

		for (auto &miss : vMissingBlocks) {
			int leftPos = max(nAudioDataWithoutHeaderLeftRange, miss.first);
			int rightPos = min(nAudioDataWithoutHeaderRightRange, miss.second);

			if (leftPos <= rightPos) {
				vCurrentFrameMissingBlock.push_back({ leftPos - nAudioDataWithoutHeaderLeftRange, rightPos - nAudioDataWithoutHeaderLeftRange });
			}
		}
	}


	/*
		Always, iId = 0;
	*/
	void CLiveAudioParserForCallee::ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFrameSizeInByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks){
		if (m_bIsRoleChanging)
		{
			return;
		}

		for (auto &missing : vMissingBlocks)
		{
			MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA]  LIVE ST %d ED %d", missing.first, missing.second);
			int left = max(nOffset, missing.first);
			if (left < missing.second)
			{
				memset(uchAudioData + left, 0, missing.second - left + 1);
			}
		}

		m_bIsCurrentlyParsingAudioData = true;

		LiveAudioParserForCalleeLocker lock(*m_pLiveReceiverMutex);
		SharedPointer<AudioPacketHeader> g_LiveReceiverHeader = AudioPacketHeader::GetInstance(HEADER_COMMON);
		size_t nNumberOfMissingBlocks = vMissingBlocks.size();
		size_t iMissingIndex = 0;

		bool bCompleteFrame = false;
		bool bCompleteFrameHeader = false;
		int iFrameNumber = 0, nUsedLength = 0;
		int iLeftRange, iRightRange, nFrameLeftRange, nFrameRightRange;
		int nCurrentFrameLenWithMediaHeader;
		nFrameLeftRange = nOffset;
		int numOfMissingFrames = 0;
		int nProcessedFramsCounter = 0;
		long long iCurrentFrameNumber = -1;
		int validHeaderLength;
		int nPacketType;		 
		const int iMediaByteHeaderSize = 1;
		
		MediaLog(LOG_DEBUG, "[FE][LAPC][PLA] #(AudioFrames)=%d, #(MissingBlocks)=%u", nNumberOfAudioFrames, nNumberOfMissingBlocks);
		while (iFrameNumber < nNumberOfAudioFrames)
		{
			bCompleteFrame = true;
			bCompleteFrameHeader = true;
			iCurrentFrameNumber = -1;

			nFrameLeftRange = nUsedLength + nOffset;
			nFrameRightRange = nFrameLeftRange + pAudioFrameSizeInByte[iFrameNumber] - 1;
			nUsedLength += pAudioFrameSizeInByte[iFrameNumber];			

			while (iMissingIndex < nNumberOfMissingBlocks && vMissingBlocks[iMissingIndex].second <= nFrameLeftRange)
				++iMissingIndex;

			if (iMissingIndex < nNumberOfMissingBlocks)
			{
				iLeftRange = vMissingBlocks[iMissingIndex].first;
				iRightRange = vMissingBlocks[iMissingIndex].second;

				iLeftRange = max(nFrameLeftRange, iLeftRange);
				iRightRange = min(nFrameRightRange, iRightRange);
				
				MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] frame imcomplete -> missing length = %d", (iRightRange - iLeftRange));

				if (iLeftRange <= iRightRange)	//The frame is not complete.
				{
					MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] Broken Frame %d", iFrameNumber);
					bCompleteFrame = false;					

					if (nFrameLeftRange < vMissingBlocks[iMissingIndex].first && (iLeftRange - nFrameLeftRange) >= MINIMUM_AUDIO_HEADER_SIZE)
					{
						MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] missing block did NOT damage the header");
						// Get header length;
						m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize);
						validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_LIVE_HEADERLENGTH);
						iCurrentFrameNumber = m_pAudioPacketHeader->GetInformation(INF_LIVE_PACKETNUMBER);

						if (uchAudioData[nFrameLeftRange + iMediaByteHeaderSize] == AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED) {
							int totalCallee = uchAudioData[nFrameLeftRange + iMediaByteHeaderSize + validHeaderLength];
							validHeaderLength += (totalCallee * AUDIO_MUX_HEADER_LENGHT + 2); /**INFO FOR TOTAL CALLEE AND SAMPLE BIT SIZE LENGTH 2 BYTE**/
							MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA]  TOTAL CALEE %d and VALID HEADER LENGTH %d", totalCallee, validHeaderLength);
						}

						MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] checking for undamaged header -> undamaged prefix length = %d, validHeaderLength = %d", iLeftRange - nFrameLeftRange, validHeaderLength);
						if (validHeaderLength > (iLeftRange - nFrameLeftRange)) {
							MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] header incomplete");
							bCompleteFrameHeader = false;
						}
					}
					else
					{
						MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] header incomplete 2"); //missing block is damaging the header
						bCompleteFrameHeader = false;
					}
				}
				else
				{
					m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize);					
					iCurrentFrameNumber = m_pAudioPacketHeader->GetInformation(INF_LIVE_PACKETNUMBER);
				}
			}
			else
			{				
				m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize);
				iCurrentFrameNumber = m_pAudioPacketHeader->GetInformation(INF_LIVE_PACKETNUMBER);
				MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] COMPLETE FRAME. FrameNo = %lld", iCurrentFrameNumber);
			}

			++iFrameNumber;
			MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] FrameNo = %d", iFrameNumber);

			if (!bCompleteFrameHeader)
			{
				numOfMissingFrames++;
				MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] missedFrameNo = %d, numOfMissingFrames = %d", iFrameNumber - 1, numOfMissingFrames);
				continue;
			}

			nPacketType = uchAudioData[nFrameLeftRange + iMediaByteHeaderSize];
			MediaLog(LOG_DEBUG, "[FE][LAPC][PLA]  FrameCounter:%d PacketType = %d  Range[L:%d, R:%d]", iFrameNumber-1, nPacketType, nFrameLeftRange, nFrameRightRange);

			if (nPacketType == 0)
			{
				int n_HeaderSize = m_pAudioPacketHeader->GetHeaderSize();
				MediaLog(LOG_DEBUG, "[FE][LAPE] Left: %d, Right: %d, Media Byte: %d, Header Len: %d", nFrameLeftRange, nFrameRightRange, iMediaByteHeaderSize, n_HeaderSize);
				info = m_pAudioDeviceInformation->ParseInformation(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize + n_HeaderSize, nFrameRightRange - nFrameLeftRange - iMediaByteHeaderSize - n_HeaderSize + 1);			
			}

			/* Discarding broken Opus frame */
			if (!bCompleteFrame && (LIVE_CALLEE_PACKET_TYPE_OPUS == nPacketType || LIVE_PUBLISHER_PACKET_TYPE_OPUS == nPacketType))
			{
				MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] Discarding Opus Packet. PT = %d", nPacketType);
				continue;
			}

			bool bIsProcessablePacketViewer = (LIVE_CALLEE_PACKET_TYPE_OPUS == nPacketType ||
				LIVE_PUBLISHER_PACKET_TYPE_OPUS == nPacketType ||
				AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED == nPacketType ||
				AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED == nPacketType);

			if (false == bIsProcessablePacketViewer)
			{
				MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] Discarding Packets# Not suitable for Callee-Viewer. PT = %d", nPacketType);
				continue;
			}

			
			///calculate missing vector 
			std::vector<std::pair<int, int> >vCurrentAudioFrameMissingBlock;

			/*Not OPUS Packet*/
			if (LIVE_CALLEE_PACKET_TYPE_OPUS != nPacketType &&  LIVE_PUBLISHER_PACKET_TYPE_OPUS != nPacketType)
			{
				MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] Generating missing bloks for PCM. PT = %d", nPacketType);
				GenMissingBlock(uchAudioData, nFrameLeftRange, nFrameRightRange, vMissingBlocks, vCurrentAudioFrameMissingBlock);
			}

			nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;
			nProcessedFramsCounter++;

			MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] PT = %d, CompleteFrameNo = %lld, WholeFrameLength = %d[%d]", nPacketType, iCurrentFrameNumber, nCurrentFrameLenWithMediaHeader, pAudioFrameSizeInByte[iFrameNumber-1]);

			/*  Callee-Opus data */
			if (LIVE_CALLEE_PACKET_TYPE_OPUS == nPacketType)
			{
				if (m_vAudioFarEndBufferVector[1])
				{
					MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] Enqueu Callee Buffer# FN = %lld",iCurrentFrameNumber); 
					m_vAudioFarEndBufferVector[1]->EnQueue(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize, nCurrentFrameLenWithMediaHeader - 1, vCurrentAudioFrameMissingBlock);
				}
			}
			else /* Others data except callee-opus data. */
			{
				if (m_vAudioFarEndBufferVector[0])
				{
					m_vAudioFarEndBufferVector[0]->EnQueue(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize, nCurrentFrameLenWithMediaHeader - 1, vCurrentAudioFrameMissingBlock);
				}
			}

			if (-1 < m_llLastProcessedFrameNo && m_llLastProcessedFrameNo + 1 < iCurrentFrameNumber)
			{
				MediaLog(LOG_DEBUG, "[FE][LAPC][PLA] Number of Missing Frames = %lld [%lld--%lld]", iCurrentFrameNumber - m_llLastProcessedFrameNo - 1, m_llLastProcessedFrameNo + 1, iCurrentFrameNumber - 1);
			}
			m_llLastProcessedFrameNo = iCurrentFrameNumber;


			MediaLog(LOG_DEBUG, "[FE][LAPC][PLA] Used Frame No: %lld", iCurrentFrameNumber);
		}

		MediaLog(LOG_CODE_TRACE, "[FE][LAPC][PLA] number of frames -> Total = %d Used = %d Missed = %d", nNumberOfAudioFrames, nProcessedFramsCounter, nNumberOfAudioFrames - nProcessedFramsCounter);
		m_bIsCurrentlyParsingAudioData = false;
	}

} //namespace MediaSDK
