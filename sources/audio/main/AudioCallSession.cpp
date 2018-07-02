#include <sstream>
#include <string>

#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "Tools.h"
#include "LogPrinter.h"
#include "AudioMacros.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioDePacketizer.h"
#include "LiveAudioParser.h"
#include "AudioPacketHeader.h"
#include "AudioShortBufferForPublisherFarEnd.h"
#include "AudioNearEndDataProcessor.h"
#include "AudioFarEndDataProcessor.h"
#include "EchoCancellerProvider.h"
//#include "EchoCancellerInterface.h"
#include "NoiseReducerProvider.h"
#include "AudioGainInstanceProvider.h"
#include "AudioGainInterface.h"
#include "NoiseReducerInterface.h"
#include "NoiseReducerProvider.h"
#include "AudioEncoderProvider.h"
#include "AudioDecoderProvider.h"
#include "AudioEncoderInterface.h"
#include "AudioNearEndProcessorPublisher.h"
#include "AudioNearEndProcessorViewer.h"
#include "AudioNearEndProcessorCall.h"
#include "AudioNearEndProcessorThread.h"
#include "AudioFarEndProcessorPublisher.h"
#include "AudioFarEndProcessorViewer.h"
#include "AudioFarEndProcessorChannel.h"
#include "AudioFarEndProcessorCall.h"
#include "AudioFarEndProcessorThread.h"
#include "AudioEncoderBuffer.h"
#include "AudioResources.h"
#include "AudioDecoderBuffer.h"
#include "AudioLinearBuffer.h"
#include "AudioCallInfo.h"



#ifdef USE_VAD
#include "Voice.h"
#endif

#ifdef LOCAL_SERVER_LIVE_CALL
#define LOCAL_SERVER_IP "192.168.0.120"
#include "VideoSockets.h"
#endif

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define TI_NUMBER_OF_TRACE_RECVD 0
#define TI_NUMBER_OF_TRACE_FAILED 1
#define TI_SUM_OF_DELAY 2


