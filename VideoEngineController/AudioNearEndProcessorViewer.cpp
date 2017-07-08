#include "AudioNearEndProcessorViewer.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioPacketHeader.h"
#include "Tools.h"



namespace MediaSDK
{

	AudioNearEndProcessorViewer::AudioNearEndProcessorViewer(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SmartPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning) :
		AudioNearEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, pAudioEncodingBuffer, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#nearEnd# AudioNearEndProcessorViewerInCall::AudioNearEndProcessorViewerInCall()");

		m_pAudioNearEndBuffer = pAudioEncodingBuffer;
		m_pAudioCallSession = pAudioCallSession;
	}


	void AudioNearEndProcessorViewer::ProcessNearEndData()
	{
		//	MR_DEBUG("#nearEnd# AudioNearEndProcessorViewerInCall::ProcessNearEndData()");

		int version = 0;
		long long llCapturedTime, llRelativeTime = 0, llLasstTime = -1;
		if (m_pAudioNearEndBuffer->GetQueueSize() == 0)
			Tools::SOSleep(10);
		else
		{
			LOG18("#18#NE#Viewer... ");
			m_pAudioNearEndBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
			int nDataLenthInShort = AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING;

			m_pAudioCallSession->m_ViewerInCallSentDataQueue->EnQueue(m_saAudioRecorderFrame, nDataLenthInShort, m_iPacketNumber);

			DumpEncodingFrame();
			UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);

			if (false == PreProcessAudioBeforeEncoding())
			{
				return;
			}

			m_nRawFrameSize = CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short);
			memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saAudioRecorderFrame, m_nRawFrameSize);

			int iSlotID = 0;
			int iPrevRecvdSlotID = 0;
			int iReceivedPacketsInPrevSlot = 0;
			int nChannel = 0;

			m_ucaRawFrameNonMuxed[0] = 0;
			BuildAndGetHeaderInArray(AUDIO_LIVE_CALLEE_PACKET_TYPE, m_MyAudioHeadersize, 0, iSlotID, m_iPacketNumber, m_nRawFrameSize,
				iPrevRecvdSlotID, iReceivedPacketsInPrevSlot, nChannel, version, llRelativeTime, &m_ucaRawFrameNonMuxed[1]);

			++m_iPacketNumber;
			if (m_iPacketNumber == m_llMaxAudioPacketNumber)
			{
				m_iPacketNumber = 0;
			}
			LOG18("#18#NE#Viewer  StoreDataForChunk");
			StoreDataForChunk(m_ucaRawFrameNonMuxed, llRelativeTime, 1600);

			Tools::SOSleep(0);
		}
	}

} //namespace MediaSDK
