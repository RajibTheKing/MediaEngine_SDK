#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioDePacketizer.h"
#include "LiveAudioParser.h"

#include "EchoCancellerProvider.h"
#include "EchoCancellerInterface.h"

#ifdef LOCAL_SERVER_LIVE_CALL
#define LOCAL_SERVER_IP "192.168.0.120"
#endif

#define DUPLICATE_AUDIO

#include "NoiseReducerProvider.h"

#include "AudioGainInstanceProvider.h"
#include "AudioGainInterface.h"

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

#ifdef USE_VAD
#include "Voice.h"
#endif

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CEventNotifier* CAudioCallSession::m_pEventNotifier = nullptr;
SendFunctionPointerType CAudioCallSession::m_cbClientSendFunction = nullptr;
LongLong CAudioCallSession::m_FriendID = -1;

CAudioCallSession::CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, int nEntityType, AudioResources &audioResources) :
m_nEntityType(nEntityType),
m_nServiceType(nServiceType),
m_llLastPlayTime(0),
m_bIsAECMFarEndThreadBusy(false),
m_bIsAECMNearEndThreadBusy(false),
m_nCallInLiveType(CALL_IN_LIVE_TYPE_AUDIO_VIDEO),
m_bIsPublisher(true),
m_AudioEncodingBuffer(AUDIO_ENCODING_BUFFER_SIZE), 
m_cNearEndProcessorThread(nullptr),
m_cFarEndProcessorThread(nullptr)
{
	SetResources(audioResources);

	m_FriendID = llFriendID;

	//m_pAudioDePacketizer = new AudioDePacketizer(this);
	m_iRole = nEntityType;
	m_bLiveAudioStreamRunning = false;

	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
	{
		m_bLiveAudioStreamRunning = true;
	}

	m_pAudioCallSessionMutex.reset(new CLockHandler);

	m_iPrevRecvdSlotID = -1;
	m_iReceivedPacketsInPrevSlot = AUDIO_SLOT_SIZE; //used by child
	m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;

	m_bUsingLoudSpeaker = false;
	m_bEchoCancellerEnabled = true;

	if(m_bLiveAudioStreamRunning)
	{
		//m_bEchoCancellerEnabled = false;
	}

#ifdef USE_VAD
	m_pVoice = new CVoice();
#endif
	
	m_iAudioVersionFriend = -1;
	if(m_bLiveAudioStreamRunning)
	{
		m_iAudioVersionSelf = AUDIO_LIVE_VERSION;
	}
	else {
		m_iAudioVersionSelf = AUDIO_CALL_VERSION;
	}
#ifdef LOCAL_SERVER_LIVE_CALL
	m_clientSocket = VideoSockets::GetInstance();
	m_clientSocket->SetAudioCallSession(this);
#endif

	StartNearEndDataProcessing();
	StartFarEndDataProcessing(pSharedObject);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");
}

CAudioCallSession::~CAudioCallSession()
{
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

	SHARED_PTR_DELETE(m_pAudioCallSessionMutex);
}


void CAudioCallSession::SetResources(AudioResources &audioResources)
{
	m_pAudioHeader = audioResources.GetPacketHeader();

	m_pAudioEncoder = audioResources.GetEncoder();
	if (m_pAudioEncoder.get())
	{
		m_pAudioEncoder->CreateAudioEncoder();
	}

	m_pAudioDecoder = audioResources.GetDecoder();

	m_pEcho = audioResources.GetEchoCanceler();
	m_pNoiseReducer = audioResources.GetNoiseReducer();

	m_pRecorderGain = audioResources.GetRecorderGain();
	m_pPlayerGain = audioResources.GetPlayerGain();
}


