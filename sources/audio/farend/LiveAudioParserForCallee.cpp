#include "LiveAudioParserForCallee.h"
#include "LogPrinter.h"
#include "AudioPacketHeader.h"
#include "ThreadTools.h"
#include "AudioMacros.h"
#include "MediaLogger.h"
#include "AudioHeaderCall.h"

namespace MediaSDK
{

	CLiveAudioParserForCallee::CLiveAudioParserForCallee(std::vector<LiveAudioDecodingQueue*> vAudioFarEndBufferVector){
		m_vAudioFarEndBufferVector = vAudioFarEndBufferVector;
		m_pLiveReceiverMutex.reset(new CLockHandler);
		m_bIsCurrentlyParsingAudioData = false;
		m_bIsRoleChanging = false;

		m_llLastProcessedFrameNo = -1;

		m_pAudioPacketHeader = AudioPacketHeader::GetInstance(HEADER_COMMON);
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
		int validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_CALL_HEADERLENGTH);
		// add muxed header lenght with audio header length. 
		if (uchAudioData[nFrameLeftRange + iMediaByteHeaderLength] == AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED) {
			int totalCallee = uchAudioData[nFrameLeftRange + iMediaByteHeaderLength + validHeaderLength];
			validHeaderLength += (totalCallee * AUDIO_MUX_HEADER_LENGHT + 2); /**INFO FOR TOTAL CALLEE AND SAMPLE BIT SIZE LENGTH 2 BYTE**/
			LOG18("[LAPC]  TOTAL CALEE %d and VALID HEADER LENGTH %d", totalCallee, validHeaderLength);
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
			MediaLog(LOG_CODE_TRACE, "[LAPC]  LIVE ST %d ED %d", missing.first, missing.second);
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
		
		MediaLog(LOG_INFO, "[LAPC] AudioFrames = %d, MissingBlocks = %u", nNumberOfAudioFrames, nNumberOfMissingBlocks);

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
				
				MediaLog(LOG_CODE_TRACE, "[LAPC] LIVE FRAME INCOMPLETE. [%03d]", (iRightRange - iLeftRange));

				if (iLeftRange <= iRightRange)	//The frame is not complete.
				{
					MediaLog(LOG_CODE_TRACE, "[LAPC] Broken Frame %d", iFrameNumber);
					bCompleteFrame = false;					

					if (nFrameLeftRange < vMissingBlocks[iMissingIndex].first && (iLeftRange - nFrameLeftRange) >= MINIMUM_AUDIO_HEADER_SIZE)
					{
						MediaLog(LOG_CODE_TRACE, "[LAPC] VALID HEADER");
						// Get header length;
						m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize);
						validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_CALL_HEADERLENGTH);
						iCurrentFrameNumber = m_pAudioPacketHeader->GetInformation(INF_CALL_PACKETNUMBER);

						if (uchAudioData[nFrameLeftRange + iMediaByteHeaderSize] == AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED) {
							int totalCallee = uchAudioData[nFrameLeftRange + iMediaByteHeaderSize + validHeaderLength];
							validHeaderLength += (totalCallee * AUDIO_MUX_HEADER_LENGHT + 2); /**INFO FOR TOTAL CALLEE AND SAMPLE BIT SIZE LENGTH 2 BYTE**/
							MediaLog(LOG_CODE_TRACE, "[LAPC]  TOTAL CALEE %d and VALID HEADER LENGTH %d", totalCallee, validHeaderLength);
						}

						MediaLog(LOG_CODE_TRACE, "[LAPC]  LIVE FRAME CHECKED FOR VALID HEADER EXISTING DATA [%02d], VALID HEADER [%02d]", iLeftRange - nFrameLeftRange, validHeaderLength);

						if (validHeaderLength > (iLeftRange - nFrameLeftRange)) {
							MediaLog(LOG_CODE_TRACE, "[LAPC]  LIVE HEADER INCOMPLETE");
							bCompleteFrameHeader = false;
						}
					}
					else
					{
						MediaLog(LOG_CODE_TRACE, "[LAPC]  LIVE INCOMLETE FOR START INDEX IN MISSING POSITION");
						bCompleteFrameHeader = false;
					}
				}
				else
				{
					m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize);					
					iCurrentFrameNumber = m_pAudioPacketHeader->GetInformation(INF_CALL_PACKETNUMBER);
				}
			}
			else
			{				
				m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + iMediaByteHeaderSize);
				iCurrentFrameNumber = m_pAudioPacketHeader->GetInformation(INF_CALL_PACKETNUMBER);
				MediaLog(LOG_CODE_TRACE, "[LAPC] COMPLETE FRAME. FrameNo = %lld", iCurrentFrameNumber);
			}

			++iFrameNumber;
			

			if (!bCompleteFrameHeader)
			{
				numOfMissingFrames++;
				MediaLog(LOG_CODE_TRACE, "[LAPC]  Incomplete Header# FrameCounter = %d", iFrameNumber);
				continue;
			}

			nPacketType = uchAudioData[nFrameLeftRange + iMediaByteHeaderSize];
			MediaLog(LOG_CODE_TRACE, "[LAPC]  FrameCounter:%d PacketType = %d  Range[L:%d, R:%d]", iFrameNumber-1, nPacketType, nFrameLeftRange, nFrameRightRange);

			/* Discarding broken Opus frame */
			if (!bCompleteFrame && (LIVE_CALLEE_PACKET_TYPE_OPUS == nPacketType || LIVE_PUBLISHER_PACKET_TYPE_OPUS == nPacketType))
			{
				MediaLog(LOG_CODE_TRACE, "[LAPC]  Discarding Opus Packet. PT = %d", nPacketType);
				continue;
			}

			bool bIsProcessablePacketViewer = (LIVE_CALLEE_PACKET_TYPE_OPUS == nPacketType ||
				LIVE_PUBLISHER_PACKET_TYPE_OPUS == nPacketType ||
				AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED == nPacketType ||
				AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED == nPacketType);

			if (false == bIsProcessablePacketViewer)
			{
				MediaLog(LOG_CODE_TRACE, "[LAPP]  Discarding Packets# Not suitable for Callee-Viewer. PT = %d", nPacketType);
				continue;
			}

			
			///calculate missing vector 
			std::vector<std::pair<int, int> >vCurrentAudioFrameMissingBlock;

			/*Not OPUS Packet*/
			if (LIVE_CALLEE_PACKET_TYPE_OPUS != nPacketType &&  LIVE_PUBLISHER_PACKET_TYPE_OPUS != nPacketType)
			{
				MediaLog(LOG_CODE_TRACE, "[LAPC] Generating missing bloks for PCM. PT = %d", nPacketType);
				GenMissingBlock(uchAudioData, nFrameLeftRange, nFrameRightRange, vMissingBlocks, vCurrentAudioFrameMissingBlock);
			}

			nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;
			nProcessedFramsCounter++;

			MediaLog(LOG_INFO, "[LAPC] PT = %d, CompleteFrameNo = %lld, WholeFrameLength = %d[%d]", nPacketType, iCurrentFrameNumber, nCurrentFrameLenWithMediaHeader, pAudioFrameSizeInByte[iFrameNumber-1]);

			/*  Callee-Opus data */
			if (LIVE_CALLEE_PACKET_TYPE_OPUS == nPacketType)
			{
				if (m_vAudioFarEndBufferVector[1])
				{
					MediaLog(LOG_CODE_TRACE, "[LAPC] Enqueu Callee Buffer# FN = %lld",iCurrentFrameNumber); 
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
				MediaLog(LOG_DEBUG, "[LAPC] Number of Missing Frames = %lld [%lld--%lld]", iCurrentFrameNumber - m_llLastProcessedFrameNo - 1, m_llLastProcessedFrameNo + 1, iCurrentFrameNumber - 1);
			}
			m_llLastProcessedFrameNo = iCurrentFrameNumber;


			MediaLog(LOG_DEBUG, "[LAPC] Used Frame No: %lld", iCurrentFrameNumber);
		}

		MediaLog(LOG_INFO, "[LAPC] Total Frames: %d Used Frames: %d, Missing Frame: %d", nNumberOfAudioFrames, nProcessedFramsCounter, numOfMissingFrames);

		m_bIsCurrentlyParsingAudioData = false;
	}

} //namespace MediaSDK
