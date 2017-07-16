
#include "AudioNearEndDataProcessor.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioMacros.h"
#include "AudioPacketHeader.h"
#include "CommonElementsBucket.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioPacketizer.h"
#include "AudioCallSession.h"
#include "AudioMixer.h"
#include "MuxHeader.h"

#include "EncoderOpus.h"
#include "AudioEncoderInterface.h"
#include "NoiseReducerInterface.h"
#include "AudioGainInterface.h"


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

namespace MediaSDK
{

	AudioNearEndDataProcessor::AudioNearEndDataProcessor(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CAudioShortBuffer *pAudioNearEndBuffer, bool bIsLiveStreamingRunning) :
		m_nServiceType(nServiceType),
		m_nEntityType(nEntityType),
		m_bIsReady(false),
		m_pAudioCallSession(pAudioCallSession),
		m_pAudioNearEndBuffer(pAudioNearEndBuffer),
		m_bIsLiveStreamingRunning(bIsLiveStreamingRunning),
		m_bAudioEncodingThreadRunning(false),
		m_bAudioEncodingThreadClosed(true),
		m_nEncodedFrameSize(0),
		m_iPacketNumber(0),
		m_iRawDataSendIndexViewer(0),
		m_llLastChunkLastFrameRT(-1),
		m_llLastFrameRT(0),
		m_pDataReadyListener(nullptr),
		m_pPacketEventListener(nullptr)
	{
		m_pAudioEncodingMutex.reset(new CLockHandler);
		m_pAudioEncoder = pAudioCallSession->GetAudioEncoder();

		//TODO: We shall remove the AudioSession instance from Near End 
		//and shall pass necessary objects to it, e.g. Codec, Noise, Gain
		//	m_pNoise = m_pAudioCallSession->GetNoiseReducer();

		m_pAudioNearEndPacketHeader = pAudioCallSession->GetAudioNearEndPacketHeader();

		m_llMaxAudioPacketNumber = (m_pAudioNearEndPacketHeader->GetFieldCapacity(INF_PACKETNUMBER) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;

		m_MyAudioHeadersize = m_pAudioNearEndPacketHeader->GetHeaderSize();
		m_llEncodingTimeStampOffset = Tools::CurrentTimestamp();

		m_pAudioMixer = new AudioMixer(BITS_USED_FOR_AUDIO_MIXING, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);

		m_bIsReady = true;

#ifdef DUMP_FILE
		m_pAudioCallSession->FileInput = fopen("/sdcard/InputPCMN.pcm", "wb");
		m_pAudioCallSession->FileInputWithEcho = fopen("/sdcard/InputPCMN_WITH_ECHO.pcm", "wb");
		m_pAudioCallSession->FileInputPreGain = fopen("/sdcard/InputPCMNPreGain.pcm", "wb");	
		m_pAudioCallSession->File18BitType = fopen("/sdcard/File18BitType.pcm", "wb");
		m_pAudioCallSession->File18BitData = fopen("/sdcard/File18BitData.pcm", "wb");
#endif	

	}

	AudioNearEndDataProcessor::~AudioNearEndDataProcessor()
	{
		if (m_pAudioMixer)
		{
			delete m_pAudioMixer;
			m_pAudioMixer = nullptr;
		}
		if (m_pAudioNearEndPacketHeader)
		{
			//delete m_pAudioPacketHeader;
		}

	}


	void AudioNearEndDataProcessor::StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nDataLengthInByte)
	{
		NearEndLockerStoreDataForChunk lock(*m_pAudioEncodingMutex);


		if (0 == m_iRawDataSendIndexViewer && -1 == m_llLastChunkLastFrameRT)
		{
			HITLER("#RT# update lastChunkLastFrame time %lld", llRelativeTime);
			m_llLastChunkLastFrameRT = llRelativeTime;
		}

		m_llLastFrameRT = llRelativeTime;

		if ((m_iRawDataSendIndexViewer + nDataLengthInByte + m_MyAudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE)
		{
			memcpy(m_ucaRawDataToSendViewer + m_iRawDataSendIndexViewer, uchDataToChunk, nDataLengthInByte + m_MyAudioHeadersize + 1);
			m_iRawDataSendIndexViewer += (nDataLengthInByte + m_MyAudioHeadersize + 1);
			m_vRawFrameLengthViewer.push_back(nDataLengthInByte + m_MyAudioHeadersize + 1);
		}
	}


	void AudioNearEndDataProcessor::SentToNetwork(long long llRelativeTime)
	{
#ifdef  AUDIO_SELF_CALL //Todo: build while this is enable
		//Todo: m_AudioReceivedBuffer fix. not member of this class
		if (m_bIsLiveStreamingRunning == false)
		{
			ALOG("#A#EN#--->> Self#  PacketNumber = " + Tools::IntegertoStringConvert(m_iPacketNumber));
			m_pAudioCallSession->m_pFarEndProcessor->m_AudioReceivedBuffer.EnQueue(m_ucaEncodedFrame + 1, m_nEncodedFrameSize + m_MyAudioHeadersize);
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
		if(m_pPacketEventListener != nullptr)
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
			if(m_pPacketEventListener != nullptr)
			{
				m_pPacketEventListener->FirePacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
			}
#endif
		}
#endif
	}

	void AudioNearEndDataProcessor::BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header)
	{
		//LOGEF("##EN### BuildAndGetHeader ptype %d ntype %d slotnumber %d packetnumber %d plength %d reslnumber %d npacrecv %d channel %d version %d time %lld",
		//	packetType, networkType, slotNumber, packetNumber, packetLength, recvSlotNumber, numPacketRecv, channel, version, timestamp);

		m_pAudioNearEndPacketHeader->SetInformation(packetType, INF_PACKETTYPE);
		m_pAudioNearEndPacketHeader->SetInformation(nHeaderLength, INF_HEADERLENGTH);
		m_pAudioNearEndPacketHeader->SetInformation(packetNumber, INF_PACKETNUMBER);
		m_pAudioNearEndPacketHeader->SetInformation(slotNumber, INF_SLOTNUMBER);
		m_pAudioNearEndPacketHeader->SetInformation(packetLength, INF_BLOCK_LENGTH);
		m_pAudioNearEndPacketHeader->SetInformation(recvSlotNumber, INF_RECVDSLOTNUMBER);
		m_pAudioNearEndPacketHeader->SetInformation(numPacketRecv, INF_NUMPACKETRECVD);
		m_pAudioNearEndPacketHeader->SetInformation(version, INF_VERSIONCODE);
		m_pAudioNearEndPacketHeader->SetInformation(timestamp, INF_TIMESTAMP);
		m_pAudioNearEndPacketHeader->SetInformation(networkType, INF_NETWORKTYPE);
		m_pAudioNearEndPacketHeader->SetInformation(channel, INF_CHANNELS);

		m_pAudioNearEndPacketHeader->SetInformation(0, INF_PACKET_BLOCK_NUMBER);
		m_pAudioNearEndPacketHeader->SetInformation(1, INF_TOTAL_PACKET_BLOCKS);
		m_pAudioNearEndPacketHeader->SetInformation(0, INF_BLOCK_OFFSET);
		m_pAudioNearEndPacketHeader->SetInformation(packetLength, INF_FRAME_LENGTH);

		m_pAudioNearEndPacketHeader->showDetails("@#BUILD");

		m_pAudioNearEndPacketHeader->GetHeaderInByteArray(header);
	}

