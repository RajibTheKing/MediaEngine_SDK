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
#include "EchoCancellerInterface.h"
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
#include "AudioEncoderBuffer.h"
#include "AudioResources.h"
#include "AudioDecoderBuffer.h"
#include "Trace.h"
#include "AudioLinearBuffer.h"



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

#define MAX_TOLERABLE_TRACE_WAITING_FRAME_COUNT 11
#define DUPLICATE_AUDIO

namespace MediaSDK
{
	CAudioCallSession::CAudioCallSession(const bool& isVideoCallRunning, LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, int nEntityType, AudioResources &audioResources, int nAudioSpeakerType) :
		m_bIsVideoCallRunning(isVideoCallRunning),
		m_nEntityType(nEntityType),
		m_nServiceType(nServiceType),
		m_llLastPlayTime(0),
		m_bIsAECMFarEndThreadBusy(false),
		m_bIsAECMNearEndThreadBusy(false),
		m_nCallInLiveType(CALL_IN_LIVE_TYPE_AUDIO_VIDEO),
		m_bIsPublisher(true),
		m_cNearEndProcessorThread(nullptr),
		m_cFarEndProcessorThread(nullptr),
		m_bNeedToResetEcho(false)
	{
		m_recordBuffer = new AudioLinearBuffer(LINEAR_BUFFER_MAX_SIZE);

		m_pAudioCallSessionMutex.reset(new CLockHandler);
		m_PublisherBufferForMuxing.reset(new AudioShortBufferForPublisherFarEnd);
		m_FarendBuffer.reset(new CAudioShortBuffer);
		m_AudioNearEndBuffer.reset(new CAudioShortBuffer);
		m_ViewerInCallSentDataQueue.reset(new CAudioShortBuffer);

		SetResources(audioResources);
		m_pTrace = new CTrace();

		m_iSpeakerType = nAudioSpeakerType;
		

		SetSendFunction(pSharedObject->GetSendFunctionPointer());
		SetEventNotifier(pSharedObject->m_pEventNotifier);

		m_FriendID = llFriendID;

		//m_pAudioDePacketizer = new AudioDePacketizer(this);
		m_iRole = nEntityType;
		m_bLiveAudioStreamRunning = false;


		if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM || m_nServiceType == SERVICE_TYPE_CHANNEL)
		{
			m_bLiveAudioStreamRunning = true;
			//m_pPlayerGain->SetGain(9);
		}

		m_iPrevRecvdSlotID = -1;
		m_iReceivedPacketsInPrevSlot = AUDIO_SLOT_SIZE; //used by child
		m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;


		if (!m_bLiveAudioStreamRunning)
		{
			m_bEnableRecorderTimeSyncDuringEchoCancellation = true;
			m_bEnablePlayerTimeSyncDuringEchoCancellation = true;
		}
		else
		{
			m_bEnableRecorderTimeSyncDuringEchoCancellation = true;
			m_bEnablePlayerTimeSyncDuringEchoCancellation = true;
		}


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

#ifdef PCM_DUMP
		long long llcurrentTime;
		std::string sCurrentTime;
		std::stringstream ss;

		ss.clear();
		llcurrentTime = (Tools::CurrentTimestamp() / 10000) % 100000;
		ss << llcurrentTime;
		ss >> sCurrentTime;

		std::string filePrefix = "/sdcard/";
		std::string fileExtension = ".pcm";

		std::string RecordedFileName = filePrefix + sCurrentTime + "-Recorded" + fileExtension;
		std::string EchoCancelledFileName = filePrefix + sCurrentTime + "-Cancelled" + fileExtension;
		std::string PlayedFileName = filePrefix + sCurrentTime + "-Played" + fileExtension;
		std::string AfterEchoCancellationFileName = filePrefix + sCurrentTime + "-AfterCancellation" + fileExtension;

		RecordedFile = fopen(RecordedFileName.c_str(), "wb");
		EchoCancelledFile = fopen(EchoCancelledFileName.c_str(), "wb");
		PlayedFile = fopen(PlayedFileName.c_str(), "wb");
		AfterEchoCancellationFile = fopen(AfterEchoCancellationFileName.c_str(), "wb");
#endif 

		InitNearEndDataProcessing();
		InitFarEndDataProcessing();

		ResetTrace();

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

