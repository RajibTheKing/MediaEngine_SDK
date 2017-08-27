#include "AudioFarEndProcessorPublisher.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioDePacketizer.h"
#include "LiveAudioDecodingQueue.h"
#include "Tools.h"
#include "AudioDecoderBuffer.h"
#include "AudioDecoderInterface.h"
#include "MediaLogger.h"
#include "AudioPacketHeader.h"

namespace MediaSDK
{

	FarEndProcessorPublisher::FarEndProcessorPublisher(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#farEnd# FarEndProcessorPublisher::FarEndProcessorPublisher()");
	}


	void FarEndProcessorPublisher::ProcessFarEndData()
	{		
		int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
		long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
		double dbTotalTime = 0; //MeaningLess

		int iDataSentInCurrentSec = 0; //NeedToFix.
		long long llTimeStamp = 0;
		int nQueueSize = m_vAudioFarEndBufferVector[0]->GetQueueSize();
		
		m_vFrameMissingBlocks.clear();

		if (nQueueSize > 0)
		{
			const int nFarEndPacketSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);

			m_nDecodingFrameSize = nFarEndPacketSize - 1;

			
			if (m_nDecodingFrameSize < 1)
			{
				MediaLog(LOG_WARNING, "[AFEPP] m_nDecodingFrameSize = %d\n", m_nDecodingFrameSize);
				return;
			}

			/// ----------------------------------------- TEST CODE FOR PUBLISHER IN CALL ----------------------------------------------///

			llCapturedTime = Tools::CurrentTimestamp();

			int dummy;
			int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
			int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
				nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame + 1, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);

			MediaLog(LOG_CODE_TRACE, "[AFEPP] FOUND DATA OF LENGTH -> [%d %d] %d frm len = %d", iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);

			if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
			{
				MediaLog(LOG_CODE_TRACE, "[AFEPP] XXP@#@#MARUF REMOVED IN BASED ON PACKET PROCESSABLE ON ROLE");
				return;
			}

			bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
			llNow = Tools::CurrentTimestamp();
			bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + 1 + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);

			if (m_pAudioCallSession->IsOpusEnable())
			{
				MediaLog(LOG_CODE_TRACE, "[AFEPP] PushOpus Farend: %d\n", nFarEndPacketSize);
				m_pAudioCallSession->m_FarEndBufferOpus->EnQueue(m_ucaDecodingFrame, nFarEndPacketSize);
			}

			MediaLog(LOG_CODE_TRACE, "[AFEPP] iPacketNumber=%d, iBlockNumber = %d", iPacketNumber, iBlockNumber);

			if (bIsCompleteFrame){
				//m_ucaDecodingFrame
				MediaLog(LOG_CODE_TRACE, "[AFEPP] Complete[%d %d]", iPacketNumber, iBlockNumber);

				m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + 1 + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
				if (!IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
				{
					MediaLog(LOG_WARNING, "[AFEPP] REMOVED ON RELATIVE TIME");
					return;
				}
			}
			llNow = Tools::CurrentTimestamp();

			SetSlotStatesAndDecideToChangeBitRate(nSlotNumber);

			MediaLog(LOG_CODE_TRACE, "[AFEPP] nCurrentAudioPacketType = %d", nCurrentAudioPacketType);

			if (bIsCompleteFrame){


				if (LIVE_CALLEE_PACKET_TYPE_OPUS == nCurrentAudioPacketType)	/*Decoding for OPUS*/
				{
					int nEncodedDataSize = nFarEndPacketSize - 1 - nCurrentPacketHeaderLength;
					m_nDecodedFrameSize = m_pAudioCallSession->GetAudioDecoder()->DecodeAudio(m_ucaDecodingFrame + nCurrentPacketHeaderLength + 1, nEncodedDataSize, m_saDecodedFrame);

					MediaLog(LOG_CODE_TRACE, "[AFEPP] OPUS# EncodedSize = %d, DecodedSize = %d HeaderLen = %d", nEncodedDataSize, m_nDecodedFrameSize, nCurrentPacketHeaderLength);
				}
				else{	/*PCM*/
					m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
					DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);

					MediaLog(LOG_CODE_TRACE, "[AFEPP] PCM# m_nDecodingFrameSize = %d", m_nDecodingFrameSize);
				}
				
				DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
				PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
				
				if (m_nDecodedFrameSize < 1)
				{
					MediaLog(LOG_INFO, "[AFEPP]  REMOVED FOR LOW SIZE.");
					return;
				}

				MediaLog(LOG_INFO, "[AFEPP] Publisher# SendToPlayer, FN = %d", iPacketNumber);

				SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber);
				Tools::SOSleep(0);
			}
		}
		else 
		{
			Tools::SOSleep(10);
		}
#ifdef USE_AECM
		if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER)
		{
			ProcessPlayingData();
		}
#endif

	}

} //namespace MediaSDK
