#include "LiveAudioParserForPublisher.h"
#include "LogPrinter.h"
#include "AudioPacketHeader.h"
#include "ThreadTools.h"
#include "AudioMacros.h"
#include "CommonTypes.h"
#include "MediaLogger.h"
#include "AudioHeaderLive.h"
#include "AudioCallSession.h"

namespace MediaSDK
{

	CLiveAudioParserForPublisher::CLiveAudioParserForPublisher(std::vector<LiveAudioDecodingQueue*> vAudioFarEndBufferVector, DeviceInformationInterface *pDeviceInfoInterface){
		m_vAudioFarEndBufferVector = vAudioFarEndBufferVector;
		m_pLiveReceiverMutex.reset(new CLockHandler);
		m_bIsCurrentlyParsingAudioData = false;
		m_bIsRoleChanging = false;

		m_pAudioPacketHeader = AudioPacketHeader::GetInstance(HEADER_LIVE);
		m_pDeviceInfoInterface = pDeviceInfoInterface;
	}

	CLiveAudioParserForPublisher::~CLiveAudioParserForPublisher(){
		SHARED_PTR_DELETE(m_pLiveReceiverMutex);
	}

	void CLiveAudioParserForPublisher::SetRoleChanging(bool bFlah){
		m_bIsRoleChanging = bFlah;
	}

	bool CLiveAudioParserForPublisher::GetRoleChanging(){
		return m_bIsRoleChanging;
	}

	bool CLiveAudioParserForPublisher::IsParsingAudioData(){
		return m_bIsCurrentlyParsingAudioData;
	}