		if (m_pTrace)
		{
			delete m_pTrace;
			m_pTrace = NULL;
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

#ifdef PCM_DUMP
		if (RecordedFile) fclose(RecordedFile);
		if (EchoCancelledFile) fclose(EchoCancelledFile);
		if (AfterEchoCancellationFile) fclose(AfterEchoCancellationFile);
		if (PlayedFile) fclose(PlayedFile);
#endif
		if (m_recordBuffer)
		{
			delete m_recordBuffer;
		}
		SHARED_PTR_DELETE(m_pAudioCallSessionMutex);
	}

	void CAudioCallSession::ResetTrace()
	{
		//Trace and Delay Related
		m_bRecordingStarted = false;
		m_llTraceSendingTime = 0;
		m_llTraceReceivingTime = 0;
		m_b1stRecordedDataSinceCallStarted = true;
		m_llDelayFraction = 0;
		m_llDelay = 0;
		m_iDeleteCount = 10;
		m_bTraceSent = m_bTraceRecieved = m_bTraceWillNotBeReceived = false;
		m_nFramesRecvdSinceTraceSent = 0;
		m_bTraceTailRemains = true;
		m_pTrace->Reset();
		m_FarendBuffer->ResetBuffer();
		m_pFarEndProcessor->m_b1stPlaying = true;
		m_pFarEndProcessor->m_llNextPlayingTime = -1;
		m_iStartingBufferSize = m_iDelayFractionOrig = -1;
	}

	void CAudioCallSession::ResetAEC()
	{
		if (m_pEcho.get())
		{
			m_pEcho.reset();
		}

		m_pEcho = EchoCancellerProvider::GetEchoCanceller(WebRTC_ECM, m_bLiveAudioStreamRunning);
	}

	bool CAudioCallSession::IsEchoCancellerEnabled()
	{
#ifdef USE_AECM
#ifdef __ANDROID__
		if (!m_bLiveAudioStreamRunning || (m_bLiveAudioStreamRunning && (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER || m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)))
		{
			return true;
		}
		else
		{
			return false;
		}
#elif defined (DESKTOP_C_SHARP)
		if (!m_bLiveAudioStreamRunning)
		{
			return true;
		}
		else
		{
			return false;
		}
#endif
#else
		return false
#endif
	}

	bool CAudioCallSession::IsTraceSendingEnabled()
	{
#ifdef USE_AECM
#ifdef __ANDROID__
		if (m_iSpeakerType == AUDIO_PLAYER_LOUDSPEAKER
			&& !m_bLiveAudioStreamRunning)
		{
			return true;
		}
		else
		{
			return false;
		}
#elif defined (DESKTOP_C_SHARP)
		return false;
#endif
#else
		return false
#endif
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
		}

		m_pAudioDecoder = audioResources.GetDecoder();

		m_pEcho = audioResources.GetEchoCanceler();
		m_pNoiseReducer = audioResources.GetNoiseReducer();

		m_pPlayerGain = audioResources.GetPlayerGain();
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
		m_pNearEndProcessor->SetEventCallback(this);
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