namespace MediaSDK
{
	CAudioCallSession::CAudioCallSession(const bool& isVideoCallRunning, LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, int nEntityType, AudioResources &audioResources, bool bOpusCodec,
	AudioCallParams acParams
		) :
		m_bIsVideoCallRunning(isVideoCallRunning),
		m_nEntityType(nEntityType),
		m_nServiceType(nServiceType),
		m_llLastPlayTime(0),
		m_nCallInLiveType(CALL_IN_LIVE_TYPE_AUDIO_VIDEO),
		m_cNearEndProcessorThread(nullptr),
		m_cFarEndProcessorThread(nullptr),
		m_bIsOpusCodec(bOpusCodec)
	{
		m_bRecordingStarted = false;
		MediaLog(LOG_INFO, "\n[NE][ACS] AudioCallSession# Initialized. ServiceType=%d, EntityType=%d, Opus[%d]----------\n", nServiceType, nEntityType, (int)m_bIsOpusCodec);

		m_pAudioCallSessionMutex.reset(new CLockHandler);

		if (IsOpusEnable())
		{
			m_FarEndBufferOpus.reset(new CAudioByteBuffer);			
		}
		else 
		{
			m_PublisherBufferForMuxing.reset(new AudioShortBufferForPublisherFarEnd);
		}
		
		bool bIsCameraEnalbed = false;
		bool bIsMicrophoneEnabled = true;
		m_pAudioCallInfo = new CAudioCallInfo(bIsCameraEnalbed, bIsMicrophoneEnabled);

		m_FarendBuffer.reset(new CAudioShortBuffer);
		m_AudioNearEndBuffer.reset(new CAudioShortBuffer);
		m_ViewerInCallSentDataQueue.reset(new CAudioShortBuffer);

		SetResources(audioResources);
		if (GetPlayerGain().get())
		{
			GetPlayerGain()->Init(m_nServiceType);
			GetPlayerGain()->SetGain(0);
			//GetPlayerGain()->SetGain(10); //TODO: remove this
		}

		if (GetRecorderGain().get())
		{
			GetRecorderGain()->Init(m_nServiceType);
			if (m_nEntityType == ENTITY_TYPE_PUBLISHER && m_nServiceType == SERVICE_TYPE_LIVE_STREAM)
			{
				//Gain level is incremented to recover losses due to noise.
				//And noise is only applied to publisher NOT in call.
				GetRecorderGain()->SetGain(DEFAULT_GAIN + 1);
			}
		}

		m_iSpeakerType = acParams.nAudioSpeakerType;

		SetSendFunction(pSharedObject->GetSendFunctionPointer());
		SetEventNotifier(pSharedObject->m_pEventNotifier);

		m_FriendID = llFriendID;

		//m_pAudioDePacketizer = new AudioDePacketizer(this);
		m_iRole = nEntityType;
		m_bLiveAudioStreamRunning = false;


		if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
		{
			m_bLiveAudioStreamRunning = true;
		}

		//m_iPrevRecvdSlotID = -1;
		m_iReceivedPacketsInPrevSlot = AUDIO_SLOT_SIZE; //used by child

#ifdef USE_VAD
		m_pVoice = new CVoice();
#endif

		m_iAudioVersionFriend = -1;
		if (m_bLiveAudioStreamRunning)
		{
			m_iAudioVersionSelf = AUDIO_LIVE_VERSION;
		}
		else
		{
			m_iAudioVersionSelf = AUDIO_CALL_VERSION;
		}
#ifdef LOCAL_SERVER_LIVE_CALL
		m_clientSocket = VideoSockets::GetInstance();
		m_clientSocket->SetAudioCallSession(this);
#endif

		m_pChunckedNE = new CAudioDumper("RecordedChuncked.pcm", false);
		m_pPlayedFE = new CAudioDumper("Played.pcm", true);
		m_pPlayerSidePreGain = new CAudioDumper("Ppg.pcm", true);
		m_pPlayedPublisherFE = new CAudioDumper("PlayedPublisher.pcm", true);
		m_pPlayedCalleeFE = new CAudioDumper("PlayedCallee.pcm", true);



		InitNearEndDataProcessing();
		InitFarEndDataProcessing();

		if (!m_bLiveAudioStreamRunning)
		{
			m_pNearEndProcessor->SetEnableRecorderTimeSyncDuringEchoCancellation(true);
			m_bEnablePlayerTimeSyncDuringEchoCancellation = true;
		}
		else
		{
			m_pNearEndProcessor->SetEnableRecorderTimeSyncDuringEchoCancellation(true);
			m_bEnablePlayerTimeSyncDuringEchoCancellation = true;
		}

		m_pNearEndProcessor->SetNeedToResetAudioEffects(true);

		m_cNearEndProcessorThread = new AudioNearEndProcessorThread(m_pNearEndProcessor);
		if (m_cNearEndProcessorThread != nullptr)
		{
			m_cNearEndProcessorThread->StartNearEndThread();
		}

		m_cFarEndProcessorThread = new AudioFarEndProcessorThread(m_pFarEndProcessor);
		if (m_cFarEndProcessorThread != nullptr)
		{
			m_cFarEndProcessorThread->StartFarEndThread();
		}

		MediaLog(LOG_INFO, "[NE][ACS] AudioCallSession Initialization Successful!!, nAudioSpeakerType = %d, sManuName = %s, sModelName = %s, nSDKVersion = %d,
			bDeviceHasAEC = %d\n",
			acParams.nAudioSpeakerType, acParams.sManuName, acParams.sModelName, acParams.sOSVersion, acParams.nSDKVersion,
			acParams.bDeviceHasAEC);

		SetTraceInfo(acParams.nTraceInfoLength, acParams.npTraceInfo, acParams.bDeviceHasAEC);

		CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");
	}