	void CLiveAudioParserForPublisher::GenMissingBlock(unsigned char* uchAudioData, int nFrameLeftRange, int nFrameRightRange, std::vector<std::pair<int, int>>&vMissingBlocks, std::vector<std::pair<int, int>>&vCurrentFrameMissingBlock)
	{
		int mediaByteSize = 1;
		m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + mediaByteSize);
		int validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_LIVE_HEADERLENGTH);
		// add muxed header lenght with audio header length. 

		if (uchAudioData[nFrameLeftRange + mediaByteSize] == AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED) {
			int totalCallee = uchAudioData[nFrameLeftRange + validHeaderLength + mediaByteSize];
			validHeaderLength += (totalCallee * AUDIO_MUX_HEADER_LENGHT + 2);
		}
		// get audio data left range
		int nAudioDataWithoutHeaderRightRange = nFrameRightRange;
		int nAudioDataWithoutHeaderLeftRange = nFrameLeftRange + validHeaderLength + mediaByteSize;

		for (auto &miss : vMissingBlocks) {
			int leftPos = max(nAudioDataWithoutHeaderLeftRange, miss.first);
			int rightPos = min(nAudioDataWithoutHeaderRightRange, miss.second);

			if (leftPos <= rightPos) {
				vCurrentFrameMissingBlock.push_back({ leftPos - nAudioDataWithoutHeaderLeftRange, rightPos - nAudioDataWithoutHeaderLeftRange });
			}
		}
	}


	void CLiveAudioParserForPublisher::ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFrameSizeInByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks){
		if (m_bIsRoleChanging)
		{
			return;
		}

		for (auto &missing : vMissingBlocks)
		{
			MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] missing block -> [ST-%d:ED-%d]", missing.first, missing.second);
			int left = max(nOffset, missing.first);
			if (left < missing.second)
			{
				memset(uchAudioData + left, 0, missing.second - left + 1);
			}
		}

		m_bIsCurrentlyParsingAudioData = true;

		LiveAudioParserForPublisherLocker lock(*m_pLiveReceiverMutex);
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
		int nPacketType;

		int mediaByteSize = 1;
		
		MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] #(AudioFrames)=%d, #(MissingBlocks)=%u",nNumberOfAudioFrames, nNumberOfMissingBlocks);
		while (iFrameNumber < nNumberOfAudioFrames)
		{
			bCompleteFrame = true;
			bCompleteFrameHeader = true;

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
				
				if (iLeftRange <= iRightRange)	//The frame is not complete.
				{
					bCompleteFrame = false;

					MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] frame imcomplete -> missing length = %d", (iRightRange - iLeftRange));

					if (nFrameLeftRange < vMissingBlocks[iMissingIndex].first && (iLeftRange - nFrameLeftRange) >= MINIMUM_AUDIO_HEADER_SIZE)//missing block is only within data part not damaging header
					{
						MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] missing block did NOT damage the header");
						m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + mediaByteSize);
						int validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_LIVE_HEADERLENGTH);						

						if (uchAudioData[nFrameLeftRange + mediaByteSize] == AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED) {
							int totalCallee = uchAudioData[nFrameLeftRange + validHeaderLength + mediaByteSize];
							validHeaderLength += (totalCallee * AUDIO_MUX_HEADER_LENGHT + 2);
						}

						MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] checking for undamaged header -> undamaged prefix length = %d, validHeaderLength = %d", iLeftRange - nFrameLeftRange, validHeaderLength);
						if (validHeaderLength > (iLeftRange - nFrameLeftRange)) {
							MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] header incomplete");
							bCompleteFrameHeader = false;
						}
					}
					else
					{
						MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] header incomplete 2"); //missing block is damaging the header
						bCompleteFrameHeader = false;
					}
				}
			}

			++iFrameNumber;
			MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] FrameNo = %d", iFrameNumber);

			if (!bCompleteFrameHeader)
			{				
				numOfMissingFrames++;
				MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] missedFrameNo = %d, numOfMissingFrames = %d", iFrameNumber - 1, numOfMissingFrames);
				continue;
			}

			nPacketType = uchAudioData[nFrameLeftRange + mediaByteSize];
			MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA]  PacketType = %d", nPacketType);

			if (nPacketType == SESSION_STATISTICS_PACKET_TYPE)
			{
				int n_HeaderSize = m_pAudioPacketHeader->GetHeaderSize();
				MediaLog(LOG_DEBUG, "[FE][LAPE] Left: %d, Right: %d, Media Byte: %d, Header Len: %d", nFrameLeftRange, nFrameRightRange, mediaByteSize, n_HeaderSize);
				//m_pDeviceInfoInterface->GenerateReport(uchAudioData + nFrameLeftRange + mediaByteSize + n_HeaderSize, nFrameRightRange - nFrameLeftRange - mediaByteSize - n_HeaderSize + 1);
				
				m_pDeviceInfoInterface->SetDeviceInformationOfViewerInCall(uchAudioData + nFrameLeftRange + mediaByteSize + n_HeaderSize, nFrameRightRange - nFrameLeftRange - mediaByteSize - n_HeaderSize + 1);
			}

			/* Discarding broken Opus frame */
			
			if (!bCompleteFrame && LIVE_CALLEE_PACKET_TYPE_OPUS == nPacketType)
			{
				MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] Discarding Opus Packet. PT = %d", nPacketType);
				continue;
			}

			bool bIsProcessablePacket = (LIVE_CALLEE_PACKET_TYPE_OPUS == nPacketType ||
				AUDIO_LIVE_CALLEE_PACKET_TYPE == nPacketType);

			if (false == bIsProcessablePacket)
			{
				MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] Discarding Packets# Not suitable for publisher. PT = %d", nPacketType);
				continue;
			}
						
			MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] CompleteFrameNo = %lld", m_pAudioPacketHeader->GetInformation(INF_LIVE_PACKETNUMBER));
			///calculate missing vector 
			std::vector<std::pair<int, int> >vCurrentAudioFrameMissingBlock;
			GenMissingBlock(uchAudioData, nFrameLeftRange, nFrameRightRange, vMissingBlocks, vCurrentAudioFrameMissingBlock);

			nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;
			nProcessedFramsCounter++;
			if (m_vAudioFarEndBufferVector[iId])
			{
				m_vAudioFarEndBufferVector[iId]->EnQueue(uchAudioData + nFrameLeftRange, nCurrentFrameLenWithMediaHeader , vCurrentAudioFrameMissingBlock);
			}
		}

		MediaLog(LOG_CODE_TRACE, "[FE][LAPP][PLA] number of frames -> Total = %d Used = %d Missed = %d", nNumberOfAudioFrames, nProcessedFramsCounter, nNumberOfAudioFrames - nProcessedFramsCounter);
		m_bIsCurrentlyParsingAudioData = false;		
	}

} //namespace MediaSDK
 