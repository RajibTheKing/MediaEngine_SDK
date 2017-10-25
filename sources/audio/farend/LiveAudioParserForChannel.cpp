#include "LiveAudioParserForChannel.h"
#include "LogPrinter.h"
#include "MediaLogger.h"
#include "AudioPacketHeader.h"
#include "ThreadTools.h"
#include "AudioMacros.h"
#include "AudioHeaderCall.h"

namespace MediaSDK
{

	CLiveAudioParserForChannel::CLiveAudioParserForChannel(std::vector<LiveAudioDecodingQueue*> vAudioFarEndBufferVector){
		m_vAudioFarEndBufferVector = vAudioFarEndBufferVector;
		m_pLiveReceiverMutex.reset(new CLockHandler);
		m_bIsCurrentlyParsingAudioData = false;
		m_bIsRoleChanging = false;

		m_pAudioPacketHeader = AudioPacketHeader::GetInstance(HEADER_COMMON);
	}

	CLiveAudioParserForChannel::~CLiveAudioParserForChannel(){
		SHARED_PTR_DELETE(m_pLiveReceiverMutex);
		//delete m_pAudioPacketHeader;
	}

	void CLiveAudioParserForChannel::SetRoleChanging(bool bFlah){
		m_bIsRoleChanging = bFlah;
	}

	bool CLiveAudioParserForChannel::GetRoleChanging(){
		return m_bIsRoleChanging;
	}

	bool CLiveAudioParserForChannel::IsParsingAudioData(){
		return m_bIsCurrentlyParsingAudioData;
	}

	void CLiveAudioParserForChannel::GenMissingBlock(unsigned char* uchAudioData, int nFrameLeftRange, int nFrameRightRange, std::vector<std::pair<int, int>>&vMissingBlocks, std::vector<std::pair<int, int>>&vCurrentFrameMissingBlock)
	{
		m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + 1);
		int validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_CALL_HEADERLENGTH);
		// add muxed header lenght with audio header length. 
		if (uchAudioData[nFrameLeftRange + 1] == AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED) {
			int totalCallee = uchAudioData[nFrameLeftRange + validHeaderLength + 1];
			validHeaderLength += (totalCallee * AUDIO_MUX_HEADER_LENGHT + 2);
		}
		// get audio data left range
		int nAudioDataWithoutHeaderRightRange = nFrameRightRange;
		int nAudioDataWithoutHeaderLeftRange = nFrameLeftRange + validHeaderLength + 1;

		for (auto &miss : vMissingBlocks) {
			int leftPos = max(nAudioDataWithoutHeaderLeftRange, miss.first);
			int rightPos = min(nAudioDataWithoutHeaderRightRange, miss.second);

			if (leftPos <= rightPos) {
				vCurrentFrameMissingBlock.push_back({ leftPos - nAudioDataWithoutHeaderLeftRange, rightPos - nAudioDataWithoutHeaderLeftRange });
			}
		}
	}

	void CLiveAudioParserForChannel::ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFrameSizeInByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks){
		if (m_bIsRoleChanging)
		{
			return;
		}

		for (auto &missing : vMissingBlocks)
		{
			MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] missing block -> [ST-%d:ED-%d]", missing.first, missing.second);
			int left = max(nOffset, missing.first);
			if (left < missing.second)
			{
				memset(uchAudioData + left, 0, missing.second - left + 1);
			}
		}

		m_bIsCurrentlyParsingAudioData = true;

		LiveAudioParserForChannelLocker lock(*m_pLiveReceiverMutex);
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

		MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] #(AudioFrames)=%d, #(MissingBlocks)=%u", nNumberOfAudioFrames, nNumberOfMissingBlocks);
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
					MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] frame imcomplete -> missing length = %d", (iRightRange - iLeftRange));
					if (nFrameLeftRange < vMissingBlocks[iMissingIndex].first && (iLeftRange - nFrameLeftRange) >= MINIMUM_AUDIO_HEADER_SIZE) //missing block is only within data part not damaging header
					{
						MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] missing block did NOT damage the header");
						m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + 1);
						int validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_CALL_HEADERLENGTH);

						MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] checking for undamaged header -> undamaged prefix length = %d, validHeaderLength = %d", iLeftRange - nFrameLeftRange, validHeaderLength);
						if (validHeaderLength > (iLeftRange - nFrameLeftRange)) {
							MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] header incomplete");
							bCompleteFrameHeader = false;
						}
					}
					else
					{
						MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] header incomplete 2"); //missing block is damaging the header
						bCompleteFrameHeader = false;
					}
				}
			}

			++iFrameNumber;
			MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] FrameNo = %d", iFrameNumber);

			if (!bCompleteFrame)
			{
				numOfMissingFrames++;
				MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] missedFrameNo = %d, numOfMissingFrames = %d", iFrameNumber - 1, numOfMissingFrames);
				continue;
			}

			nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;
			nProcessedFramsCounter++;
			std::vector<std::pair<int, int>>vMissingFrame;
			if (m_vAudioFarEndBufferVector[iId])
			{
				m_vAudioFarEndBufferVector[iId]->EnQueue(uchAudioData + nFrameLeftRange + 1, nCurrentFrameLenWithMediaHeader - 1, vMissingFrame);
			}
		}

		MediaLog(LOG_CODE_TRACE, "[FE][LAPCh][PLA] number of frames -> Total = %d Used = %d Missed = %d", nNumberOfAudioFrames, nProcessedFramsCounter, nNumberOfAudioFrames - nProcessedFramsCounter);
		m_bIsCurrentlyParsingAudioData = false;
	}

} //namespace MediaSDK