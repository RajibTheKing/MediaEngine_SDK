#include "AudioFarEndProcessorPublisher.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioDePacketizer.h"
#include "LiveAudioDecodingQueue.h"
#include "Tools.h"


namespace MediaSDK
{

	FarEndProcessorPublisher::FarEndProcessorPublisher(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#farEnd# FarEndProcessorPublisher::FarEndProcessorPublisher()");
	}


	void FarEndProcessorPublisher::ProcessFarEndData()
	{
		//	MR_DEBUG("#farEnd# FarEndProcessorPublisher::ProcessFarEndData()");

		int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
		long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
		double dbTotalTime = 0; //MeaningLess

		int iDataSentInCurrentSec = 0; //NeedToFix.
		long long llTimeStamp = 0;
		int nQueueSize = m_vAudioFarEndBufferVector[0]->GetQueueSize();
		m_vFrameMissingBlocks.clear();
		if (nQueueSize > 0)
		{
			m_nDecodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);

			LOG18("#18#FE#Publisher..");
			if (m_nDecodingFrameSize < 1)
			{
				//LOGE("##DE# CAudioCallSession::DecodingThreadProcedure queue size 0.");
				return;
			}

			/// ----------------------------------------- TEST CODE FOR PUBLISHER IN CALL ----------------------------------------------///

			llCapturedTime = Tools::CurrentTimestamp();

			int dummy;
			int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
			int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
				nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);

			HITLER("XXP@#@#MARUF FOUND DATA OF LENGTH -> [%d %d] %d frm len = %d", iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);
			if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
			{
				HITLER("XXP@#@#MARUF REMOVED IN BASED ON PACKET PROCESSABLE ON ROLE");
				return;
			}

			bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
			llNow = Tools::CurrentTimestamp();
			bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);
			HITLER("XXP@#@#MARUF [%d %d]", iPacketNumber, iBlockNumber);
			if (bIsCompleteFrame){
				//m_ucaDecodingFrame
				HITLER("XXP@#@#MARUF Complete[%d %d]", iPacketNumber, iBlockNumber);

				m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
				if (!IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
				{
					HITLER("XXP@#@#MARUF REMOVED ON RELATIVE TIME");
					return;
				}
			}
			llNow = Tools::CurrentTimestamp();

			SetSlotStatesAndDecideToChangeBitRate(nSlotNumber);

			if (bIsCompleteFrame){
				HITLER("XXP@#@#MARUF WORKING ON COMPLETE FRAME . ");
				m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
				HITLER("XXP@#@#MARUF  -> HEHE %d %d", m_nDecodingFrameSize, nCurrentPacketHeaderLength);
				DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);
				DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
				PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
				HITLER("XXP@#@#MARUF AFTER POST PROCESS ... deoding frame size %d", m_nDecodedFrameSize);
				if (m_nDecodedFrameSize < 1)
				{
					HITLER("XXP@#@#MARUF REMOVED FOR LOW SIZE.");
					return;
				}
				LOG18("#18#FE#Publisher SendToPlayer");
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
