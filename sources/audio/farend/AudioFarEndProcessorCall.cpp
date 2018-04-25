#include "AudioFarEndProcessorCall.h"
#include "Tools.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioDePacketizer.h"
#include "LiveAudioDecodingQueue.h"
#include "AudioFarEndDataProcessor.h"
#include "AudioNearEndDataProcessor.h"
#include "AudioDecoderBuffer.h"
#include "AudioPacketHeader.h"
#include "AudioTypes.h"
#include "InterfaceOfAudioVideoEngine.h"


namespace MediaSDK
{
	FarEndProcessorCall::FarEndProcessorCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, bIsLiveStreamingRunning),
		m_iLastFarEndFrameNumber(-1)
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
				MediaLog(LOG_WARNING, "[FE] Too small data");
				return;
			}

			llCapturedTime = Tools::CurrentTimestamp();

			int dummy;
			int nPacketDataLength, nChannel, nVersion;
			int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength, nEchoStateFlags;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, iPacketNumber, nPacketDataLength,
				nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength, nEchoStateFlags);

			

			MediaLog(LOG_DEBUG, "[FE][AFEDPC][POL] FarFrameNumber = %d DataLen = %d", iPacketNumber, nPacketDataLength);
			
			MediaLog(CODE_TRACE, "[FE][AFEDPC][ECHOFLAG] playerside nEchoStateFlags = %d, Version = %d, HeaderLen = %d\n", nEchoStateFlags, nVersion, nCurrentPacketHeaderLength);

			/* Skipping old and duplicate packets. */
			if (m_iLastFarEndFrameNumber >= iPacketNumber)
			{
				MediaLog(LOG_WARNING, "[FE][AFEDPC] DISCURDING OLD FRAME. FN = %d", iPacketNumber);
				return;
			}

			if (m_iLastFarEndFrameNumber + 1 < iPacketNumber){
				int nMissingFrames = iPacketNumber - m_iLastFarEndFrameNumber - 1;
				MediaLog(LOG_WARNING, "[FE][AFEPC][MISS] MISSING FRAMES ---------------------------------> %d  [%d-%d]\n", nMissingFrames, m_iLastFarEndFrameNumber + 1, iPacketNumber - 1);
			}

			m_iLastFarEndFrameNumber = iPacketNumber;

			if (!IsPacketTypeSupported(nCurrentAudioPacketType))
			{
				MediaLog(LOG_WARNING, "[FE][AFEDPC] REMOVED PACKET TYPE SUPPORTED");
				return;
			}

			if (!IsPacketProcessableInNormalCall(nCurrentAudioPacketType, nVersion))
			{
				MediaLog(LOG_WARNING, "[FE][AFEDPC] REMOVED PACKET PROCESSABLE IN NORMAL CALL");
				return;
			}

			bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
			llNow = Tools::CurrentTimestamp();
			bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);
			
			if (bIsCompleteFrame){
				
				MediaLog(CODE_TRACE, "[FE][AFEDPC] Complete[%d %d]", iPacketNumber, iBlockNumber);
				m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
			}

			llNow = Tools::CurrentTimestamp();

			if (bIsCompleteFrame){				
				m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
				
				DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);
				DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
				PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
				MediaLog(CODE_TRACE, "[FE][AFEDPC] WORKING ON COMPLETE FRAME. Decoded Size: %d", m_nDecodedFrameSize);				

				if (m_nDecodedFrameSize < 1)
				{
					MediaLog(LOG_WARNING, "[FE][AFEDPC] REMOVED FRAME FOR LOW SIZE.");
					return;
				}

				MediaLog(CODE_TRACE, "[FE][AFEDPC] AudioCall SendToPlayer");

				SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber, nEchoStateFlags);
				if (!m_pAudioCallSession->m_pNearEndProcessor->IsEchoCancellerEnabled())
				{
					ProcessPlayingData();
				}
			}
		}
		else
		{
			if (!m_pAudioCallSession->m_pNearEndProcessor->IsEchoCancellerEnabled())
			{
				Tools::SOSleep(10);
			}
		}
		if (m_pAudioCallSession->m_pNearEndProcessor->IsEchoCancellerEnabled())
		{
			ProcessPlayingData();
		}
	}


	void FarEndProcessorCall::DequeueData(int &decodingFrameSize)
	{
		if (m_bIsLiveStreamingRunning)
		{
#ifndef LOCAL_SERVER_LIVE_CALL
			if (m_nEntityType != ENTITY_TYPE_PUBLISHER_CALLER)
			{
				decodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_nRelativeTimeOffset);
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
				if (false == m_bIsLiveStreamingRunning && m_pAudioCallSession->m_bIsVideoCallRunning){
					if (m_pAudioAlarmListener != nullptr)
					{
						m_pAudioAlarmListener->FireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO, 0, 0);
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