	CAudioCallSession::~CAudioCallSession()
	{
		MediaLog(LOG_INFO, "[NE][ACS] AudioCallSession Uninitializating...");

		if (m_cNearEndProcessorThread != nullptr)
		{
			delete m_cNearEndProcessorThread;
			m_cNearEndProcessorThread = nullptr;
		}

		if (m_cFarEndProcessorThread != nullptr)
		{
			delete m_cFarEndProcessorThread;
			m_cFarEndProcessorThread = nullptr;
		}

		if (m_pNearEndProcessor)
		{
			delete m_pNearEndProcessor;
			m_pNearEndProcessor = NULL;
		}

		if (m_pFarEndProcessor)
		{
			delete m_pFarEndProcessor;
			m_pFarEndProcessor = NULL;
		}

#ifdef USE_VAD
		delete m_pVoice;
#endif

#ifdef DUMP_FILE
		fclose(FileOutput);
		fclose(FileInput);
		fclose(FileInputWithEcho);
		fclose(FileInputPreGain);
#endif

		if (m_pChunckedNE != nullptr)
		{
			delete m_pChunckedNE;
			m_pChunckedNE = nullptr;
		}
		if (m_pPlayedFE != nullptr)
		{
			delete m_pPlayedFE;
			m_pPlayedFE = nullptr;
		}
		if (m_pPlayerSidePreGain != nullptr)
		{
			delete m_pPlayerSidePreGain;
			m_pPlayerSidePreGain = nullptr;
		}

		if (m_pPlayedPublisherFE != nullptr)
		{
			delete m_pPlayedPublisherFE;
			m_pPlayedPublisherFE = nullptr;
		}

		if (m_pPlayedCalleeFE != nullptr)
		{
			delete m_pPlayedCalleeFE;
			m_pPlayedCalleeFE = nullptr;
		}

		if (nullptr != m_pAudioCallInfo)
		{
			delete m_pAudioCallInfo;
			m_pAudioCallInfo = nullptr;
		}

		SHARED_PTR_DELETE(m_pAudioCallSessionMutex);
		MediaLog(LOG_INFO, "[NE][ACS] AudioCallSession Uninitialization Successfull!!");
	}

	void CAudioCallSession::NotifyTraceInfo(int nTR, int nNTR, int sDelay)
	{
		MediaLog(LOG_DEBUG, "[NE][ACS][TP] NotifyTraceInfo called, nTR = %d, nNTR= %d, sDelay= %d", nTR, nNTR, sDelay);

		m_nNumTraceReceived += nTR;
		m_nNumTraceNotReceived += nNTR;
		m_nSumDelay += sDelay;

		int nTraceInfoArray[3];
		nTraceInfoArray[TI_NUMBER_OF_TRACE_RECVD] = m_nNumTraceReceived;
		nTraceInfoArray[TI_NUMBER_OF_TRACE_FAILED] = m_nNumTraceNotReceived;
		nTraceInfoArray[TI_SUM_OF_DELAY] = m_nSumDelay;

		FireAudioAlarm(AUDIO_EVENT_TRACE_NOTIFICATION, 3, nTraceInfoArray);
	}

	void CAudioCallSession::SetTraceInfo(int nTraceInfoLength, int * nTraceInfoArray, bool bDeviceHasAEC)
	{
		if (nTraceInfoLength < 3)
		{
			MediaLog(LOG_DEBUG, "[NE][ACS][TP] SetTraceInfo falied.");
			return;
		}
		MediaLog(LOG_DEBUG, "[NE][ACS][TP] SetTraceInfo started, nTraceInfoLength = %d", nTraceInfoLength);
		
		m_nNumTraceReceived = nTraceInfoArray[TI_NUMBER_OF_TRACE_RECVD];
		m_nNumTraceNotReceived = nTraceInfoArray[TI_NUMBER_OF_TRACE_FAILED];
		m_nSumDelay = nTraceInfoArray[TI_SUM_OF_DELAY];
		MediaLog(LOG_DEBUG, "[NE][ACS][TP] SetTraceInfo m_nNumTraceReceived = %d, m_nNumTraceNotReceived= %d, m_nSumDelay = %d",
			m_nNumTraceReceived, m_nNumTraceNotReceived, m_nSumDelay);
		if (m_nNumTraceReceived > 0)
		{
			m_fAvgDelay = m_nSumDelay * 1.0 / m_nNumTraceReceived;
			m_nAvgDelayFrames = m_fAvgDelay / 100;
			m_nAvgDelayFraction = (int)m_fAvgDelay % 100;
		}
		else
		{
			m_fAvgDelay = m_nAvgDelayFrames = m_nAvgDelayFraction = 0;
		}

		if (m_nNumTraceReceived + m_nNumTraceNotReceived > 0)
		{
			m_fTraceReceivingProbability = m_nNumTraceReceived * 1.0 / (m_nNumTraceReceived + m_nNumTraceNotReceived);
		}
		else
		{
			m_fTraceReceivingProbability = 0;
		}
		MediaLog(LOG_INFO, "[NE][ACS] SetTraceInfo successful 2.");
	}

