#include "AudioNearEndProcessorPublisher.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"

namespace MediaSDK
{


	AudioNearEndProcessorPublisher::AudioNearEndProcessorPublisher(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CAudioShortBuffer *pAudioNearEndBuffer, bool bIsLiveStreamingRunning) :
		AudioNearEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, pAudioNearEndBuffer, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#nearEnd# AudioNearEndProcessorPublisher::AudioNearEndProcessorPublisher()");

		m_pAudioNearEndBuffer = pAudioNearEndBuffer;
	}

	//FILE* fp = fopen("/sdcard/out.pcm", "wb");

	void AudioNearEndProcessorPublisher::ProcessNearEndData()
	{
		//	MR_DEBUG("#nearEnd# AudioNearEndProcessorPublisher::ProcessNearEndData()");

		int version = 0;
		long long llCapturedTime = 0, llRelativeTime = 0, llLasstTime = -1;
		if (m_pAudioCallSession->m_recordBuffer->PopData(m_saAudioRecorderFrame) == 0)
		//if (m_pAudioNearEndBuffer->GetQueueSize() == 0)
		{
			Tools::SOSleep(10);
		}
		else
		{
			//fwrite(m_saAudioRecorderFrame, 2, CHUNK_SIZE, fp);
			LOGT("##TT#18#NE#Publisher...");
			//m_pAudioNearEndBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
			llCapturedTime = Tools::CurrentTimestamp();
			m_pAudioCallSession->PreprocessAudioData(m_saAudioRecorderFrame, CHUNK_SIZE);
			DumpEncodingFrame();

			int nSendingDataSizeInByte = 1600;	//Or contain 18 bit data with mixed header.
			bool bIsMuxed = false;

			if (false == PreProcessAudioBeforeEncoding())
			{
				return;
			}

			bIsMuxed = MuxIfNeeded(m_saAudioRecorderFrame, m_saSendingDataPublisher, nSendingDataSizeInByte, m_iPacketNumber);

			UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);

			memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saSendingDataPublisher, nSendingDataSizeInByte);

			int nSendingFramePacketType = bIsMuxed ? AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED : AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED;

			int iSlotID = 0;
			int iPrevRecvdSlotID = 0;
			int iReceivedPacketsInPrevSlot = 0;
			int nChannel = 0;
			m_ucaRawFrameNonMuxed[0] = 0;
			BuildAndGetHeaderInArray(nSendingFramePacketType, m_MyAudioHeadersize, 0, iSlotID, m_iPacketNumber, nSendingDataSizeInByte,
				iPrevRecvdSlotID, iReceivedPacketsInPrevSlot, nChannel, version, llRelativeTime, &m_ucaRawFrameNonMuxed[1]);

			++m_iPacketNumber;
			if (m_iPacketNumber == m_llMaxAudioPacketNumber)
			{
				m_iPacketNumber = 0;
			}
			LOG18("#18#NE#Publish  StoreDataForChunk nSendingDataSizeInByte = %d m_MyAudioHeadersize = %d", nSendingDataSizeInByte, m_MyAudioHeadersize);

			int nSendigFrameSize = nSendingDataSizeInByte + m_MyAudioHeadersize + 1;
			StoreDataForChunk(m_ucaRawFrameNonMuxed, llRelativeTime, nSendigFrameSize);

			Tools::SOSleep(0);
		}
	}

} //namespace MediaSDK
