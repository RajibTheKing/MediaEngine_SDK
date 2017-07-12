#include "AudioNearEndProcessorPublisher.h"
#include "AudioShortBufferForPublisherFarEnd.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioPacketHeader.h"
#include "AudioMixer.h"
#include "MuxHeader.h"
#include "Tools.h"



namespace MediaSDK
{

	AudioNearEndProcessorPublisher::AudioNearEndProcessorPublisher(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SmartPointer<CAudioShortBuffer> pAudioNearEndBuffer, bool bIsLiveStreamingRunning) :
		AudioNearEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, pAudioNearEndBuffer, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#nearEnd# AudioNearEndProcessorPublisher::AudioNearEndProcessorPublisher()");

		m_pAudioMixer.reset(new AudioMixer(BITS_USED_FOR_AUDIO_MIXING, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING));
	}


	void AudioNearEndProcessorPublisher::ProcessNearEndData()
	{
		//	MR_DEBUG("#nearEnd# AudioNearEndProcessorPublisher::ProcessNearEndData()");

		int version = 0;
		long long llCapturedTime, llRelativeTime = 0, llLasstTime = -1;
		if (m_pAudioNearEndBuffer->GetQueueSize() == 0)
			Tools::SOSleep(10);
		else
		{
			LOG18("#18#NE#Publisher...");
			m_pAudioNearEndBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
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


	bool AudioNearEndProcessorPublisher::MuxIfNeeded(short* shPublisherData, short *shMuxedData, int &nDataSizeInByte, int nPacketNumber)
	{
		long long nFrameNumber;
		nDataSizeInByte = 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING;
		int nLastDecodedFrameSizeInByte = 0;
		bool bIsMuxed = false;
		int iDataStartIndex, iDataEndIndex;
		int iCallId = 0, nNumberOfBlocks;
		std::vector<std::pair<int, int> > vMissingBlocks;
		MuxHeader oCalleeMuxHeader;
		if (m_pAudioCallSession->m_PublisherBufferForMuxing->GetQueueSize() != 0)
		{
			bIsMuxed = true;
			nLastDecodedFrameSizeInByte = m_pAudioCallSession->m_PublisherBufferForMuxing->DeQueue(m_saAudioPrevDecodedFrame, nFrameNumber, oCalleeMuxHeader) * 2;	//#Magic
			LOG18("#18@# DEQUE data of size %d", nLastDecodedFrameSizeInByte);
			m_pAudioMixer->reset(18, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);

			iDataStartIndex = 0;
			iDataEndIndex = 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING - 1;
			iCallId = 0;	//Publisher
			nNumberOfBlocks = 16;
			int nMuxHeaderSize = 14;

			MuxHeader oPublisherMuxHeader(iCallId, nPacketNumber, vMissingBlocks);
			LOG18("#18@# -> PUB ID %lld CALLEE ID %lld", oPublisherMuxHeader.getCalleeId(), oCalleeMuxHeader.getCalleeId());
			m_pAudioMixer->addAudioData((unsigned char*)shPublisherData, oPublisherMuxHeader); // this data should contains only the mux header

			if (nLastDecodedFrameSizeInByte == 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING) //Both must be 800
			{
				m_pAudioMixer->addAudioData((unsigned char*)m_saAudioPrevDecodedFrame, oCalleeMuxHeader); // this data should contains only the mux header

				nDataSizeInByte = m_pAudioMixer->getAudioData((unsigned char*)shMuxedData);
			}
			else
			{
				LOG18("#18@# RETURN WITH FALSE and zeor");
				nDataSizeInByte = 0;
				return false;
			}
		}
		else
		{
			memcpy(shMuxedData, m_saAudioRecorderFrame, nDataSizeInByte);
		}
#ifdef DUMP_FILE
		AudioMixer* DumpAudioMixer = new AudioMixer(18, 800);
		unsigned char temp[2000];

		if (nDataSizeInByte == 1800)
		{
			DumpAudioMixer->Convert18BitTo16Bit((unsigned char*)shMuxedData, temp, 800);
			fwrite((short *)temp, sizeof(short), 800, m_pAudioCallSession->File18BitData);
		}
		else {
			fwrite(shMuxedData, sizeof(short), 800, m_pAudioCallSession->File18BitData);
		}

		short val = nDataSizeInByte;
		LOG18("#18#NE#DMP nDataSizeInByte = %d", nDataSizeInByte);
		fwrite(&val, 2, 1, m_pAudioCallSession->File18BitType);

#endif
		return bIsMuxed;
	}


} //namespace MediaSDK
