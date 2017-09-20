#include "AudioFarEndProcessorViewer.h"
#include "AudioCallSession.h"
#include "AudioDePacketizer.h"
#include "AudioMixer.h"
#include "AudioEncoderBuffer.h"
#include "LogPrinter.h"
#include "LiveAudioDecodingQueue.h"
#include "AudioPacketHeader.h"
#include "Tools.h"
#include "AudioDecoderInterface.h"
#include "MediaLogger.h"
#include "DecoderOpus.h"

namespace MediaSDK
{

	FarEndProcessorViewer::FarEndProcessorViewer(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#farEnd# FarEndProcessorViewer::FarEndProcessorViewer()");
		m_pCalleeDecoderOpus.reset(new DecoderOpus());
		m_pAudioMixer.reset(new AudioMixer(BITS_USED_FOR_AUDIO_MIXING, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING));
	}

	void FarEndProcessorViewer::ProcessFarEndData()
	{
		if (m_pAudioCallSession->IsOpusEnable())
		{
			ProcessFarEndDataOpus();
		}
		else 
		{
			ProcessFarEndDataPCM();
		}
	}

	void FarEndProcessorViewer::ProcessFarEndDataOpus()
	{
		//	MR_DEBUG("#farEnd# FarEndProcessorViewer::ProcessFarEndData()");

		int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
		long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
		double dbTotalTime = 0; //MeaningLess

		int iDataSentInCurrentSec = 0; //NeedToFix.
		long long llTimeStamp = 0;
		int nQueueSize = 0;

		int nRequiredCallPerticipantNumber = MAX_NUMBER_OF_CALL_PARTICIPANTS;

		if (ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			nRequiredCallPerticipantNumber = 1;	/*Callee will discard his own data.*/
		}

		for (int i = 0; i < nRequiredCallPerticipantNumber; i++)
		{
			 int nCurrentSize= m_vAudioFarEndBufferVector[i]->GetQueueSize();
			 MediaLog(LOG_CODE_TRACE, "[AFEPV] [i:%d] QueueSize = %d", i, nCurrentSize);
			 nQueueSize += nCurrentSize;
		}

		MediaLog(LOG_CODE_TRACE, "[AFEPV]  nQueueSize = %d nRequiredCallPerticipantNumber = %d", nQueueSize, nRequiredCallPerticipantNumber);

		int nCalleeId = 1;
		m_vFrameMissingBlocks.clear();
		int iTotalFrameCounter = 0;
		int naFrameNumbers[2];
		naFrameNumbers[0] = naFrameNumbers[1] = -1;
		int nEchoStateFlags = 0;
		if (nQueueSize > 0)
		{
			m_pAudioMixer->ResetPCMAdder();

			for (int iterator = 0; iterator < nRequiredCallPerticipantNumber; iterator++)
			{
				if (0 == m_vAudioFarEndBufferVector[iterator]->GetQueueSize())
				{
					continue;
				}								

				m_nDecodingFrameSize = m_vAudioFarEndBufferVector[iterator]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);

				if (m_nDecodingFrameSize < 1)
				{
					MediaLog(LOG_WARNING, "[AFEPV] [Iterator:%d] Too Small Frame!!!!!!!. DecodingFrameSize = %d",iterator, m_nDecodingFrameSize);
					continue;
				}				

				llCapturedTime = Tools::CurrentTimestamp();

				
				int nPacketDataLength, nChannel, nVersion;
								

				ParseLiveHeader(nCurrentAudioPacketType, nCurrentPacketHeaderLength, nVersion, iPacketNumber, nPacketDataLength,
					llRelativeTime, nEchoStateFlags, m_ucaDecodingFrame);

				MediaLog(LOG_CODE_TRACE, "[AFEPV] [Iterator:%d] PT:%d PN:%d DL:%d RT:%lld", iterator, nCurrentAudioPacketType, iPacketNumber, nPacketDataLength, llRelativeTime);

				if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
				{
					MediaLog(LOG_WARNING, "[AFEPV] [Iterator:%d] Not Processable based on Role!!!!!. PT = %d", iterator, nCurrentAudioPacketType);
					continue;
				}

				m_nDecodedFrameSize = -1;
				bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
				llNow = Tools::CurrentTimestamp();
				
				int nEncodedFrameSize = m_nDecodingFrameSize - nCurrentPacketHeaderLength;

				MediaLog(LOG_CODE_TRACE, "[AFEPV] bIsCompleteFrame = %d\n", bIsCompleteFrame);
				
				/*Skip delay packet only for publisher.*/
				if (0 == iterator && !IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
				{
					MediaLog(LOG_WARNING, "[AFEPV]  [Iterator:%d] nCurrentAudioPacketType = %d", iterator, llRelativeTime);
					continue;
				}

				llNow = Tools::CurrentTimestamp();
															
				if (0 == iterator)
				{
					/* OPUS Decoder for Publisher*/
					m_nDecodedFrameSize = m_pAudioCallSession->GetAudioDecoder()->DecodeAudio(m_ucaDecodingFrame + nCurrentPacketHeaderLength, nEncodedFrameSize, m_saDecodedFrame);
				}
				else
				{
					/* OPUS Decoder for Callee Data*/
					m_nDecodedFrameSize = m_pCalleeDecoderOpus->DecodeAudio(m_ucaDecodingFrame + nCurrentPacketHeaderLength, nEncodedFrameSize, m_saDecodedFrame);
				}
										
				PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);					

				if (m_nDecodedFrameSize < 1)
				{
					MediaLog(LOG_WARNING, "[AFEPV] [Iterator:%d] REMOVED DECODED FRAME# LEN = %d", m_nDecodedFrameSize);
					continue;
				}

				naFrameNumbers[iterator] = iPacketNumber;

				m_pAudioMixer->AddDataToPCMAdder(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);					
				LOGFARQUAD("Farquad calling SendToPlayer viewer");
				

				MediaLog(LOG_CODE_TRACE, "[AFEPV] [Iterator:%d] USED FRAME# FN: %d EncodedFrameSize = %d, DecodedFrameSize = %d, HL=%d", iterator, iPacketNumber, nEncodedFrameSize, m_nDecodedFrameSize, nCurrentPacketHeaderLength);
			}
			
			m_pAudioMixer->GetAddedData(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);	/*Mixed Audio Data*/

			DumpDecodedFrame(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);
			MediaLog(LOG_INFO, "[AFEPV] Viewer-SendToPlayer# PublisherFN = %d, CalleeFN = %d", naFrameNumbers[0], naFrameNumbers[1]);

			SendToPlayer(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING, m_llLastTime, iPacketNumber, nEchoStateFlags);
			Tools::SOSleep(0);
		}
		else
		{
			Tools::SOSleep(10);
		}
#ifdef USE_AECM
		if (m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
		{
			LOGFARQUAD("Farquad calling ProcessPlayingData viewer");
			ProcessPlayingData();
		}
#endif

	}