void CAudioCallSession::StartNearEndDataProcessing()
{
	MR_DEBUG("#farEnd# CAudioCallSession::StartNearEndDataProcessing()");

	if (m_bLiveAudioStreamRunning)
	{
		if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
		{
			m_pNearEndProcessor = new AudioNearEndProcessorPublisher(m_nServiceType, m_nEntityType, this, &m_AudioEncodingBuffer, m_bLiveAudioStreamRunning);
		}
		else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			m_pNearEndProcessor = new AudioNearEndProcessorViewer(m_nServiceType, m_nEntityType, this, &m_AudioEncodingBuffer, m_bLiveAudioStreamRunning);
		}
	}
	else
	{
		m_pNearEndProcessor = new AudioNearEndProcessorCall(m_nServiceType, m_nEntityType, this, &m_AudioEncodingBuffer, m_bLiveAudioStreamRunning);
	}

	m_pNearEndProcessor->SetDataReadyCallback((OnDataReadyToSendCB)OnDataReadyCallback);
	m_pNearEndProcessor->SetEventCallback((OnFirePacketEventCB)OnPacketEventCallback);

	m_cNearEndProcessorThread = new AudioNearEndProcessorThread(m_pNearEndProcessor);
	if (m_cNearEndProcessorThread != nullptr)
	{
		m_cNearEndProcessorThread->StartNearEndThread();
	}
}


void CAudioCallSession::StartFarEndDataProcessing(CCommonElementsBucket* pSharedObject)
{
	MR_DEBUG("#farEnd# CAudioCallSession::StartFarEndDataProcessing()");

	if (SERVICE_TYPE_LIVE_STREAM == m_nServiceType || SERVICE_TYPE_SELF_STREAM == m_nServiceType)
	{
		if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)		//Is Viewer or Callee.
		{
			m_pFarEndProcessor = new FarEndProcessorViewer(m_FriendID, m_nServiceType, m_nEntityType, this, pSharedObject, m_bLiveAudioStreamRunning);
		}
		else if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
		{
			m_pFarEndProcessor = new FarEndProcessorPublisher(m_FriendID, m_nServiceType, m_nEntityType, this, pSharedObject, m_bLiveAudioStreamRunning);
		}
	}
	else if (SERVICE_TYPE_CHANNEL == m_nServiceType)
	{
		m_pFarEndProcessor = new FarEndProcessorChannel(m_FriendID, m_nServiceType, m_nEntityType, this, pSharedObject, m_bLiveAudioStreamRunning);
	}
	else if (SERVICE_TYPE_CALL == m_nServiceType || SERVICE_TYPE_SELF_CALL == m_nServiceType)
	{
		m_pFarEndProcessor = new FarEndProcessorCall(m_FriendID, m_nServiceType, m_nEntityType, this, pSharedObject, m_bLiveAudioStreamRunning);
	}

	m_pFarEndProcessor->SetEventCallback((OnFireDataEventCB)OnDataEventCallback, (OnFireNetworkChangeCB)OnNetworkChangeCallback, (OnFireAudioAlarmCB)OnAudioAlarmCallback);

	m_cFarEndProcessorThread = new AudioFarEndProcessorThread(m_pFarEndProcessor);
	if (m_cFarEndProcessorThread != nullptr)
	{
		m_cFarEndProcessorThread->StartFarEndThread();
	}
}


bool CAudioCallSession::getIsAudioLiveStreamRunning(){
	return m_bLiveAudioStreamRunning;
}

void CAudioCallSession::SetEchoCanceller(bool bOn)
{
	return;

	m_bEchoCancellerEnabled = /*bOn*/ true;
}


