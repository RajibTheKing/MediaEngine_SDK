#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioPacketizer.h"
#include "AudioDePacketizer.h"
#include "LiveAudioParser.h"


#ifdef LOCAL_SERVER_LIVE_CALL
#define LOCAL_SERVER_IP "192.168.0.120"
#endif

#define __DUPLICATE_AUDIO__

#ifdef USE_AECM
#include "Echo.h"
#endif
#ifdef USE_ANS
#include "Noise.h"
#endif
#ifdef USE_AGC
#include "Gain.h"
#endif
#ifdef USE_VAD
#include "Voice.h"
#endif
#include "GomGomGain.h"




#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif


CAudioCallSession::CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, bool bIsCheckCall) :
m_pCommonElementsBucket(pSharedObject),
m_bIsCheckCall(bIsCheckCall),
m_nServiceType(nServiceType),
m_llLastPlayTime(0),
m_bIsAECMFarEndThreadBusy(false),
m_bIsAECMNearEndThreadBusy(false),
m_nCallInLiveType(CALL_IN_LIVE_TYPE_AUDIO_VIDEO),
m_bIsPublisher(true)
{
	InitializeAudioCallSession(llFriendID);
	//m_pAudioDePacketizer = new AudioDePacketizer(this);
	m_iRole = CALL_NOT_RUNNING;
	m_bLiveAudioStreamRunning = false;

	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
	{
		m_bLiveAudioStreamRunning = true;
	}

	m_pAudioCallSessionMutex.reset(new CLockHandler);
	m_FriendID = llFriendID;

#ifdef AAC_ENABLED
	m_cAac = new CAac();
	m_cAac->SetParameters(44100, 2);
#endif

	m_iPrevRecvdSlotID = -1;
	m_iReceivedPacketsInPrevSlot = AUDIO_SLOT_SIZE; //used by child
	m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;

	m_bUsingLoudSpeaker = false;
	m_bEchoCancellerEnabled = true;

	if(m_bLiveAudioStreamRunning)
	{
		//m_bEchoCancellerEnabled = false;
	}

#ifdef USE_AECM
	m_pEcho = new CEcho(66);
#ifdef USE_ECHO2
	m_pEcho2 = new CEcho(77);
#endif
#endif

#ifdef USE_ANS
	m_pNoise = new CNoise();
#endif

#ifdef USE_AGC
	m_pRecorderGain = new CGain();
	m_pPlayerGain = new CGain();
#endif

#ifdef USE_VAD
	m_pVoice = new CVoice();
#endif

	
	m_iAudioVersionFriend = -1;
	if(m_bLiveAudioStreamRunning)
	{
		m_iAudioVersionSelf = __AUDIO_LIVE_VERSION__;
	}
	else {
		m_iAudioVersionSelf = __AUDIO_CALL_VERSION__;
	}
#ifdef LOCAL_SERVER_LIVE_CALL
	m_clientSocket = VideoSockets::GetInstance();
	m_clientSocket->SetAudioCallSession(this);
#endif

	m_pNearEndProcessor = new CAudioNearEndDataProcessor(llFriendID, this, pSharedObject, &m_AudioEncodingBuffer, m_bLiveAudioStreamRunning);
	m_pFarEndProcessor = new CAudioFarEndDataProcessor(llFriendID, this, pSharedObject, m_bLiveAudioStreamRunning);

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");
}

CAudioCallSession::~CAudioCallSession()
{
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
	
#ifdef AAC_ENABLED
	if (m_cAac != nullptr){
		delete m_cAac;
		m_cAac = nullptr;
	}
#endif
#ifdef OPUS_ENABLED
	delete m_pAudioCodec;
#else
	delete m_pG729CodecNative;
#endif
#ifdef USE_AECM
	if (m_pEcho != nullptr)
	{
	delete m_pEcho;
		m_pEcho = nullptr;
	}
#ifdef USE_ECHO2
	if (m_pEcho2 != nullptr)
	{
	delete m_pEcho2;
		m_pEcho2 = nullptr;
	}
#endif
#endif
#ifdef USE_ANS
	delete m_pNoise;
#endif
#ifdef USE_AGC
	delete m_pRecorderGain;
	delete m_pPlayerGain;
#endif
#ifdef USE_VAD
	delete m_pVoice;
#endif

	m_FriendID = -1;
#ifdef __DUMP_FILE__
	fclose(FileOutput);
	fclose(FileInput);
	fclose(FileInputWithEcho);
	fclose(FileInputPreGain);
#endif

	SHARED_PTR_DELETE(m_pAudioCallSessionMutex);
}

