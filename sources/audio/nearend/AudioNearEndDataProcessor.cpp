
#include "AudioNearEndDataProcessor.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioMacros.h"
#include "AudioPacketHeader.h"
#include "AudioHeaderLive.h"
#include "CommonElementsBucket.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioPacketizer.h"
#include "AudioCallSession.h"
#include "AudioMixer.h"
#include "MuxHeader.h"
#include "AudioShortBufferForPublisherFarEnd.h"

#include "EncoderOpus.h"
#include "EncoderPCM.h"
#include "AudioEncoderInterface.h"
#include "NoiseReducerInterface.h"
#include "AudioGainInterface.h"
#include "AudioHeaderCall.h"


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

namespace MediaSDK
{

	AudioNearEndDataProcessor::AudioNearEndDataProcessor(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioNearEndBuffer, bool bIsLiveStreamingRunning) :
		m_nServiceType(nServiceType),
		m_nEntityType(nEntityType),
		m_bIsReady(false),
		m_pAudioCallSession(pAudioCallSession),
		m_pAudioNearEndBuffer(pAudioNearEndBuffer),
		m_bIsLiveStreamingRunning(bIsLiveStreamingRunning),
		m_bAudioEncodingThreadRunning(false),
		m_bAudioEncodingThreadClosed(true),
		m_iPacketNumber(0),
		m_nStoredDataLengthNear(0),
		m_nStoredDataLengthFar(0),
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

		m_llMaxAudioPacketNumber = (m_pAudioNearEndPacketHeader->GetFieldCapacity(INF_CALL_PACKETNUMBER) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;

		m_MyAudioHeadersize = m_pAudioNearEndPacketHeader->GetHeaderSize();
		m_llEncodingTimeStampOffset = Tools::CurrentTimestamp();
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
		if (m_pAudioNearEndPacketHeader)
		{
			//delete m_pAudioPacketHeader;
		}

	}

	void AudioNearEndDataProcessor::StoreDataForChunkDeviceInformation(unsigned char *uchDataToChunk, long long llRelativeTime, int nFrameLengthInByte)
	{
		NearEndLockerStoreDataForChunk lock(*m_pAudioEncodingMutex);


		if (0 == m_nStoredDataLengthNear && -1 == m_llLastChunkLastFrameRT)
		{
			HITLER("#RT# update lastChunkLastFrame time %lld", llRelativeTime);
			m_llLastChunkLastFrameRT = max(0LL, llRelativeTime - 100);
		}

		m_llLastFrameRT = llRelativeTime;

		if ((m_nStoredDataLengthNear + nFrameLengthInByte) < MAX_AUDIO_DATA_TO_SEND_SIZE)
		{
			memcpy(m_ucaRawDataToSendNear + m_nStoredDataLengthNear, uchDataToChunk, nFrameLengthInByte);
			m_nStoredDataLengthNear += (nFrameLengthInByte);
			m_vRawFrameLengthNear.push_back(nFrameLengthInByte);
		}
	}