	void CAudioCallSession::SetEchoCanceller(bool bOn)
	{

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

		m_pFarEndProcessor->m_pLiveAudioParser->SetRoleChanging(true);
		while (m_pFarEndProcessor->m_pLiveAudioParser->IsParsingAudioData())
		{
			Tools::SOSleep(1);
		}

		//LOGE("### Start call in live");
		m_nEntityType = iRole;
		m_iRole = iRole;

		if (!m_pEcho.get())
		{
			m_pEcho = EchoCancellerProvider::GetEchoCanceller(WebRTC_ECM, true);
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

		m_ViewerInCallSentDataQueue->ResetBuffer();
		m_pNearEndProcessor->StartCallInLive(m_nEntityType);
		m_pFarEndProcessor->StartCallInLive(m_nEntityType);

		Tools::SOSleep(20);

		m_pFarEndProcessor->m_llDecodingTimeStampOffset = -1;
		m_pFarEndProcessor->m_pAudioDePacketizer->ResetDepacketizer();

		ResetTrace();
#ifdef DUMP_FILE
		if (m_iRole == ENTITY_TYPE_PUBLISHER_CALLER)
		{
			FileInputMuxed = fopen("/sdcard/InputPCMN_MUXED.pcm", "wb");
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

		if (m_pEcho.get())
		{
			//TODO: WE MUST FIND BETTER WAY TO AVOID THIS LOOPING
			while (m_bIsAECMNearEndThreadBusy || m_bIsAECMFarEndThreadBusy)
			{
				Tools::SOSleep(1);
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

	void CAudioCallSession::SyncRecordingTime()
	{
		if (m_b1stRecordedDataSinceCallStarted)
		{
			m_ll1stRecordedDataTime = Tools::CurrentTimestamp();
			m_llnextRecordedDataTime = m_ll1stRecordedDataTime + 100;
			m_b1stRecordedDataSinceCallStarted = false;
		}
		else
		{
			long long llNOw = Tools::CurrentTimestamp();
			if (llNOw + 20 < m_llnextRecordedDataTime)
			{
				Tools::SOSleep(m_llnextRecordedDataTime - llNOw - 20);
			}
			m_llnextRecordedDataTime += 100;
		}
	}

	void CAudioCallSession::PreprocessAudioData(short *psaEncodingAudioData, unsigned int unLength)
	{
		long long llCurrentTime = Tools::CurrentTimestamp();
		LOG_50MS("_+_+ NearEnd & Echo Cancellation Time= %lld", llCurrentTime);

#ifdef USE_AECM

#ifdef PCM_DUMP
		if (RecordedFile)
		{
			fwrite(psaEncodingAudioData, 2, unLength, RecordedFile);
		}
#endif

		if (IsEchoCancellerEnabled())
		{
			if (m_bNeedToResetEcho)
			{
				ResetAEC();
				ResetTrace();
				m_bNeedToResetEcho = false;
			}
			//Sleep to maintain 100 ms recording time diff
			if (m_bEnableRecorderTimeSyncDuringEchoCancellation)
			{
				SyncRecordingTime();
			}

			//If trace is received, current and next frames are deleted
			if ((m_bTraceRecieved || m_bTraceWillNotBeReceived) && m_iDeleteCount > 0)
			{
				memset(psaEncodingAudioData, 0, sizeof(short) * unLength);
				m_iDeleteCount--;
			}
			//Handle Trace
			if (!m_bTraceRecieved && m_bTraceSent && m_nFramesRecvdSinceTraceSent < MAX_TOLERABLE_TRACE_WAITING_FRAME_COUNT)
			{
				m_nFramesRecvdSinceTraceSent++;
				if (m_nFramesRecvdSinceTraceSent == MAX_TOLERABLE_TRACE_WAITING_FRAME_COUNT)
				{
					m_FarendBuffer->ResetBuffer();
					m_bTraceWillNotBeReceived = true; // 8-(
				}
				else
				{
					m_llDelayFraction = m_pTrace->DetectTrace(psaEncodingAudioData, unLength, 80);
					LOG18("mansur: m_llDelayFraction : %lld", m_llDelayFraction);
					if (m_llDelayFraction != -1)
					{
						m_llTraceReceivingTime = Tools::CurrentTimestamp();
						m_llDelay = m_llTraceReceivingTime - m_llTraceSendingTime;
						//m_llDelayFraction = m_llDelay % 100;
						m_iDelayFractionOrig = m_llDelayFraction;
						m_llDelayFraction /= 8;
						memset(psaEncodingAudioData, 0, sizeof(short) * unLength);
						m_bTraceRecieved = true;
					}
				}

			}
			if (!m_bTraceRecieved && !m_bTraceWillNotBeReceived)
			{
				memset(psaEncodingAudioData, 0, sizeof(short) * unLength);
			}
			LOGFARQUAD("55555Delay = %lld, m_bTraceRecieved = %d, m_bTraceSent = %d, m_llTraceSendingTime = %lld, m_iDelayFractionOrig= %d\n",
				m_llDelay, m_bTraceRecieved, m_bTraceSent, m_llTraceSendingTime, m_iDelayFractionOrig);


			LOG18("b4 farnear m_bTraceRecieved = %d", m_bTraceRecieved);
			m_bIsAECMNearEndThreadBusy = true;

#ifdef DUMP_FILE
			fwrite(psaEncodingAudioData, 2, unLength, FileInputWithEcho);
#endif //DUMP_FILE

			if (m_pEcho.get() && (m_bTraceRecieved || m_bTraceWillNotBeReceived))
			{
				long long llTS;
				if (m_iStartingBufferSize == -1)
				{
					m_iStartingBufferSize = m_FarendBuffer->GetQueueSize();
					LOGFARQUAD("qpushpop getting m_iStartingBufferSize,  m_FarendBufferSize = %d, m_iStartingBufferSize = %d, m_llDelay = %lld, m_bTraceRecieved = %d",
						m_FarendBuffer->GetQueueSize(), m_iStartingBufferSize, m_llDelay, m_bTraceRecieved);
				}
				LOG18("mansur: entering m_llDelayFraction : %d", m_llDelayFraction);
				long long llCurrentTimeStamp = Tools::CurrentTimestamp();
				LOGFARQUAD("qpushpop m_FarendBufferSize = %d, m_iStartingBufferSize = %d, m_llDelay = %lld, m_bTraceRecieved = %d llCurrentTimeStamp = %lld",
					m_FarendBuffer->GetQueueSize(), m_iStartingBufferSize, m_llDelay, m_bTraceRecieved, llCurrentTimeStamp);

				int iFarendDataLength = m_FarendBuffer->DeQueue(m_saFarendData, llTS);
				if (iFarendDataLength > 0)
				{
					if ((m_iSpeakerType == AUDIO_PLAYER_LOUDSPEAKER) && GetPlayerGain().get())
					{
						GetPlayerGain()->AddFarEnd(m_saFarendData, unLength);
					}

					m_pEcho->AddFarEndData(m_saFarendData, unLength, getIsAudioLiveStreamRunning());


					m_pEcho->CancelEcho(psaEncodingAudioData, unLength, getIsAudioLiveStreamRunning(), m_llDelayFraction);

					if ((m_iSpeakerType == AUDIO_PLAYER_LOUDSPEAKER) && GetPlayerGain().get())
					{
						GetPlayerGain()->AddGain(psaEncodingAudioData, unLength, m_nServiceType == SERVICE_TYPE_LIVE_STREAM);
					}

					LOG18("Successful farnear");
#ifdef PCM_DUMP
					if (EchoCancelledFile)
					{
						fwrite(psaEncodingAudioData, 2, unLength, EchoCancelledFile);
					}
#endif
				}
				else
				{
					LOG18("UnSuccessful farnear");
				}

#ifdef DUMP_FILE
				fwrite(psaEncodingAudioData, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bLiveAudioStreamRunning), FileInputPreGain);
#endif

			}
#ifdef PCM_DUMP
			if (AfterEchoCancellationFile)
			{
				fwrite(psaEncodingAudioData, 2, unLength, AfterEchoCancellationFile);
			}
#endif

			m_bIsAECMNearEndThreadBusy = false;
		}

#elif defined(DESKTOP_C_SHARP)
		if ((m_iSpeakerType == AUDIO_PLAYER_LOUDSPEAKER) && GetPlayerGain().get())
		{
			GetPlayerGain()->AddGain(psaEncodingAudioData, unLength, m_nServiceType == SERVICE_TYPE_LIVE_STREAM);
		}
		else printf("##TT encodeaudiodata no gain\n");
#endif
	}

	int CAudioCallSession::PushAudioData(short *psaEncodingAudioData, unsigned int unLength)
	{
		//	HITLER("#@#@26022017## ENCODE DATA SMAPLE LENGTH %u", unLength);
		/*if (CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bLiveAudioStreamRunning) != unLength)
		{
		ALOG("Invalid Audio Frame Length");
		return -1;
		}*/
		//	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");
		m_bRecordingStarted = true;
		//LOGT("##TT encodeaudiodata");
		//int returnedValue = m_AudioNearEndBuffer.EnQueue(psaEncodingAudioData, unLength, Tools::CurrentTimestamp());
		m_recordBuffer->PushData(psaEncodingAudioData, unLength);

		return 0;
	}

	int CAudioCallSession::CancelAudioData(short *psaPlayingAudioData, unsigned int unLength)
	{
		/*LOG_50MS("_+_+ FarEnd Time= %lld", Tools::CurrentTimestamp());

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
		}*/

		return true;
	}

	void CAudioCallSession::SetVolume(int iVolume, bool bRecorder)
	{
		m_pPlayerGain.get() ? m_pPlayerGain->SetGain(iVolume) : 0;
	}

	void CAudioCallSession::SetSpeakerType(int iSpeakerType)
	{
		if (m_iSpeakerType != iSpeakerType)
		{
			m_bNeedToResetEcho = true;
		}
		m_iSpeakerType = iSpeakerType;
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


	void CAudioCallSession::GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime)
	{
		m_pNearEndProcessor->GetAudioDataToSend(pAudioCombinedDataToSend, CombinedLength, vCombinedDataLengthVector, sendingLengthViewer, sendingLengthPeer, llAudioChunkDuration, llAudioChunkRelativeTime);
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

	void CAudioCallSession::FireAudioAlarm(int eventType)
	{
		m_pEventNotifier->fireAudioAlarm(eventType, 0, 0);
	}

} //namespace MediaSDK