	bool AudioNearEndDataProcessor::PreProcessAudioBeforeEncoding()
	{
		//if (!m_bIsLiveStreamingRunning)
		{
#ifdef USE_VAD			
			if (!m_pVoice->HasVoice(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning)))
			{
				return false;
			}
#endif



			//if (m_pNoise.get())
			//{
			//	m_pNoise->Denoise(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), m_saAudioRecorderFrame, m_bIsLiveStreamingRunning);
			//}

		}
		return true;
	}

	void AudioNearEndDataProcessor::GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime)
	{
		NearEndLockerGetAudioDataToSend lock(*m_pAudioEncodingMutex);

		vCombinedDataLengthVector.clear();
		CombinedLength = 0;
		sendingLengthViewer = 0;
		sendingLengthPeer = 0;
		llAudioChunkDuration = 0;
		llAudioChunkRelativeTime = -1;

		if (-1 == m_llLastChunkLastFrameRT)
		{
			return;
		}

		//	HITLER("#RT# lastFrameRT: %lld, lastChunkLastFrameRT: %lld", m_llLastFrameRT, m_llLastChunkLastFrameRT);

		llAudioChunkDuration = m_llLastFrameRT - m_llLastChunkLastFrameRT;

		if (0 == llAudioChunkDuration)
		{
			return;
		}

		llAudioChunkRelativeTime = m_llLastChunkLastFrameRT;
		m_llLastChunkLastFrameRT = m_llLastFrameRT;


		vCombinedDataLengthVector = m_vRawFrameLengthViewer;
		memcpy(pAudioCombinedDataToSend, m_ucaRawDataToSendViewer, m_iRawDataSendIndexViewer);
		CombinedLength += m_iRawDataSendIndexViewer;
		sendingLengthViewer = m_iRawDataSendIndexViewer;

		m_iRawDataSendIndexViewer = 0;
		m_vRawFrameLengthViewer.clear();
	}

	void AudioNearEndDataProcessor::UpdateRelativeTimeAndFrame(long long &llLasstTime, long long & llRelativeTime, long long & llCapturedTime)
	{
		llLasstTime = llCapturedTime;
		llRelativeTime = llCapturedTime - m_llEncodingTimeStampOffset;
	}


	void AudioNearEndDataProcessor::DumpEncodingFrame()
	{
#ifdef DUMP_FILE
		fwrite(m_saAudioRecorderFrame, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), m_pAudioCallSession->FileInput);
