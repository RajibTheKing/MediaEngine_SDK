#include "AudioFarEndProcessorChannel.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioDePacketizer.h"
#include "LiveAudioDecodingQueue.h"
#include "Tools.h"


namespace MediaSDK
{

	FarEndProcessorChannel::FarEndProcessorChannel(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#farEnd# FarEndProcessorChannel::FarEndProcessorChannel()");
	}


	void FarEndProcessorChannel::ProcessFarEndData()
	{
		//	MR_DEBUG("#farEnd# FarEndProcessorChannel::ProcessFarEndData()");

		int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
		long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
		double dbTotalTime = 0; //MeaningLess

		int iDataSentInCurrentSec = 0; //NeedToFix.
		long long llTimeStamp = 0;
		int nQueueSize = m_vAudioFarEndBufferVector[0]->GetQueueSize();
		m_vFrameMissingBlocks.clear();
		MediaLog(LOG_INFO, "[FE][AFEPC] QueueSize=%d", nQueueSize);
		if (nQueueSize > 0)
		{
			m_nDecodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);
			MediaLog(LOG_INFO, "[FE][AFEPC] BeforeDecoding -> FrameSize=%d, #(FrameMissingBlocks)=%d", m_nDecodingFrameSize, (int)m_vFrameMissingBlocks.size());
			if (m_nDecodingFrameSize < 1)
			{
				MediaLog(LOG_INFO, "[FE][AFEPC] BeforeDecoding -> Removed for FrameSize<1");
				return;
			}

			/// ----------------------------------------- TEST CODE FOR CHANNEL ----------------------------------------------///

			llCapturedTime = Tools::CurrentTimestamp();

			int dummy;
			int nPacketDataLength, nChannel, nVersion;
			int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength, nEchoStateFlags;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, iPacketNumber, nPacketDataLength,
				nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength, nEchoStateFlags);

			MediaLog(LOG_INFO, "[FE][AFEPC] BeforeDecoding:ParsedData -> RelativeTime=%lld, PN=%d, PDL=%d, FrameLength=%d", llRelativeTime, iPacketNumber, nPacketDataLength, nFrameLength);
			if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
			{
				MediaLog(LOG_INFO, "[FE][AFEPC] BeforeDecoding -> Removed based on packet type & corresponding role");
				return;
			}

			bool bIsCompleteFrame = true;
			llNow = Tools::CurrentTimestamp();
			bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);
			MediaLog(LOG_INFO, "[FE][AFEPC] BeforeDecoding:AfterDePacketize -> PN=%d, PDL=%d, BN=%d, FrameLength=%d", iPacketNumber, nPacketDataLength, iBlockNumber, nFrameLength);

			if (bIsCompleteFrame){
				MediaLog(LOG_INFO, "[FE][AFEPC] Complete[P=%d B=%d]", iPacketNumber, iBlockNumber);

				m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
				if (!IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
				{
					MediaLog(LOG_INFO, "[FE][AFEPC] BeforeDecoding ->  Removed based on relative time");
					return;
				}
			}
			llNow = Tools::CurrentTimestamp();


			if (bIsCompleteFrame){
				MediaLog(LOG_INFO, "[FE][AFEPC] WORKING ON COMPLETE FRAME . ");
				m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
				MediaLog(LOG_INFO, "[FE][AFEPC] BeforeDecoding -> DecodingFrameSize=%d, PN=%d, PT=%d, PHL=%d", m_nDecodingFrameSize, iPacketNumber, nCurrentAudioPacketType, nCurrentPacketHeaderLength);
				DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);
				DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
				PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
				MediaLog(LOG_INFO, "[FE][AFEPC] AfterDecoding -> DecodedFrameSize=%d", m_nDecodedFrameSize);
				if (m_nDecodedFrameSize < 1)
				{
					MediaLog(LOG_INFO, "[FE][AFEPC] Removed for DecodedFrameSize<1");
					return;
				}
				MediaLog(LOG_INFO, "[FE][AFEPC] ChannelSendToPlayer");
				SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber, nEchoStateFlags);
				Tools::SOSleep(0);
			}
		}
		else {
			Tools::SOSleep(5);
		}
	}

} //namespace MediaSDK