void CAudioCallSession::StartCallInLive(int iRole, int nCallInLiveType)
{
	if (iRole != ENTITY_TYPE_VIEWER_CALLEE && iRole != ENTITY_TYPE_PUBLISHER_CALLER)//Unsupported or inaccessible role
	{
		return;
	}
	
	if (ENTITY_TYPE_PUBLISHER_CALLER == m_iRole || ENTITY_TYPE_VIEWER_CALLEE == m_iRole) //Call inside a call
	{
		return;
	}

	m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging (true);
	while (m_pFarEndProcessor->m_pLiveAudioParser->IsParsingAudioData())
	{
		m_Tools.SOSleep(1);
	}

	//LOGE("### Start call in live");
	m_nEntityType = iRole;
	m_iRole = iRole;

	if (!m_pEcho.get())
	{
		m_pEcho = EchoCancellerProvider::GetEchoCanceller(WebRTC_ECM);
	}

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

	m_ViewerInCallSentDataQueue.ResetBuffer();
	m_pNearEndProcessor->StartCallInLive(m_nEntityType);
	m_pFarEndProcessor->StartCallInLive(m_nEntityType);

	m_Tools.SOSleep(20);

	m_pFarEndProcessor->m_llDecodingTimeStampOffset = -1;
	m_pFarEndProcessor->m_pAudioDePacketizer->ResetDepacketizer();
#ifdef DUMP_FILE
	if (m_iRole == ENTITY_TYPE_PUBLISHER_CALLER)
	{
		FileInputMuxed= fopen("/sdcard/InputPCMN_MUXED.pcm", "wb");
	}
#endif
	m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(false);
}

void CAudioCallSession::EndCallInLive()
{
	if (m_iRole != ENTITY_TYPE_VIEWER_CALLEE && m_iRole != ENTITY_TYPE_PUBLISHER_CALLER)//Call Not Running
	{
		return;
	}
	m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(true);
	while (m_pFarEndProcessor->m_pLiveAudioParser->IsParsingAudioData())
	{
		m_Tools.SOSleep(1);
	}

#ifdef DUMP_FILE
	if (m_iRole == ENTITY_TYPE_PUBLISHER_CALLER)
	{
		fclose(FileInputMuxed);
	}
#endif

	//m_pLiveAudioReceivedQueue->ResetBuffer();
	m_pFarEndProcessor->m_AudioReceivedBuffer.ResetBuffer();

	m_Tools.SOSleep(20);

	if (m_pEcho.get())
	{
		//TODO: WE MUST FIND BETTER WAY TO AVOID THIS LOOPING
		while (m_bIsAECMNearEndThreadBusy || m_bIsAECMFarEndThreadBusy)
		{
			m_Tools.SOSleep(1);
		}

		m_pEcho.reset();
	}

	if (ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
	{
		m_nEntityType = ENTITY_TYPE_PUBLISHER;
	}
	else if (ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
	{
		m_nEntityType = ENTITY_TYPE_VIEWER;
	}


	m_iRole = m_nEntityType;

	m_pNearEndProcessor->StopCallInLive(m_nEntityType);
	m_pFarEndProcessor->StopCallInLive(m_nEntityType);

	m_pFarEndProcessor->m_llDecodingTimeStampOffset = -1;
	m_pFarEndProcessor->m_pAudioDePacketizer->ResetDepacketizer();
	m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(false);
}

void CAudioCallSession::SetCallInLiveType(int nCallInLiveType)
{
	m_nCallInLiveType = nCallInLiveType;
}

long long CAudioCallSession::GetBaseOfRelativeTime()
{
	return m_pNearEndProcessor->GetBaseOfRelativeTime();
}

int CAudioCallSession::EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength)
{
//	HITLER("#@#@26022017## ENCODE DATA SMAPLE LENGTH %u", unLength);
	if (CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bLiveAudioStreamRunning) != unLength)
	{
		ALOG("Invalid Audio Frame Length");
		return -1;
	}
	//	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");
	long long llCurrentTime = Tools::CurrentTimestamp();
	LOG_50MS("_+_+ NearEnd & Echo Cancellation Time= %lld", llCurrentTime);

	if (m_bEchoCancellerEnabled && 
		(!m_bLiveAudioStreamRunning || 
		(m_bLiveAudioStreamRunning && (ENTITY_TYPE_PUBLISHER_CALLER == m_iRole || ENTITY_TYPE_VIEWER_CALLEE == m_iRole) )))
	{
		m_bIsAECMNearEndThreadBusy = true;

#ifdef DUMP_FILE
		fwrite(psaEncodingAudioData, 2, unLength, FileInputWithEcho);
#endif //DUMP_FILE

		if (m_pEcho.get())
		{
			//m_pEcho->AddFarEndData(psaEncodingAudioData, unLength, getIsAudioLiveStreamRunning());
			m_pEcho->CancelEcho(psaEncodingAudioData, unLength, getIsAudioLiveStreamRunning());

#ifdef DUMP_FILE
			fwrite(psaEncodingAudioData, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bLiveAudioStreamRunning), FileInputPreGain);
#endif
		}

		m_bIsAECMNearEndThreadBusy = false;
	}

	int returnedValue = m_AudioEncodingBuffer.EnQueue(psaEncodingAudioData, unLength, llCurrentTime);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");

	return returnedValue;
}

