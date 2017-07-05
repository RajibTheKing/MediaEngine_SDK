#include "AudioFarEndProcessorCall.h"

namespace MediaSDK
{


	FarEndProcessorCall::FarEndProcessorCall(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(llFriendID, nServiceType, nEntityType, pAudioCallSession, pCommonElementsBucket, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#farEnd# FarEndProcessorCall::FarEndProcessorCall()");
		m_bProcessFarendDataStarted = false;
	}


	void FarEndProcessorCall::ProcessFarEndData()
	{
		//	MR_DEBUG("#farEnd# FarEndProcessorCall::ProcessFarEndData()");

		int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
		long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
		double dbTotalTime = 0; //MeaningLess

		int iDataSentInCurrentSec = 0; //NeedToFix.
		long long llTimeStamp = 0;

		if (!IsQueueEmpty())
		{
			m_bProcessFarendDataStarted = true;
			DequeueData(m_nDecodingFrameSize);
			LOG18("#18#FE#AudioCall...");
			if (m_nDecodingFrameSize < 1)
			{
				//LOGE("##DE# CAudioCallSession::DecodingThreadProcedure queue size 0.");
				return;
			}

			llCapturedTime = Tools::CurrentTimestamp();

			int dummy;
			int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
			int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
				nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);

			HITLER("XXP@#@#MARUF FOUND DATA OF LENGTH -> [%d %d] %d frm len = %d", iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);

			if (!IsPacketTypeSupported(nCurrentAudioPacketType))
			{
				HITLER("XXP@#@#MARUF REMOVED PACKET TYPE SUPPORTED");
				return;
			}

			if (!IsPacketProcessableInNormalCall(nCurrentAudioPacketType, nVersion))
			{
				HITLER("XXP@#@#MARUF REMOVED PACKET PROCESSABLE IN NORMAL CALL");
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
			}
			llNow = Tools::CurrentTimestamp();

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
				LOG18("#18#FE#AudioCall SendToPlayer");

				SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber);
#ifndef __ANDROID__
				ProcessPlayingData();
#endif
			}
		}
		else
		{
#ifndef __ANDROID__
			Tools::SOSleep(10);
#endif
		}
#ifdef __ANDROID__
		ProcessPlayingData();	
#endif
	}

} //namespace MediaSDK
