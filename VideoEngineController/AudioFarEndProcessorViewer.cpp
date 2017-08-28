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
			nQueueSize += m_vAudioFarEndBufferVector[i]->GetQueueSize();
		}

		int nCalleeId = 1;
		m_vFrameMissingBlocks.clear();
		int iTotalFrameCounter = 0;

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
					MediaLog(LOG_WARNING, "[AFEPV] [iterator:%d] m_nDecodingFrameSize = %d",iterator, m_nDecodingFrameSize);
					continue;
				}

				/// ----------------------------------------- TEST CODE FOR VIWER IN CALL ----------------------------------------------///

				llCapturedTime = Tools::CurrentTimestamp();

				int dummy;
				int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
				int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength;
				ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
					nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);

				MediaLog(LOG_CODE_TRACE, "[AFEPV] [iterator:%d]  PT:%d PN:%d BN:%d DataLen:%d FL:%d", iterator, nCurrentAudioPacketType, iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);				

				if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
				{
					MediaLog(LOG_WARNING, "[AFEPV] [iterator:%d] nCurrentAudioPacketType = %d", iterator, nCurrentAudioPacketType);
					continue;
				}

				bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
				llNow = Tools::CurrentTimestamp();
				bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);				

				if (bIsCompleteFrame)
				{
					m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;

					if (!IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
					{
						MediaLog(LOG_WARNING, "[AFEPV] [iterator:%d] nCurrentAudioPacketType = %d", iterator, llRelativeTime);
						continue;
					}

					llNow = Tools::CurrentTimestamp();
					int nEncodedFrameSize = m_nDecodingFrameSize - nCurrentPacketHeaderLength;					
										
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
					
					MediaLog(LOG_CODE_TRACE, "[AFEPV]  [Iterator:%d] EncodedFrameSize = %d, DecodedFrameSize = %d, HL=%d", iterator, nEncodedFrameSize, m_nDecodedFrameSize, nCurrentPacketHeaderLength);
					PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);					

					if (m_nDecodedFrameSize < 1)
					{
						MediaLog(LOG_WARNING, "[AFEPV] [iterator:%d]  REMOVED DECODED FRAME# LEN = %d", m_nDecodedFrameSize);
						continue;
					}
					m_pAudioMixer->AddDataToPCMAdder(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);
					MediaLog(LOG_INFO, "[AFEPV] Viewer# SendToPlayer, FN = %d", iPacketNumber);
					LOGFARQUAD("Farquad calling SendToPlayer viewer");
				}
			}
			
			m_pAudioMixer->GetAddedData(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);	/*Mixed Audio Data*/

			DumpDecodedFrame(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);
			MediaLog(LOG_INFO, "[AFEPV] Publisher# SendToPlayer, FN = %d", iPacketNumber);

			SendToPlayer(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING, m_llLastTime, iPacketNumber);
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

						while (0 < m_pAudioCallSession->m_ViewerInCallSentDataQueue->GetQueueSize())
						{
							nSize = m_pAudioCallSession->m_ViewerInCallSentDataQueue->DeQueueForCallee(m_saCalleeSentData, llLastFrameNumber, nGetOwnFrameNumber);
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

				MediaLog(LOG_INFO, "[AFEPV] Viewer# SendToPlayer, FN = %d", iPacketNumber); 
				LOGFARQUAD("Farquad calling SendToPlayer viewer");
				SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber);
				Tools::SOSleep(0);
			}
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