void CAudioCallSession::InitializeAudioCallSession(LongLong llFriendID)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession");

	//this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket);

	//m_pAudioCodec->CreateAudioEncoder();

	//m_pAudioDecoder->CreateAudioDecoder();

	this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket, this, llFriendID);
	m_pAudioCodec->CreateAudioEncoder();

	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession session initialized, iRet = " + m_Tools.IntegertoStringConvert(iRet));

}

bool CAudioCallSession::getIsAudioLiveStreamRunning(){
	return m_bLiveAudioStreamRunning;
}

void CAudioCallSession::SetEchoCanceller(bool bOn)
{
	return;
#ifdef USE_AECM
	m_bEchoCancellerEnabled = /*bOn*/ true;
#endif
}


void CAudioCallSession::StartCallInLive(int iRole, int nCallInLiveType)
{
	if (iRole != VIEWER_IN_CALL && iRole != PUBLISHER_IN_CALL)//Unsupported or inaccessible role
	{
		return;
	}
	if (m_iRole != CALL_NOT_RUNNING)//Call inside a call
	{
		return;
	}

	m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging (true);
	while (m_pFarEndProcessor->m_pLiveAudioParser->IsParsingAudioData())
	{
		m_Tools.SOSleep(1);
	}

	//LOGE("### Start call in live");
	m_iRole = iRole;

#ifdef USE_AECM
	if (m_pEcho == nullptr)
	{
		m_pEcho = new CEcho(66);
	}
#ifdef USE_ECHO2
	if (m_pEcho2 == nullptr)
	{
		m_pEcho2 = new CEcho(77);
	}
#endif
#endif

	if (m_iRole == PUBLISHER_IN_CALL)
	{
#ifdef LOCAL_SERVER_LIVE_CALL
		m_clientSocket->InitializeSocket(LOCAL_SERVER_IP, 60001);
#endif
		m_AudioDecodedBuffer.ResetBuffer(); //Contains Data From Last Call
	}
	else if (m_iRole == VIEWER_IN_CALL)
	{
#ifdef LOCAL_SERVER_LIVE_CALL
		m_clientSocket->InitializeSocket(LOCAL_SERVER_IP, 60002);
#endif
	}

	m_pNearEndProcessor->StartCallInLive();
	m_pFarEndProcessor->StartCallInLive();

	m_Tools.SOSleep(20);

	m_pFarEndProcessor->m_llDecodingTimeStampOffset = -1;
	m_pFarEndProcessor->m_pAudioDePacketizer->ResetDepacketizer();
#ifdef __DUMP_FILE__
	if (m_iRole == PUBLISHER_IN_CALL)
	{
		FileInputMuxed= fopen("/sdcard/InputPCMN_MUXED.pcm", "wb");
	}
#endif
	m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(false);
}

void CAudioCallSession::EndCallInLive()
{
	if (m_iRole != VIEWER_IN_CALL && m_iRole != PUBLISHER_IN_CALL)//Call Not Running
	{
		return;
	}
	m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(true);
	while (m_pFarEndProcessor->m_pLiveAudioParser->IsParsingAudioData())
	{
		m_Tools.SOSleep(1);
	}

#ifdef __DUMP_FILE__
	if (m_iRole == PUBLISHER_IN_CALL)
	{
		fclose(FileInputMuxed);
	}
#endif

	//m_pLiveAudioReceivedQueue->ResetBuffer();
	m_pFarEndProcessor->m_AudioReceivedBuffer.ResetBuffer();

	m_Tools.SOSleep(20);

#ifdef USE_AECM
	if (m_pEcho != nullptr)
	{
		while (m_bIsAECMNearEndThreadBusy || m_bIsAECMFarEndThreadBusy )
		{
			m_Tools.SOSleep(1);
		}

		delete m_pEcho;
		m_pEcho = nullptr;
	}
#ifdef USE_ECHO2
	if (m_pEcho2 != nullptr)
	{
		delete m_pEcho2;
		m_pEcho2 = nullptr;
	}
#endif
#endif

	m_iRole = CALL_NOT_RUNNING;
	m_pFarEndProcessor->m_llDecodingTimeStampOffset = -1;
	m_pFarEndProcessor->m_pAudioDePacketizer->ResetDepacketizer();
	m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(false);
}

