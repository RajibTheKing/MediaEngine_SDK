#include "InterfaceOfAudioVideoEngine.h"
#include "AudioNearEndProcessorCall.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "AudioPacketHeader.h"
#include "AudioEncoderInterface.h"
#include "AudioFarEndDataProcessor.h"
#include "AudioFileCodec.h"
#include "AudioLinearBuffer.h"
#include "Tools.h"
#include "MediaLogger.h"



namespace MediaSDK
{

	AudioNearEndProcessorCall::AudioNearEndProcessorCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioNearEndBuffer, bool bIsLiveStreamingRunning, const bool &isVideoCallRunning) :
		AudioNearEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, pAudioNearEndBuffer, bIsLiveStreamingRunning),
		m_nEncodedFrameSize(0),
		m_bIsVideoCallRunning(isVideoCallRunning)
	{
		MR_DEBUG("#nearEnd# AudioNearEndProcessorCall::AudioNearEndProcessorCall()");
	}


	void AudioNearEndProcessorCall::ProcessNearEndData()
	{
		//	MR_DEBUG("#nearEnd# AudioNearEndProcessorCall::ProcessNearEndData()");

		int version = 0;
		long long llCapturedTime, llRelativeTime = 0, llLasstTime = -1;;
		if (m_pAudioCallSession->m_recordBuffer->PopData(m_saAudioRecorderFrame) == 0)
		{
			Tools::SOSleep(10);
		}
		else
		{
			//LOGT("##TT dequed #18#NE#AudioCall...");
			int nEchoStateFlags = m_pAudioCallSession->PreprocessAudioData(m_saAudioRecorderFrame, CHUNK_SIZE);
			//m_pAudioNearEndBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);

			DumpEncodingFrame();
			UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);

			if (false == PreProcessAudioBeforeEncoding())
			{
				return;
			}

			long long llEncodingTime, llTimeBeforeEncoding = Tools::CurrentTimestamp();
			m_nEncodedFrameSize = m_pAudioEncoder->EncodeAudio(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), &m_ucaEncodedFrame[1 + m_MyAudioHeadersize]);

			//ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize) + " PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
			llEncodingTime = Tools::CurrentTimestamp() - llTimeBeforeEncoding;
			this->DecideToChangeComplexity(llEncodingTime);


			int iSlotID = 0;
			int nChannel = 0;
			int nPacketType = AUDIO_NORMAL_PACKET_TYPE;

			BuildAndGetHeaderInArray(AUDIO_NORMAL_PACKET_TYPE, m_MyAudioHeadersize, 0, m_iPacketNumber, m_nEncodedFrameSize, nChannel, version, llRelativeTime, nEchoStateFlags, &m_ucaEncodedFrame[1]);

			MediaLog(LOG_DEBUG, "[ANEPC] FrameNo = %d, RT = %lld, PacketType = %d, EncodedSize = %d",
				m_iPacketNumber, llRelativeTime, nPacketType, m_nEncodedFrameSize);

			++m_iPacketNumber;
			if (m_iPacketNumber == m_llMaxAudioPacketNumber)
			{
				m_iPacketNumber = 0;
			}

			SentToNetwork(llRelativeTime);
			LOG18("#18#NE#AudioCall Sent");
			Tools::SOSleep(0);
		}
	}


	void AudioNearEndProcessorCall::SentToNetwork(long long llRelativeTime)
	{
#ifdef  AUDIO_SELF_CALL //Todo: build while this is enable
		//Todo: m_AudioReceivedBuffer fix. not member of this class
		if (m_bIsLiveStreamingRunning == false)
		{
			ALOG("#A#EN#--->> Self#  PacketNumber = " + Tools::IntegertoStringConvert(m_iPacketNumber));
			m_pAudioCallSession->m_pFarEndProcessor->m_AudioReceivedBuffer->EnQueue(m_ucaEncodedFrame + 1, m_nEncodedFrameSize + m_MyAudioHeadersize);
			return;
		}
#endif

#ifndef NO_CONNECTIVITY
		//	MR_DEBUG("#ptt# SentToNetwork, %x", *m_cbOnDataReady);
		//m_pCommonElementsBucket->SendFunctionPointer(m_llFriendID, MEDIA_TYPE_AUDIO, m_ucaEncodedFrame, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, 0, std::vector< std::pair<int, int> >());
		if (m_pDataReadyListener != nullptr)
		{
			m_pDataReadyListener->OnDataReadyToSend(MEDIA_TYPE_AUDIO, m_ucaEncodedFrame, m_nEncodedFrameSize + m_MyAudioHeadersize + 1);
		}
#else
		//m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
		if (m_pPacketEventListener != nullptr)
		{
			m_pPacketEventListener->FirePacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
		}
#endif

#ifdef  DUPLICATE_AUDIO
		if (false == m_bIsLiveStreamingRunning && m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning())
		{
			Tools::SOSleep(5);
#ifndef NO_CONNECTIVITY
			//m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, MEDIA_TYPE_AUDIO, m_ucaEncodedFrame, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, 0, std::vector< std::pair<int, int> >());
			if (m_pDataReadyListener != nullptr)
			{
				m_pDataReadyListener->OnDataReadyToSend(MEDIA_TYPE_AUDIO, m_ucaEncodedFrame, m_nEncodedFrameSize + m_MyAudioHeadersize + 1);
			}
#else
			//m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
			if (m_pPacketEventListener != nullptr)
			{
				m_pPacketEventListener->FirePacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
			}
#endif
		}
#endif
	}


	void AudioNearEndProcessorCall::DecideToChangeComplexity(int iEncodingTime)
	{
		int nComplexity = m_pAudioEncoder->GetComplexity();

		if (iEncodingTime > AUDIO_MAX_TOLERABLE_ENCODING_TIME && nComplexity > OPUS_MIN_COMPLEXITY)
		{
			m_pAudioEncoder->SetComplexity(nComplexity - 1);
		}

		if (iEncodingTime < AUDIO_MAX_TOLERABLE_ENCODING_TIME / 2 && nComplexity < OPUS_MAX_COMPLEXITY)
		{
			m_pAudioEncoder->SetComplexity(nComplexity + 1);
		}
	}
} //namespace MediaSDK