	void CAudioCallSession::SetResources(AudioResources &audioResources)
	{
		MR_DEBUG("#resource# CAudioCallSession::SetResources()");

		m_pAudioNearEndPacketHeader = audioResources.GetNearEndPacketHeader();
		m_pAudioFarEndPacketHeader = audioResources.GetFarEndPacketHeader();

		m_pAudioEncoder = audioResources.GetEncoder();
		if (m_pAudioEncoder.get())
		{
			m_pAudioEncoder->CreateAudioEncoder();
			
			if (SERVICE_TYPE_LIVE_STREAM == m_nServiceType && IsOpusEnable())
			{
				m_pAudioEncoder->SetBitrate(OPUS_BITRATE_INIT_LIVE);
			}
		}

		m_pAudioDecoder = audioResources.GetDecoder();

		m_pEcho = audioResources.GetEchoCanceler();
		m_pNoiseReducer = audioResources.GetNoiseReducer();

		m_pPlayerGain = audioResources.GetPlayerGain();
		m_pRecorderGain = audioResources.GetRecorderGain();
	}


	void CAudioCallSession::InitNearEndDataProcessing()
	{
		MR_DEBUG("#nearEnd# CAudioCallSession::StartNearEndDataProcessing()");

		if (m_bLiveAudioStreamRunning)
		{
			if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
			{
				m_pNearEndProcessor = new AudioNearEndProcessorPublisher(m_nServiceType, m_nEntityType, this, m_AudioNearEndBuffer, m_bLiveAudioStreamRunning);
			}
			else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
			{
				m_pNearEndProcessor = new AudioNearEndProcessorViewer(m_nServiceType, m_nEntityType, this, m_AudioNearEndBuffer, m_bLiveAudioStreamRunning);
			}
		}
		else
		{
			m_pNearEndProcessor = new AudioNearEndProcessorCall(m_nServiceType, m_nEntityType, this, m_AudioNearEndBuffer, m_bLiveAudioStreamRunning, m_bIsVideoCallRunning);
		}

		m_pNearEndProcessor->SetDataReadyCallback(this);
		m_pNearEndProcessor->SetEventCallback(this, this);
	}


	void CAudioCallSession::InitFarEndDataProcessing()
	{
		MR_DEBUG("#farEnd# CAudioCallSession::StartFarEndDataProcessing()");

		if (SERVICE_TYPE_LIVE_STREAM == m_nServiceType || SERVICE_TYPE_SELF_STREAM == m_nServiceType)
		{
			if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)		//Is Viewer or Callee.
			{
				m_pFarEndProcessor = new FarEndProcessorViewer(m_nServiceType, m_nEntityType, this, m_bLiveAudioStreamRunning);
			}
			else if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
			{
				m_pFarEndProcessor = new FarEndProcessorPublisher(m_nServiceType, m_nEntityType, this, m_bLiveAudioStreamRunning);
			}
		}
		else if (SERVICE_TYPE_CHANNEL == m_nServiceType)
		{
			m_pFarEndProcessor = new FarEndProcessorChannel(m_nServiceType, m_nEntityType, this, m_bLiveAudioStreamRunning);
		}
		else if (SERVICE_TYPE_CALL == m_nServiceType || SERVICE_TYPE_SELF_CALL == m_nServiceType)
		{
			m_pFarEndProcessor = new FarEndProcessorCall(m_nServiceType, m_nEntityType, this, m_bLiveAudioStreamRunning);
		}