int CAudioCallSession::CancelAudioData(short *psaPlayingAudioData, unsigned int unLength)
{
	LOG_50MS("_+_+ FarEnd Time= %lld", Tools::CurrentTimestamp());

	if (m_bEchoCancellerEnabled && 
		(!m_bLiveAudioStreamRunning || 
		(m_bLiveAudioStreamRunning && (ENTITY_TYPE_PUBLISHER_CALLER == m_iRole || ENTITY_TYPE_VIEWER_CALLEE == m_iRole))))
	{
		m_bIsAECMFarEndThreadBusy = true;

		if (m_pEcho.get())
		{
			m_pEcho->AddFarEndData(psaPlayingAudioData, unLength, getIsAudioLiveStreamRunning());
		}

		m_bIsAECMFarEndThreadBusy = false;
	}

	return true;
}

void CAudioCallSession::SetVolume(int iVolume, bool bRecorder)
{
	if (bRecorder)
	{
		m_pRecorderGain.get() ? m_pRecorderGain->SetGain(iVolume) : 0;
	}
	else
	{
		m_pPlayerGain.get() ? m_pPlayerGain->SetGain(iVolume) : 0;
	}
}

void CAudioCallSession::SetLoudSpeaker(bool bOn)
{
	m_bUsingLoudSpeaker = bOn;
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


void CAudioCallSession::SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llNow, long long &llLastTime, int iCurrentPacketNumber)
{
	m_pFarEndProcessor->SendToPlayer(pshSentFrame, nSentFrameSize, llLastTime, iCurrentPacketNumber);
}


void CAudioCallSession::GetAudioSendToData(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
	int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime)
{
	m_pNearEndProcessor->GetAudioDataToSend(pAudioCombinedDataToSend, CombinedLength, vCombinedDataLengthVector, sendingLengthViewer, sendingLengthPeer, llAudioChunkDuration, llAudioChunkRelativeTime);
}

int CAudioCallSession::GetServiceType()
{
	return m_nServiceType;
}

int CAudioCallSession::GetRole()
{
	return m_iRole;
}

int CAudioCallSession::GetEntityType()
{
	return m_nEntityType;
}

void CAudioCallSession::OnDataReadyCallback(int mediaType, unsigned char* dataBuffer, size_t dataLength)
{
//	MR_DEBUG("#ptt# CAudioCallSession::OnDataReadyCallback, %x", m_cbClientSendFunction);
	m_cbClientSendFunction(CAudioCallSession::m_FriendID, mediaType, dataBuffer, dataLength, 0, std::vector< std::pair<int, int> >());
}

void CAudioCallSession::OnPacketEventCallback(int eventType, size_t dataLength, unsigned char* dataBuffer)
{
	m_pEventNotifier->fireAudioPacketEvent(eventType, dataLength, dataBuffer);
}

void CAudioCallSession::OnDataEventCallback(int eventType, size_t dataLength, short* dataBuffer)
{
	m_pEventNotifier->fireAudioEvent(m_FriendID, eventType, dataLength, dataBuffer);
}

void CAudioCallSession::OnNetworkChangeCallback(int eventType)
{
	m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_FriendID, eventType);
}

void CAudioCallSession::OnAudioAlarmCallback(int eventType)
{
	m_pEventNotifier->fireAudioAlarm(eventType, 0, 0);
}