void CAudioCallSession::SetCallInLiveType(int nCallInLiveType)
{
	m_nCallInLiveType = nCallInLiveType;
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
#ifdef USE_AECM
	if (m_bEchoCancellerEnabled && 
		(!m_bLiveAudioStreamRunning || 
		(m_bLiveAudioStreamRunning && m_iRole != CALL_NOT_RUNNING)))
	{
		m_bIsAECMNearEndThreadBusy = true;
#ifdef USE_ECHO2
		if (m_pEcho2 != nullptr)
		{
			m_pEcho2->AddFarEnd(psaEncodingAudioData, unLength, getIsAudioLiveStreamRunning());
		}
#endif
#ifdef __DUMP_FILE__
		fwrite(psaEncodingAudioData, 2, unLength, FileInputWithEcho);
#endif // __DUMP_FILE__
		if (m_pEcho != nullptr)
		{
			m_pEcho->CancelEcho(psaEncodingAudioData, unLength, m_bUsingLoudSpeaker, getIsAudioLiveStreamRunning());
#ifdef __DUMP_FILE__
			fwrite(psaEncodingAudioData, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bLiveAudioStreamRunning), FileInputPreGain);
#endif
		}
		m_bIsAECMNearEndThreadBusy = false;
	}
#endif
	int returnedValue = m_AudioEncodingBuffer.EnQueue(psaEncodingAudioData, unLength, llCurrentTime);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");

	return returnedValue;
}

int CAudioCallSession::CancelAudioData(short *psaPlayingAudioData, unsigned int unLength)
{
	LOG_50MS("_+_+ FarEnd Time= %lld", Tools::CurrentTimestamp());
#ifdef USE_AECM
	if (m_bEchoCancellerEnabled && 
		(!m_bLiveAudioStreamRunning || 
		(m_bLiveAudioStreamRunning && m_iRole != CALL_NOT_RUNNING)))
	{
		m_bIsAECMFarEndThreadBusy = true;
#ifdef USE_ECHO2
		if (m_pEcho2 != nullptr)
		{
			m_pEcho2->CancelEcho(psaPlayingAudioData, unLength, m_bUsingLoudSpeaker, getIsAudioLiveStreamRunning());
		}
#endif
		if (m_pEcho != nullptr)
		{
			m_pEcho->AddFarEnd(psaPlayingAudioData, unLength, getIsAudioLiveStreamRunning(), m_bUsingLoudSpeaker);
		}
		m_bIsAECMFarEndThreadBusy = false;
	}
#endif
	return true;
}

void CAudioCallSession::SetVolume(int iVolume, bool bRecorder)
{
#ifdef USE_AGC
	//if (!m_bLiveAudioStreamRunning)
	{
		if (bRecorder)
		{
			m_pRecorderGain->SetGain(iVolume);
		}
		else
		{
			m_pPlayerGain->SetGain(iVolume);
		}
	}
#endif
}

void CAudioCallSession::SetLoudSpeaker(bool bOn)
{
#ifdef USE_AGC
	/*if (m_bUsingLoudSpeaker != bOn)
	{
	m_bUsingLoudSpeaker = bOn;
	if (bOn)
	{
	m_iVolume = m_iVolume * 1.0 / LS_RATIO;
	}
	else
	{
	m_iVolume *= LS_RATIO;
	}
	}*/
	//This method may be used in future.
#endif
	m_bUsingLoudSpeaker = bOn;
	/*
	#ifdef USE_AECM
	delete m_pEcho;
	m_pEcho = new CEcho(66);
	#ifdef USE_ECHO2
	delete m_pEcho2;
	m_pEcho2 = new CEcho(77);
	#endif
	#endif*/
}

int CAudioCallSession::DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	return m_pFarEndProcessor->DecodeAudioData(nOffset, pucaDecodingAudioData, unLength, numberOfFrames, frameSizes, vMissingFrames);
}

CAudioCodec* CAudioCallSession::GetAudioCodec()
{
	return m_pAudioCodec;
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