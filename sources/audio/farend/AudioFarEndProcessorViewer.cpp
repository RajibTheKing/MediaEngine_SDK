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

	FarEndProcessorViewer::FarEndProcessorViewer(int nAudioFlowType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		AudioFarEndDataProcessor(nAudioFlowType, nEntityType, pAudioCallSession, bIsLiveStreamingRunning),
		m_nLastPublisherFrame(-1)
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

				m_nDecodingFrameSize = m_vAudioFarEndBufferVector[iterator]->DeQueue(m_ucaDecodingFrame, m_nRelativeTimeOffset);

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

				if (0 == iterator)
				{
					m_pAudioCallSession->m_pPlayedPublisherFE->WriteDump(m_saDecodedFrame, 2, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);
				}
				if (1 == iterator)
				{
					m_pAudioCallSession->m_pPlayedCalleeFE->WriteDump(m_saDecodedFrame, 2, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);
				}				

				naFrameNumbers[iterator] = iPacketNumber;

				m_pAudioMixer->AddDataToPCMAdder(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);					
				LOGFARQUAD("Farquad calling SendToPlayer viewer");
				

				MediaLog(LOG_CODE_TRACE, "[AFEPV] [Iterator:%d] USED FRAME# FN: %d EncodedFrameSize = %d, DecodedFrameSize = %d, HL=%d", iterator, iPacketNumber, nEncodedFrameSize, m_nDecodedFrameSize, nCurrentPacketHeaderLength);
			}
			
			m_pAudioMixer->GetAddedData(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);	/*Mixed Audio Data*/

			DumpDecodedFrame(m_saDecodedFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);
			MediaLog(LOG_INFO, "[AFEPV] Viewer-SendToPlayer# PublisherFN = %d, CalleeFN = %d", naFrameNumbers[0], naFrameNumbers[1]);

			if (m_nLastPublisherFrame + 1 < naFrameNumbers[0]){
				int nMissingFrames = naFrameNumbers[0] - m_nLastPublisherFrame - 1;
				MediaLog(LOG_WARNING, "[FE][AFEPV][MISS] MISSING FRAMES ---------------------------------> %d  [%d-%d]\n", nMissingFrames, m_nLastPublisherFrame + 1, naFrameNumbers[0] - 1);
			}

			m_nLastPublisherFrame = naFrameNumbers[0];

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
		//Romoved PCM Codes.
	}

} //namespace MediaSDK