	void FarEndProcessorViewer::ProcessFarEndDataPCM()
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

			int nPacketDataLength, nVersion;
			int nEchoStateFlags;

			ParseLiveHeader(nCurrentAudioPacketType, nCurrentPacketHeaderLength, nVersion, iPacketNumber, nPacketDataLength,
				llRelativeTime, nEchoStateFlags, m_ucaDecodingFrame);
			
			m_nDecodingFrameSize = nPacketDataLength;

			MediaLog(LOG_CODE_TRACE, "[AFEPV] PT:%d PN:%d DataLen:%d RT:%lld ESF:%d", nCurrentAudioPacketType, iPacketNumber, nPacketDataLength, llRelativeTime, nEchoStateFlags);

			if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
			{
				MediaLog(LOG_WARNING, "[AFEPV] Not Processable based on Role!!!!!. PT = %d", nCurrentAudioPacketType);
				return;
			}

			bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
			llNow = Tools::CurrentTimestamp();						
							
			if (!IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
			{
				MediaLog(LOG_CODE_TRACE, "[AFEPV] REMOVED ON RELATIVE TIME");
				return;
			}


			MediaLog(LOG_CODE_TRACE, "[AFEPV] WORKING ON COMPLETE FRAME . ");
			m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
			MediaLog(LOG_CODE_TRACE, "[AFEPV] -> HEHE %d %d", m_nDecodingFrameSize, nCurrentPacketHeaderLength);

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

					//MediaLog(LOG_CODE_TRACE, "[AFEPV] m_ViewerInCallSentDataQueue # Queue Size %d", m_pAudioCallSession->m_ViewerInCallSentDataQueue.GetQueueSize());

					while (0 < m_pAudioCallSession->m_ViewerInCallSentDataQueue->GetQueueSize())
					{
						nSize = m_pAudioCallSession->m_ViewerInCallSentDataQueue->DeQueueForCallee(m_saCalleeSentData, llLastFrameNumber, nGetOwnFrameNumber);
						MediaLog(LOG_CODE_TRACE, "[AFEPV] FOUND OWNFrame %lld, queued frame no, %lld", nGetOwnFrameNumber, llLastFrameNumber);

						if (nSize == -1) {
							MediaLog(LOG_CODE_TRACE, "[AFEPV] FOUND EITHER CURRENT FRAME IS GREATER THAN FRONT OF QUEUE OR NO DATA IN QUEUE");
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
						MediaLog(LOG_CODE_TRACE, "[AFEPV] FOUND REMOVED AUDIO DATA");
						m_nDecodedFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
					}
					else
					{
						//Do Some thing;
						MediaLog(LOG_CODE_TRACE, "[AFEPV] FOUND REMOVED AUDIO DATA with -1");
						nCalleeId = -1;
						m_nDecodedFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
					}
				}
				else //For Only Viewers
				{
					MediaLog(LOG_CODE_TRACE, "[AFEPV] FOUND REMOVED AUDIO DATA ONLY VIEWR");
					nCalleeId = -1;
					m_nDecodedFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
				}
			}
			else
			{
				MediaLog(LOG_CODE_TRACE, "[AFEPV] FOUND REMOVED AUDIO DATA ONLY VIEWR wrong media");
				memcpy(m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize);
				m_nDecodedFrameSize = m_nDecodingFrameSize / sizeof(short);
			}

			MediaLog(LOG_CODE_TRACE, "[AFEPV] Viewer  m_nDecodingFrameSize = %d", m_nDecodingFrameSize);
			DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
			PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
			MediaLog(LOG_CODE_TRACE, "[AFEPV] AFTER POST PROCESS ... deoding frame size %d", m_nDecodedFrameSize);
			if (m_nDecodedFrameSize < 1)
			{
				MediaLog(LOG_CODE_TRACE, "[AFEPV] REMOVED FOR LOW SIZE.");
				return;
			}

			MediaLog(LOG_INFO, "[AFEPV] Viewer# SendToPlayer, FN = %d", iPacketNumber); 
			LOGFARQUAD("Farquad calling SendToPlayer viewer");
			SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber, nEchoStateFlags);
			Tools::SOSleep(0);			
		}
		else 
		{
			Tools::SOSleep(10);
		}
#ifdef USE_AECM
		if (m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
		{
			LOGFARQUAD("Farquad calling ProcessPlayingData viewer");
			ProcessPlayingData();
		}
#endif
		
	}

} //namespace MediaSDK

