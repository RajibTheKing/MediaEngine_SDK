#include "AudioNearEndProcessorViewer.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioPacketHeader.h"
#include "AudioLinearBuffer.h"
#include "Tools.h"
#include "AudioEncoderInterface.h"


namespace MediaSDK
{

	AudioNearEndProcessorViewer::AudioNearEndProcessorViewer(int nAudioFlowType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning) :
		AudioNearEndDataProcessor(nAudioFlowType, nEntityType, pAudioCallSession, pAudioEncodingBuffer, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#nearEnd# AudioNearEndProcessorViewerInCall::AudioNearEndProcessorViewerInCall()");
	}

	AudioNearEndProcessorViewer::~AudioNearEndProcessorViewer()
	{
	}


	void AudioNearEndProcessorViewer::ProcessNearEndData()
	{
		//	MR_DEBUG("#nearEnd# AudioNearEndProcessorViewerInCall::ProcessNearEndData()");

		int version = 0;
		long long llCapturedTime, llRelativeTime = 0, llLasstTime = -1;		
		int nPacketType = 0;
		int nEncodedFrameSizeB;

		if (m_recordBuffer->PopData(m_saAudioRecorderFrame) == 0)
		{
			Tools::SOSleep(10);
		}
		else
		{
			int nEchoStateFlags = PreprocessAudioData(m_saAudioRecorderFrame, CHUNK_SIZE);
			llCapturedTime = Tools::CurrentTimestamp();

			int nDataLenthInShort = AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING;

			if (false == m_pAudioCallSession->IsOpusEnable())
			{
				m_pAudioCallSession->m_ViewerInCallSentDataQueue->EnQueue(m_saAudioRecorderFrame, nDataLenthInShort, m_iPacketNumber);
			}

			m_nTotalSentFrameSize = m_iPacketNumber + 1;
			if (m_nTotalSentFrameSize % SESSION_STATISTICS_PACKET_INTERVAL == 0)
			{
				UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);
				// Make Chunk for Session Statistics
				m_ucaRawFrameForInformation[0] = 0;
				int nNowSendingDataSizeInByte = 1 + m_MyAudioHeadersize;

				int nSizeOfInformation = m_pAudioSessionStatistics->GetChunk(&m_ucaRawFrameForInformation[nNowSendingDataSizeInByte]);
				BuildHeaderForLive(SESSION_STATISTICS_PACKET_TYPE, m_MyAudioHeadersize, version, m_iPacketNumber, nSizeOfInformation, llRelativeTime, nEchoStateFlags, &m_ucaRawFrameForInformation[1]);
				
				nNowSendingDataSizeInByte += nSizeOfInformation;

				m_pAudioSessionStatistics->ResetVaryingData();

				StoreDataForChunk(m_ucaRawFrameForInformation, llRelativeTime, nNowSendingDataSizeInByte);
				llCapturedTime = Tools::CurrentTimestamp();
			}

			DumpEncodingFrame();
			UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);

			if (false == PreProcessAudioBeforeEncoding())
			{
				return;
			}

			int nFrameSizeInByte;

			if (m_pAudioCallSession->IsOpusEnable())	//For Opus
			{
				nPacketType = LIVE_CALLEE_PACKET_TYPE_OPUS;
				nEncodedFrameSizeB = m_pAudioEncoder->EncodeAudio(m_saAudioRecorderFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING, &m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize]);
				nFrameSizeInByte = nEncodedFrameSizeB + 1 + m_MyAudioHeadersize;				
			}
			else 
			{				
				nPacketType = AUDIO_LIVE_CALLEE_PACKET_TYPE;
				nEncodedFrameSizeB = CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short);
				memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saAudioRecorderFrame, nEncodedFrameSizeB);
				nFrameSizeInByte = 1 + m_MyAudioHeadersize + nEncodedFrameSizeB;				
			}
			
			MediaLog(LOG_DEBUG, "[ANEPV] FrameNo = %d, RT = %lld, PacketType = %d, EncodedSize = %d ,nSendingDataSizeInByte = %d", m_iPacketNumber, llRelativeTime, nPacketType, nEncodedFrameSizeB, nFrameSizeInByte);

			m_ucaRawFrameNonMuxed[0] = 0;
/*BuildAndGetHeaderInArray(nPacketType, m_MyAudioHeadersize, 0, m_iPacketNumber, nEncodedFrameSizeB,
				nChannel, version, llRelativeTime, m_pAudioCallSession->m_nEchoStateFlags, &m_ucaRawFrameNonMuxed[1]);*/

			BuildHeaderForLive(nPacketType, m_MyAudioHeadersize, version, m_iPacketNumber, nEncodedFrameSizeB,
				llRelativeTime, nEchoStateFlags, &m_ucaRawFrameNonMuxed[1]);


			++m_iPacketNumber;
			if (m_iPacketNumber == m_llMaxAudioPacketNumber)
			{
				m_iPacketNumber = 0;
			}
			
			if (m_pAudioCallSession->IsOpusEnable())
			{				
				StoreDataForChunk( m_ucaRawFrameNonMuxed, nFrameSizeInByte, nullptr, 0, llRelativeTime);
			}
			else 
			{
				StoreDataForChunk( m_ucaRawFrameNonMuxed, llRelativeTime, nFrameSizeInByte);
			}
			
			Tools::SOSleep(0);
		}
	}

} //namespace MediaSDK