#endif
	}


	bool AudioNearEndDataProcessor::MuxIfNeeded(short* shPublisherData, short *shMuxedData, int &nDataSizeInByte, int nPacketNumber)
	{
		long long nFrameNumber;
		nDataSizeInByte = 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING;
		int nLastDecodedFrameSizeInByte = 0;
		bool bIsMuxed = false;
		int iDataStartIndex, iDataEndIndex;
		int iCallId = 0, nNumberOfBlocks;
		std::vector<std::pair<int, int> > vMissingBlocks;
		MuxHeader oCalleeMuxHeader;
		if (m_pAudioCallSession->m_PublisherBufferForMuxing.GetQueueSize() != 0)
		{
			bIsMuxed = true;
			nLastDecodedFrameSizeInByte = m_pAudioCallSession->m_PublisherBufferForMuxing.DeQueue(m_saAudioPrevDecodedFrame, nFrameNumber, oCalleeMuxHeader) * 2;	//#Magic
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
		AudioMixer* DumpAudioMixer = new AudioMixer(18,800);
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


	void AudioNearEndDataProcessor::StartCallInLive(int nEntityType)
	{
		if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			m_llLastChunkLastFrameRT = -1;
			m_iRawDataSendIndexViewer = 0;
		}
		m_nEntityType = nEntityType;
	}

	void AudioNearEndDataProcessor::StopCallInLive(int nEntityType)
	{
		if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			m_llLastChunkLastFrameRT = -1;
			m_iRawDataSendIndexViewer = 0;
		}
		m_nEntityType = nEntityType;
	}

	long long AudioNearEndDataProcessor::GetBaseOfRelativeTime()
	{
		return m_llEncodingTimeStampOffset;
	}

	void AudioNearEndDataProcessor::DecideToChangeComplexity(int iEncodingTime)
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
