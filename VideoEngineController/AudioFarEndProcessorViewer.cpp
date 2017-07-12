#include "AudioFarEndProcessorViewer.h"
#include "AudioMixer.h"

namespace MediaSDK
{

	FarEndProcessorViewer::FarEndProcessorViewer(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(llFriendID, nServiceType, nEntityType, pAudioCallSession, pCommonElementsBucket, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#farEnd# FarEndProcessorViewer::FarEndProcessorViewer()");
	}


	void FarEndProcessorViewer::ProcessFarEndData()
	{
		//	MR_DEBUG("#farEnd# FarEndProcessorViewer::ProcessFarEndData()");

		int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
		long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
		double dbTotalTime = 0; //MeaningLess

		int iDataSentInCurrentSec = 0; //NeedToFix.
		long long llTimeStamp = 0;
		int nQueueSize = m_vAudioFarEndBufferVector[0]->GetQueueSize();
		int nCalleeId = 1;
		m_vFrameMissingBlocks.clear();
		if (nQueueSize > 0)
		{
			m_nDecodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);
			DOG("#18#FE#Viewer... ");

			if (m_nDecodingFrameSize < 1)
			{
				//LOGE("##DE# CAudioCallSession::DecodingThreadProcedure queue size 0.");
				return;
			}

			/// ----------------------------------------- TEST CODE FOR VIWER IN CALL ----------------------------------------------///

			llCapturedTime = Tools::CurrentTimestamp();

			int dummy;
			int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
			int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
				nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);

			DOG("XXP@#@#MARUF FOUND DATA OF LENGTH -> [%d %d] %d frm len = %d", iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);

			if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
			{
				DOG("XXP@#@#MARUF REMOVED IN BASED ON PACKET PROCESSABLE ON ROLE");
				return;
			}

			bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
			llNow = Tools::CurrentTimestamp();
			bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);
			DOG("XXP@#@#MARUF [%d %d]", iPacketNumber, iBlockNumber);
			if (bIsCompleteFrame){
				//m_ucaDecodingFrame
				DOG("XXP@#@#MARUF Complete[%d %d]", iPacketNumber, iBlockNumber);

				m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
				if (!IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
				{
					DOG("XXP@#@#MARUF REMOVED ON RELATIVE TIME");
					return;
				}
			}
			llNow = Tools::CurrentTimestamp();

			if (bIsCompleteFrame){
				DOG("XXP@#@#MARUF WORKING ON COMPLETE FRAME . ");
				m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
				DOG("XXP@#@#MARUF  -> HEHE %d %d", m_nDecodingFrameSize, nCurrentPacketHeaderLength);

				//DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);

				if (AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED == nCurrentAudioPacketType)
				{
					if (m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
					{
						nCalleeId = 1;	//Should be fixed.
						long long nGetOwnFrameNumber;
						nGetOwnFrameNumber = m_pAudioMixer->GetAudioFrameByParsingMixHeader(m_ucaDecodingFrame + nCurrentPacketHeaderLength, nCalleeId);
						long long llLastFrameNumber;
						int nSize;
						bool bFound = false;

						DOG("#18@# m_ViewerInCallSentDataQueue # Queue Size %d", m_pAudioCallSession->m_ViewerInCallSentDataQueue.GetQueueSize());

						while (0 < m_pAudioCallSession->m_ViewerInCallSentDataQueue.GetQueueSize())
						{
							nSize = m_pAudioCallSession->m_ViewerInCallSentDataQueue.DeQueueForCallee(m_saCalleeSentData, llLastFrameNumber, nGetOwnFrameNumber);
							DOG("#18@# FOUND OWNFrame %lld, queued frame no, %lld", nGetOwnFrameNumber, llLastFrameNumber);

							if (nSize == -1) {
								DOG("#18@# FOUND EITHER CURRENT FRAME IS GREATER THAN FRONT OF QUEUE OR NO DATA IN QUEUE");
								break;
							}

							if (nGetOwnFrameNumber == llLastFrameNumber)
							{
								bFound = true;
								break;
							}
						}
						if (bFound)
						{
							DOG("#18@# FOUND REMOVED AUDIO DATA");
							m_nDecodedFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
						}
						else
						{
							//Do Some thing;
							DOG("#18@# FOUND REMOVED AUDIO DATA with -1");
							nCalleeId = -1;
							m_nDecodedFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
						}
					}
					else //For Only Viewers
					{
						DOG("#18@# FOUND REMOVED AUDIO DATA ONLY VIEWR");
						nCalleeId = -1;
						m_nDecodedFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
					}
				}
				else
				{
					DOG("#18@# FOUND REMOVED AUDIO DATA ONLY VIEWR wrong media");
					memcpy(m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize);
					m_nDecodedFrameSize = m_nDecodingFrameSize / sizeof(short);
				}

				DOG("#18#FE#Viewer  m_nDecodingFrameSize = %d", m_nDecodingFrameSize);
				DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
				PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
				DOG("XXP@#@#MARUF AFTER POST PROCESS ... deoding frame size %d", m_nDecodedFrameSize);
				if (m_nDecodedFrameSize < 1)
				{
					DOG("XXP@#@#MARUF REMOVED FOR LOW SIZE.");
					return;
				}
				DOG("#18#FE#Viewer  SendToPlayer");
				SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber);
#ifdef USE_AECM
				if (m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
				{
					ProcessPlayingData();
				}
#endif
				Tools::SOSleep(0);
			}
		}
		else {
			Tools::SOSleep(5);
		}
		
	}

} //namespace MediaSDK