		m_pFarEndProcessor->SetEventCallback(this, this, this);
	}

	bool CAudioCallSession::SetAudioQuality(int level)
	{
		if (level < 1 || level > 3) return false;

		if (m_pAudioEncoder.get())
		{
			return m_pAudioEncoder->SetAudioQuality(level);
		}
		return false;
	}

	void CAudioCallSession::StartCallInLive(int iRole, int nCallInLiveType)
	{
		MediaLog(LOG_CODE_TRACE, "[NE][ACS] StartCallInLive Starting...");
		if (iRole != ENTITY_TYPE_VIEWER_CALLEE && iRole != ENTITY_TYPE_PUBLISHER_CALLER)//Unsupported or inaccessible role
		{
			MediaLog(LOG_ERROR, "[NE][ACS] StartCallInLive FAILED!!!Unsupported or inaccessible role.\n");
			return;
		}

		if (ENTITY_TYPE_PUBLISHER_CALLER == m_iRole || ENTITY_TYPE_VIEWER_CALLEE == m_iRole) //Call inside a call
		{
			MediaLog(LOG_WARNING, "[NE][ACS] StartCallInLive FAILED!!! Call inside call.\n");
			return;
		}

		m_bRecordingStarted = false;
		m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(true);
		while (m_pFarEndProcessor->m_pLiveAudioParser->IsParsingAudioData())
		{
			Tools::SOSleep(1);
		}

		if (GetRecorderGain().get() && GetNoiseReducer().get() && m_nEntityType == ENTITY_TYPE_PUBLISHER)
		{
			//Gain reseted down to default state. As noise suppression will NOT occur in live call.
			GetRecorderGain()->SetGain(DEFAULT_GAIN);
		}

		//LOGE("### Start call in live");
		m_nEntityType = iRole;
		m_iRole = iRole;

		if (m_iRole == ENTITY_TYPE_PUBLISHER_CALLER)
		{
#ifdef LOCAL_SERVER_LIVE_CALL
			m_clientSocket->InitializeSocket(LOCAL_SERVER_IP, 60001);
#endif
		}
		else if (m_iRole == ENTITY_TYPE_VIEWER_CALLEE)
		{
#ifdef LOCAL_SERVER_LIVE_CALL
			m_clientSocket->InitializeSocket(LOCAL_SERVER_IP, 60002);
#endif
		}

		m_pNearEndProcessor->ClearRecordBuffer();
		m_ViewerInCallSentDataQueue->ResetBuffer();
		m_pNearEndProcessor->StartCallInLive(m_nEntityType);
		m_pFarEndProcessor->StartCallInLive(m_nEntityType);

		Tools::SOSleep(20);

		m_pFarEndProcessor->m_llDecodingTimeStampOffset = -1;
		m_pFarEndProcessor->m_pAudioDePacketizer->ResetDepacketizer();

#ifdef DUMP_FILE
		if (m_iRole == ENTITY_TYPE_PUBLISHER_CALLER)
		{
			FileInputMuxed = fopen("/sdcard/InputPCMN_MUXED.pcm", "wb");
		}
#endif
		m_pNearEndProcessor->SetNeedToResetAudioEffects(true);
		m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(false);

		MediaLog(LOG_INFO, "\n\n[NE][ACS]!!!!!!!  StartCallInLive !!!!!!!!!\n\n");
	}

	void CAudioCallSession::EndCallInLive()
	{
		MediaLog(LOG_CODE_TRACE, "[NE][ACS]EndCallInLive Starting");

		if (m_iRole != ENTITY_TYPE_VIEWER_CALLEE && m_iRole != ENTITY_TYPE_PUBLISHER_CALLER)//Call Not Running
		{
			MediaLog(LOG_WARNING, "[NE][ACS] EndCallInLive FAILED!!!!!\n");
			return;
		}
		m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(true);
		m_pNearEndProcessor->SetNeedToResetAudioEffects(true);
		m_bRecordingStarted = false;
		while (m_pFarEndProcessor->m_pLiveAudioParser->IsParsingAudioData())
		{
			Tools::SOSleep(1);
		}

#ifdef DUMP_FILE
		if (m_iRole == ENTITY_TYPE_PUBLISHER_CALLER)
		{
			fclose(FileInputMuxed);
		}
#endif

		//m_pLiveAudioReceivedQueue->ResetBuffer();
		m_pFarEndProcessor->m_AudioReceivedBuffer->ResetBuffer();

		Tools::SOSleep(20);

		if (GetRecorderGain().get() && GetNoiseReducer().get() && m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER)
		{
			//Again going back to publisher NOT in call. So increasing gain level. 
			GetRecorderGain()->SetGain(DEFAULT_GAIN + 1);
		}

		if (ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
		{
			m_nEntityType = ENTITY_TYPE_PUBLISHER;
		}
		else if (ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			m_nEntityType = ENTITY_TYPE_VIEWER;
		}

		m_pNearEndProcessor->ClearRecordBuffer();

		m_iRole = m_nEntityType;

		m_pNearEndProcessor->StopCallInLive(m_nEntityType);
		m_pFarEndProcessor->StopCallInLive(m_nEntityType);

		m_pFarEndProcessor->m_llDecodingTimeStampOffset = -1;
		m_pFarEndProcessor->m_pAudioDePacketizer->ResetDepacketizer();
		m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(false);

		MediaLog(LOG_INFO, "\n\n[NE][ACS]!!!!!!!  EndCallInLive !!!!!!!!!\n\n");
	}

	SessionStatisticsInterface *CAudioCallSession::GetSessionStatListener()
	{
		return m_pNearEndProcessor->GetSessionStatListener();
	}

	void CAudioCallSession::SetCallInLiveType(int nCallInLiveType)
	{
		m_nCallInLiveType = nCallInLiveType;
	}

	long long CAudioCallSession::GetBaseOfRelativeTime()
	{
		return m_pNearEndProcessor->GetBaseOfRelativeTime();
	}	

	int CAudioCallSession::PushAudioData(short *psaEncodingAudioData, unsigned int unLength)
	{
		
		MediaLog(LOG_DEBUG, "[CL][NE] DataToMedia Size: %u", unLength);

		m_pNearEndProcessor->PushDataInRecordBuffer(psaEncodingAudioData, unLength);

		return 0;
	}


	void CAudioCallSession::SetVolume(int iVolume, bool bRecorder)
	{
		if (GetPlayerGain().get())
		{
			GetPlayerGain()->SetGain(iVolume);
		}
	}

	void CAudioCallSession::SetSpeakerType(AudioCallParams acParams)
	{
		m_bRecordingStarted = false;
		//if (m_iSpeakerType != iSpeakerType)
		{
			m_pNearEndProcessor->ClearRecordBuffer();
			m_pNearEndProcessor->SetNeedToResetAudioEffects(true);
		}
		m_iSpeakerType = acParams.nAudioSpeakerType;
		SetTraceInfo(acParams.nTraceInfoLength, acParams.npTraceInfo, acParams.bDeviceHasAEC);
	}

	void CAudioCallSession::SetCameraMode(bool bCameraEnable)
	{
		m_pAudioCallInfo->SetCameraMode(bCameraEnable);
	}

	void CAudioCallSession::SetMicrophoneMode(bool bMicrophoneEnable)
	{
		m_pAudioCallInfo->SetMicrophoneMode(bMicrophoneEnable);
	}

	int CAudioCallSession::DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
	{
		return m_pFarEndProcessor->DecodeAudioData(nOffset, pucaDecodingAudioData, unLength, numberOfFrames, frameSizes, vMissingFrames);
	}

#ifdef FIRE_ENC_TIME
	int encodingtimetimes = 0, cumulitiveenctime = 0;
#endif

	void CAudioCallSession::DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize)
	{
		m_pFarEndProcessor->DumpDecodedFrame(psDecodedFrame, nDecodedFrameSize);
	}


	void CAudioCallSession::SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llNow, long long &llLastTime, int iCurrentPacketNumber, int nEchoStateFlags)
	{
		m_pFarEndProcessor->SendToPlayer(pshSentFrame, nSentFrameSize, llLastTime, iCurrentPacketNumber, nEchoStateFlags);
	}


	void CAudioCallSession::GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime)
	{
		m_pNearEndProcessor->GetAudioDataToSend(pAudioCombinedDataToSend, CombinedLength, vCombinedDataLengthVector, sendingLengthViewer, sendingLengthPeer, llAudioChunkDuration, llAudioChunkRelativeTime);
	}

	unsigned int CAudioCallSession::GetNumberOfFrameForChunk()
	{
		return m_pNearEndProcessor->GetNumberOfFrameForChunk();
	}


	void CAudioCallSession::OnDataReadyToSend(int mediaType, unsigned char* dataBuffer, size_t dataLength)
	{
		m_cbClientSendFunction(CAudioCallSession::m_FriendID, mediaType, dataBuffer, dataLength, 0, std::vector< std::pair<int, int> >());
	}

	void CAudioCallSession::FirePacketEvent(int eventType, size_t dataLength, unsigned char* dataBuffer)
	{
		m_pEventNotifier->fireAudioPacketEvent(eventType, dataLength, dataBuffer);
	}

	void CAudioCallSession::FireDataEvent(int eventType, size_t dataLength, short* dataBuffer)
	{
		m_pEventNotifier->fireAudioEvent(m_FriendID, eventType, dataLength, dataBuffer);
	}

	void CAudioCallSession::FireNetworkChange(int eventType)
	{
		m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_FriendID, eventType);
	}

	void CAudioCallSession::FireAudioAlarm(int eventType, size_t dataLength, int* dataBuffer)
	{
		m_pEventNotifier->fireAudioAlarm(eventType, dataLength, dataBuffer);
	}

} //namespace MediaSDK
