#include "AudioFarEndProcessorCall.h"
#include "Tools.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioDePacketizer.h"
#include "LiveAudioDecodingQueue.h"
#include "AudioFarEndDataProcessor.h"
#include "AudioDecoderBuffer.h"
#include "AudioPacketHeader.h"
#include "AudioTypes.h"


namespace MediaSDK
{
	FarEndProcessorCall::FarEndProcessorCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#farEnd# FarEndProcessorCall::FarEndProcessorCall()");
		m_bProcessFarendDataStarted = false;
	}


	void FarEndProcessorCall::ProcessFarEndData()
	{
		int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
		long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
		double dbTotalTime = 0; //MeaningLess

		int iDataSentInCurrentSec = 0; //NeedToFix.
		long long llTimeStamp = 0;

		if (!IsQueueEmpty())
		{
			m_bProcessFarendDataStarted = true;
			DequeueData(m_nDecodingFrameSize);
			
			if (m_nDecodingFrameSize < 1)
			{
				MediaLog(CODE_TRACE, "[FE] Too small data");
				return;
			}

			llCapturedTime = Tools::CurrentTimestamp();

			int dummy;
			int nPacketDataLength, nChannel, nVersion;
			int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength, nEchoStateFlags;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, iPacketNumber, nPacketDataLength,
				nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength, nEchoStateFlags);

			MediaLog(LOG_DEBUG, "[ECHOFLAG] playerside nEchoStateFlags = %d\n", nEchoStateFlags);

			MediaLog(CODE_TRACE, "XXP@#@#MARUF FOUND DATA OF LENGTH -> [%d %d] %d frm len = %d", iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);

			if (!IsPacketTypeSupported(nCurrentAudioPacketType))
			{
				MediaLog(CODE_TRACE, "XXP@#@#MARUF REMOVED PACKET TYPE SUPPORTED");
				return;
			}

			if (!IsPacketProcessableInNormalCall(nCurrentAudioPacketType, nVersion))
			{
				MediaLog(CODE_TRACE, "XXP@#@#MARUF REMOVED PACKET PROCESSABLE IN NORMAL CALL");
				return;
			}

			bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
			llNow = Tools::CurrentTimestamp();
			bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);
			MediaLog(CODE_TRACE, "XXP@#@#MARUF [%d %d]", iPacketNumber, iBlockNumber);
			if (bIsCompleteFrame){
				//m_ucaDecodingFrame
				MediaLog(CODE_TRACE, "XXP@#@#MARUF Complete[%d %d]", iPacketNumber, iBlockNumber);

				m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
			}
			llNow = Tools::CurrentTimestamp();

			if (bIsCompleteFrame){
				MediaLog(CODE_TRACE, "XXP@#@#MARUF WORKING ON COMPLETE FRAME . ");
				m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
				MediaLog(CODE_TRACE, "XXP@#@#MARUF  -> HEHE %d %d", m_nDecodingFrameSize, nCurrentPacketHeaderLength);
				DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);
				DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
				PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
				MediaLog(CODE_TRACE, "XXP@#@#MARUF AFTER POST PROCESS ... deoding frame size %d", m_nDecodedFrameSize);
				if (m_nDecodedFrameSize < 1)
				{
					MediaLog(CODE_TRACE, "XXP@#@#MARUF REMOVED FOR LOW SIZE.");
					return;
				}
				MediaLog(CODE_TRACE, "FE#AudioCall SendToPlayer");

				SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber, nEchoStateFlags);
#ifndef USE_AECM
				ProcessPlayingData();
#endif
			}
		}
		else
		{
#ifndef USE_AECM
			Tools::SOSleep(10);
#endif
		}
#ifdef USE_AECM
		ProcessPlayingData();	
#endif
	}


	void FarEndProcessorCall::DequeueData(int &decodingFrameSize)
	{
		if (m_bIsLiveStreamingRunning)
		{
#ifndef LOCAL_SERVER_LIVE_CALL
			if (m_nEntityType != ENTITY_TYPE_PUBLISHER_CALLER)
			{
				decodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);
			}
			else
			{
				decodingFrameSize = m_AudioReceivedBuffer->DeQueue(m_ucaDecodingFrame);
			}
#else
			if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER || m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
			{
				decodingFrameSize = m_AudioReceivedBuffer->DeQueue(m_ucaDecodingFrame);
			}
			else
			{
				decodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame);
			}
#endif
		}
		else
		{
			decodingFrameSize = m_AudioReceivedBuffer->DeQueue(m_ucaDecodingFrame);
		}
	}


	bool FarEndProcessorCall::IsPacketProcessableInNormalCall(int &nCurrentAudioPacketType, int &nVersion)
	{
		if (false == m_bIsLiveStreamingRunning)
		{
			if (AUDIO_SKIP_PACKET_TYPE == nCurrentAudioPacketType)
			{				
				Tools::SOSleep(0);
				return false;
			}
			else if (AUDIO_NOVIDEO_PACKET_TYPE == nCurrentAudioPacketType)
			{
				if (false == m_bIsLiveStreamingRunning){
					//m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO, 0, 0);
					if (m_pAudioAlarmListener != nullptr)
					{
						m_pAudioAlarmListener->FireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO);
					}
				}
				return true;
			}
			else if (AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType)
			{
				m_iAudioVersionFriend = nVersion;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}


	bool FarEndProcessorCall::IsQueueEmpty()
	{
		if (m_bIsLiveStreamingRunning)
		{
#ifdef LOCAL_SERVER_LIVE_CALL
			if ((m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER || m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE) && m_AudioReceivedBuffer->GetQueueSize() == 0)	//EncodedData
			{
				Tools::SOSleep(5);
				return true;
			}
			else if (m_nEntityType != ENTITY_TYPE_PUBLISHER_CALLER && m_vAudioFarEndBufferVector[0]->GetQueueSize() == 0)	//All Viewers ( including callee)
			{
				Tools::SOSleep(5);
				return true;
			}
#else
			if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER && m_AudioReceivedBuffer->GetQueueSize() == 0)	//EncodedData
			{
				Tools::SOSleep(5);
				return true;
			}
			else if (m_nEntityType != ENTITY_TYPE_PUBLISHER_CALLER && m_vAudioFarEndBufferVector[0]->GetQueueSize() == 0)	//All Viewers ( including callee)
			{
				Tools::SOSleep(5);
				return true;
			}
#endif
		}
		else if (m_AudioReceivedBuffer->GetQueueSize() == 0)
		{
			//Tools::SOSleep(10);
			return true;
		}
		return false;
	}


} //namespace MediaSDK