	void AudioNearEndDataProcessor::StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nFrameLengthInByte)
	{
		NearEndLockerStoreDataForChunk lock(*m_pAudioEncodingMutex);


		if (0 == m_nStoredDataLengthNear && -1 == m_llLastChunkLastFrameRT)
		{
			HITLER("#RT# update lastChunkLastFrame time %lld", llRelativeTime);
			m_llLastChunkLastFrameRT = max(0LL, llRelativeTime-100);
		}

		m_llLastFrameRT = llRelativeTime;

		if ((m_nStoredDataLengthNear + nFrameLengthInByte) < MAX_AUDIO_DATA_TO_SEND_SIZE)
		{
			memcpy(m_ucaRawDataToSendNear + m_nStoredDataLengthNear, uchDataToChunk, nFrameLengthInByte);
			m_nStoredDataLengthNear += (nFrameLengthInByte);
			m_vRawFrameLengthNear.push_back(nFrameLengthInByte);
		}
	}

	void AudioNearEndDataProcessor::StoreDataForChunk(unsigned char *uchNearData, int nNearFrameLengthInByte, 
		unsigned char *uchFarData, int nFarFrameLengthInByte, long long llRelativeTime)
	{
		NearEndLockerStoreDataForChunk lock(*m_pAudioEncodingMutex);

		if (0 == m_nStoredDataLengthNear && -1 == m_llLastChunkLastFrameRT)
		{
			HITLER("#RT# update lastChunkLastFrame time %lld", llRelativeTime);
			m_llLastChunkLastFrameRT = max(0LL, llRelativeTime - 100);
		}

		m_llLastFrameRT = llRelativeTime;

		MediaLog(LOG_CODE_TRACE, "[ANEDP] m_nStoredDataLengthNear = %d[%d], m_nStoredDataLengthFar = %d[%d]", m_nStoredDataLengthNear, nNearFrameLengthInByte, m_nStoredDataLengthFar, nFarFrameLengthInByte);
		
		if ((m_nStoredDataLengthNear + nNearFrameLengthInByte) < MAX_AUDIO_DATA_TO_SEND_SIZE)
		{
			memcpy(m_ucaRawDataToSendNear + m_nStoredDataLengthNear, uchNearData, nNearFrameLengthInByte);
			m_nStoredDataLengthNear += (nNearFrameLengthInByte);
			m_vRawFrameLengthNear.push_back(nNearFrameLengthInByte);
		}

		if (nFarFrameLengthInByte > 0 && (m_nStoredDataLengthFar + nFarFrameLengthInByte) < MAX_AUDIO_DATA_TO_SEND_SIZE)
		{
			memcpy(m_ucaRawDataToSendFar + m_nStoredDataLengthFar, uchFarData, nFarFrameLengthInByte);
			m_nStoredDataLengthFar += (nFarFrameLengthInByte);
			m_vRawFrameLengthFar.push_back(nFarFrameLengthInByte);
		}
	}


	void AudioNearEndDataProcessor::BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int packetNumber, int packetLength,
		int channel, int version, long long timestamp, int echoStateFlags, unsigned char* header)
	{
		MediaLog(LOG_DEBUG, "[ECHOFLAG] BuildAndGetHeader ptype %d ntype %d  packetnumber %d plength %d  channel %d version %d time %lld echoStateFlags = %d",
			packetType, networkType, packetNumber, packetLength, channel, version, timestamp, echoStateFlags);

		m_pAudioNearEndPacketHeader->SetInformation(packetType, INF_CALL_PACKETTYPE);
		m_pAudioNearEndPacketHeader->SetInformation(nHeaderLength, INF_CALL_HEADERLENGTH);
		m_pAudioNearEndPacketHeader->SetInformation(packetNumber, INF_CALL_PACKETNUMBER);
		m_pAudioNearEndPacketHeader->SetInformation(packetLength, INF_CALL_BLOCK_LENGTH);
		m_pAudioNearEndPacketHeader->SetInformation(version, INF_CALL_VERSIONCODE);
		m_pAudioNearEndPacketHeader->SetInformation(timestamp, INF_CALL_TIMESTAMP);
		m_pAudioNearEndPacketHeader->SetInformation(networkType, INF_CALL_NETWORKTYPE);
		m_pAudioNearEndPacketHeader->SetInformation(channel, INF_CALL_CHANNELS);

		m_pAudioNearEndPacketHeader->SetInformation(0, INF_CALL_PACKET_BLOCK_NUMBER);
		m_pAudioNearEndPacketHeader->SetInformation(1, INF_CALL_TOTAL_PACKET_BLOCKS);
		m_pAudioNearEndPacketHeader->SetInformation(0, INF_CALL_BLOCK_OFFSET);
		m_pAudioNearEndPacketHeader->SetInformation(packetLength, INF_CALL_FRAME_LENGTH);
		m_pAudioNearEndPacketHeader->SetInformation(echoStateFlags, INF_CALL_ECHO_STATE_FLAGS);
		MediaLog(LOG_DEBUG, "[ECHOFLAG] setting to header echoStateFlags = %d\n", echoStateFlags);

		m_pAudioNearEndPacketHeader->ShowDetails("[ECHOFLAG] setting");

		m_pAudioNearEndPacketHeader->GetHeaderInByteArray(header);
	}



	void AudioNearEndDataProcessor::BuildHeaderForLive(int nPacketType, int nHeaderLength, int nVersion, int nPacketNumber, int nPacketLength,
		long long llRelativeTime, int nEchoStateFlags, unsigned char* ucpHeader)
	{
		MediaLog(LOG_DEBUG, "[ANEDP][ECHOFLAG] BuildHeaderForLive PT=%d HL=%d V=%d PN=%d PL=%d RTS=%lld ESF=%d",
			nPacketType, nHeaderLength, nVersion, nPacketLength, nPacketLength, llRelativeTime, nEchoStateFlags);

		m_pAudioNearEndPacketHeader->SetInformation(nPacketType, INF_LIVE_PACKETTYPE);
		m_pAudioNearEndPacketHeader->SetInformation(nHeaderLength, INF_LIVE_HEADERLENGTH);
		m_pAudioNearEndPacketHeader->SetInformation(nVersion, INF_LIVE_VERSIONCODE);
		m_pAudioNearEndPacketHeader->SetInformation(nPacketNumber, INF_LIVE_PACKETNUMBER);
		m_pAudioNearEndPacketHeader->SetInformation(nPacketLength, INF_LIVE_FRAME_LENGTH);
		m_pAudioNearEndPacketHeader->SetInformation(llRelativeTime, INF_LIVE_TIMESTAMP);
		m_pAudioNearEndPacketHeader->SetInformation(nEchoStateFlags, INF_LIVE_ECHO_STATE_FLAGS);
					
		MediaLog(LOG_DEBUG, "[ANEDP][ECHOFLAG] setting to header nEchoStateFlags = %d\n", nEchoStateFlags);

		m_pAudioNearEndPacketHeader->ShowDetails("[ANEDP] BuildHeaderForLive");

		m_pAudioNearEndPacketHeader->GetHeaderInByteArray(ucpHeader);
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

	unsigned int AudioNearEndDataProcessor::GetNumberOfFrameForChunk()
	{
		NearEndLockerGetAudioDataToSend lock(*m_pAudioEncodingMutex);
		return m_vRawFrameLengthNear.size();
	}

	void AudioNearEndDataProcessor::GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &nDataLengthNear, int &nDataLengthFar, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime)
	{
		NearEndLockerGetAudioDataToSend lock(*m_pAudioEncodingMutex);

		vCombinedDataLengthVector.clear();
		CombinedLength = 0;
		nDataLengthNear = 0;
		nDataLengthFar = 0;
		llAudioChunkDuration = 0;
		llAudioChunkRelativeTime = -1;

		if (-1 == m_llLastChunkLastFrameRT)
		{
			return;
		}

		MediaLog(LOG_CODE_TRACE,"[ANEDP] lastFrameRT: %lld, lastChunkLastFrameRT: %lld", m_llLastFrameRT, m_llLastChunkLastFrameRT);

		llAudioChunkDuration = m_llLastFrameRT - m_llLastChunkLastFrameRT;

		MediaLog(LOG_DEBUG, "[ANEDP] [007] Audio Chunk Duration: %lld", llAudioChunkDuration);

		if (0 == llAudioChunkDuration)
		{
			return;
		}

		llAudioChunkRelativeTime = m_llLastChunkLastFrameRT;
		m_llLastChunkLastFrameRT = m_llLastFrameRT;
		
		/*  COPY NEAR_END DATA */
		vCombinedDataLengthVector = m_vRawFrameLengthNear;
		memcpy(pAudioCombinedDataToSend, m_ucaRawDataToSendNear, m_nStoredDataLengthNear); 
		CombinedLength += m_nStoredDataLengthNear;
		nDataLengthNear = m_nStoredDataLengthNear;

		/*  COPY FAR_END DATA */
		if (0 < m_nStoredDataLengthFar)
		{
			vCombinedDataLengthVector.insert(std::end(vCombinedDataLengthVector), std::begin(m_vRawFrameLengthFar), std::end(m_vRawFrameLengthFar));
			memcpy(pAudioCombinedDataToSend + m_nStoredDataLengthNear, m_ucaRawDataToSendFar, m_nStoredDataLengthFar);
			CombinedLength += m_nStoredDataLengthFar;
			nDataLengthFar = m_nStoredDataLengthFar;
		}

		int nFrames = vCombinedDataLengthVector.size();
		int nFramesNear = m_vRawFrameLengthNear.size();
		int nFramesFar = m_vRawFrameLengthFar.size();

		MediaLog(LOG_DEBUG, "[ANEDP] RelativeTime:%lld [Dur:%lld], NearData=%d, FarData=%d, TotalData=%d FramesTotal=%d[N:%d,F:%d]",
			llAudioChunkRelativeTime, llAudioChunkDuration, nDataLengthNear, nDataLengthFar, CombinedLength, nFrames, nFramesNear, nFramesFar);

		m_nStoredDataLengthNear = 0;
		m_nStoredDataLengthFar = 0;
		m_vRawFrameLengthNear.clear();
		m_vRawFrameLengthFar.clear();
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


	void AudioNearEndDataProcessor::StartCallInLive(int nEntityType)
	{
		if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			NearEndLockerGetAudioDataToSend lock(*m_pAudioEncodingMutex);
			m_llLastChunkLastFrameRT = -1;
			m_nStoredDataLengthNear = 0;
			m_nStoredDataLengthFar = 0;
			m_vRawFrameLengthNear.clear();
			m_vRawFrameLengthFar.clear();
		}
		m_nEntityType = nEntityType;
	}


	void AudioNearEndDataProcessor::StopCallInLive(int nEntityType)
	{
		if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			NearEndLockerGetAudioDataToSend lock(*m_pAudioEncodingMutex);
			m_llLastChunkLastFrameRT = -1;
			m_nStoredDataLengthNear = 0;
			m_nStoredDataLengthFar = 0;
			m_vRawFrameLengthNear.clear();
			m_vRawFrameLengthFar.clear();
		}
		m_nEntityType = nEntityType;
	}

	long long AudioNearEndDataProcessor::GetBaseOfRelativeTime()
	{
		return m_llEncodingTimeStampOffset;
	}

} //namespace MediaSDK
