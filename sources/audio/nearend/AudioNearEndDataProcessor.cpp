
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
#include "AudioLinearBuffer.h"

#include "EchoCancellerProvider.h"
#include "EchoCancellerInterface.h"
#include "KichCutter.h"
#include "Trace.h"
#include "NoiseReducerProvider.h"
#include "AudioGainInstanceProvider.h"
#include "AudioFarEndDataProcessor.h"


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define MAX_TOLERABLE_TRACE_WAITING_FRAME_COUNT 11
#define TRACE_DETECTION_DURATION_IN_SAMPLES 60

#define TR_TRACE_SENT 20000
#define TR_TRACE_NOT_SENT 30000
#define TR_TRACE_RECVD 10000
#define TR_TRACE_WONT_BE_RECVD -10000

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
		m_pPacketEventListener(nullptr),
		m_bNeedToResetAudioEffects(true)
	{
		m_recordBuffer = new AudioLinearBuffer(LINEAR_BUFFER_MAX_SIZE);

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
		bool bEnableNearEndDumps = true;
		m_pRecordedNE = new CAudioDumper("Recorded.pcm", bEnableNearEndDumps);
		m_pGainedNE = new CAudioDumper("Gained.pcm", bEnableNearEndDumps);
		m_pProcessed2NE = new CAudioDumper("AfterCancellation.pcm", bEnableNearEndDumps);
		m_pNoiseReducedNE = new CAudioDumper("NR.pcm", bEnableNearEndDumps);
		m_pCancelledNE = new CAudioDumper("AEC.pcm", bEnableNearEndDumps);
		m_pKichCutNE = new CAudioDumper("KC.pcm", bEnableNearEndDumps);
		m_pProcessedNE = new CAudioDumper("processed.pcm", bEnableNearEndDumps);
		m_pTraceRemoved = new CAudioDumper("tr.pcm", bEnableNearEndDumps);
		m_pTraceDetectionDump = new CAudioDumper("TD.pcm", bEnableNearEndDumps);

		m_pTrace = new CTrace();
		m_pKichCutter = nullptr;

		m_pAudioSessionStatistics = new AudioSessionStatistics(m_pAudioCallSession->GetEntityType());

	}

	AudioNearEndDataProcessor::~AudioNearEndDataProcessor()
	{
		if (m_pAudioNearEndPacketHeader)
		{
			//delete m_pAudioPacketHeader;
		}

		if (m_recordBuffer != nullptr)
		{
			delete m_recordBuffer;
			m_recordBuffer = nullptr;
		}

		if (m_pRecordedNE != nullptr)
		{
			delete m_pRecordedNE;
			m_pRecordedNE = nullptr;
		}

		if (m_pGainedNE != nullptr)
		{
			delete m_pGainedNE;
			m_pGainedNE = nullptr;
		}
		if (m_pTraceRemoved != nullptr)
		{
			delete m_pTraceRemoved;
			m_pTraceRemoved = nullptr;
		}

		if (m_pProcessed2NE != nullptr)
		{
			delete m_pProcessed2NE;
			m_pProcessed2NE = nullptr;
		}

		if (m_pNoiseReducedNE != nullptr)
		{
			delete m_pNoiseReducedNE;
			m_pNoiseReducedNE = nullptr;
		}

		if (m_pCancelledNE != nullptr)
		{
			delete m_pCancelledNE;
			m_pCancelledNE = nullptr;
		}

		if (m_pKichCutNE != nullptr)
		{
			delete m_pKichCutNE;
			m_pKichCutNE = nullptr;
		}

		if (m_pTraceDetectionDump != nullptr)
		{
			delete m_pTraceDetectionDump;
			m_pTraceDetectionDump = nullptr;
		}

		if (m_pProcessedNE != nullptr)
		{
			delete m_pProcessedNE;
			m_pProcessedNE = nullptr;
		}

		if (m_pTrace != nullptr)
		{
			delete m_pTrace;
			m_pTrace = nullptr;
		}

		if (nullptr != m_pKichCutter)
		{
			delete m_pKichCutter;
			m_pKichCutter = nullptr;
		}

		if (nullptr != m_pAudioSessionStatistics)
		{
			delete m_pAudioSessionStatistics;
			m_pAudioSessionStatistics = nullptr;
		}
	}

	void AudioNearEndDataProcessor::SetNeedToResetAudioEffects(bool flag)
	{
		m_bNeedToResetAudioEffects = flag;
	}

	void AudioNearEndDataProcessor::SetEnableRecorderTimeSyncDuringEchoCancellation(bool flag)
	{
		m_bEnableRecorderTimeSyncDuringEchoCancellation = flag;
	}

	void AudioNearEndDataProcessor::ResetTrace()
	{
		MediaLog(LOG_DEBUG, "Reset Trace Starting")
		//Trace and Delay Related		
		m_llTraceSendingTime = 0;
		m_llTraceReceivingTime = 0;
		m_b1stRecordedDataSinceCallStarted = true;
		m_llDelayFraction = -1;
		m_llDelay = 0;
		m_bTraceSent = m_bTraceRecieved = m_bTraceWillNotBeReceived = m_b30VerifiedTrace = m_bJustWrittenTraceDump = false;
		m_nFramesRecvdSinceTraceSent = 0;
		m_bTraceTailRemains = true;
		m_pTrace->Reset();
		m_iDeleteCount = (m_pTrace->m_iTracePatternLength / MAX_AUDIO_FRAME_SAMPLE_SIZE) + 2;
		m_pAudioCallSession->m_FarendBuffer->ResetBuffer();
		m_pAudioCallSession->m_pFarEndProcessor->m_bPlayingNotStartedYet = true;
		m_pAudioCallSession->m_pFarEndProcessor->m_llNextPlayingTime = -1;
		m_iStartingBufferSize = m_iDelayFractionOrig = -1;


		m_pAudioCallSession->SetRecordingStarted(true);

		MediaLog(LOG_DEBUG, "Reset Trace Ending")
	}

	void AudioNearEndDataProcessor::HandleTrace(short *psaEncodingAudioData, unsigned int unLength)
	{
		MediaLog(LOG_DEBUG, "[ANEDP][HT] length: %d", unLength);
		int iTraceInFrame = 1;
		if (!m_b30VerifiedTrace && m_bTraceSent && m_nFramesRecvdSinceTraceSent < MAX_TOLERABLE_TRACE_WAITING_FRAME_COUNT)
		{
			MediaLog(LOG_DEBUG, "[NE][ACS][TS] HandleTrace->IsEchoCancellerEnabled->Trace handled");
			m_nFramesRecvdSinceTraceSent++;
			if (m_nFramesRecvdSinceTraceSent == MAX_TOLERABLE_TRACE_WAITING_FRAME_COUNT)
			{
				MediaLog(LOG_DEBUG, "[NE][ACS][TS] HandleTrace->IsEchoCancellerEnabled->Trace handled->m_nFramesRecvdSinceTraceSent");
				m_pAudioCallSession->m_FarendBuffer->ResetBuffer();
				m_llDelay = 0;
				m_bTraceWillNotBeReceived = true; // 8-(
				MediaLog(LOG_DEBUG, "[ACS][ECHO][TS]  Detection Failed m_iSpeakerType = %d",  m_pAudioCallSession->m_iSpeakerType);
			}
			else
			{
				if (IsTraceSendingEnabled())
				{
					long long llDelayFraction = m_pTrace->DetectTrace3Times(psaEncodingAudioData, iTraceInFrame, m_b30VerifiedTrace); //we call it even after trace it received, in case m_b30VerifiedTrace is still false
					MediaLog(LOG_DEBUG, "[ACS][ECHO][TS] TimeDelay = %lldms, DelayFra,  = %lld[Sample:%d] iTraceInFrame = %d m_iSpeakerType = %d, m_b30VerifiedTrace = %d",
						m_llDelay, m_llDelayFraction, m_iDelayFractionOrig, iTraceInFrame, m_pAudioCallSession->m_iSpeakerType, m_b30VerifiedTrace);
					if (llDelayFraction != -1 && m_bTraceRecieved == false) //just got the trace
					{
						m_llDelayFraction = llDelayFraction;
						m_llTraceReceivingTime = Tools::CurrentTimestamp();
						m_llDelay = m_llTraceReceivingTime - m_llTraceSendingTime;
						//m_llDelayFraction = m_llDelay % 100;
						m_iDelayFractionOrig = m_llDelayFraction;
						m_llDelayFraction /= 8;

						m_bTraceRecieved = true;

						memset(psaEncodingAudioData, 0, sizeof(short) * unLength);

						MediaLog(LOG_DEBUG, "[ACS][ECHO][TS] TimeDelay = %lldms, DelayFra,  = %lld[Sample:%d] iTraceInFrame = %d m_iSpeakerType = %d",
							m_llDelay, m_llDelayFraction, m_iDelayFractionOrig, iTraceInFrame, m_pAudioCallSession->m_iSpeakerType);

						if (iTraceInFrame < 0)
						{
							long long llTS;
							for (int i = 0; i > iTraceInFrame; i--)
							{
								MediaLog(LOG_DEBUG, "[ACS][ECHO][TS] discarding farend");
								int iFarendDataLength = m_pAudioCallSession->m_FarendBuffer->DeQueue(m_saFarendData, llTS);
							}
						}
					}
				}

				MediaLog(LOG_DEBUG, "[NE][ACS] HandleTrace->IsEchoCancellerEnabled->Trace handled->m_llDelayFraction : %lld", m_llDelayFraction);
				
			}
		}
	}

	void AudioNearEndDataProcessor::DeleteDataB4TraceIsReceived(short *psaEncodingAudioData, unsigned int unLength)
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Delete Data Before Trace Received, length: %d",unLength);
		if (m_bTraceSent && !m_bTraceRecieved && !m_bTraceWillNotBeReceived)
		{
			MediaLog(LOG_DEBUG, "[NE][ACS] DeleteDataB4TraceIsReceived->m_bTraceRecieved");
			memset(m_saAudioTraceRemovalBuffer, TR_TRACE_SENT, sizeof(short) * unLength);
			memset(psaEncodingAudioData, 0, sizeof(short) * unLength);
		}
		else if (!m_bTraceSent && !m_bTraceRecieved && !m_bTraceWillNotBeReceived)
		{
			memset(m_saAudioTraceRemovalBuffer, TR_TRACE_NOT_SENT, sizeof(short) * unLength);
			memset(psaEncodingAudioData, 0, sizeof(short) * unLength);
		}
		
	}

	void AudioNearEndDataProcessor::DeleteDataAfterTraceIsReceived(short *psaEncodingAudioData, unsigned int unLength)
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Delete Data After Trace Received, length: %d", unLength);
		if (m_iDeleteCount > 0)
		{
			MediaLog(LOG_DEBUG, "[NE][ACS] DeleteDataAfterTraceIsReceived->IsEchoCancellerEnabled->Trace Recieved %d, m_iDeleteCount = %d", m_bTraceRecieved, m_iDeleteCount);
			if (m_bTraceRecieved == true)
			{
				if (m_bJustWrittenTraceDump == false)
				{
					m_bJustWrittenTraceDump = true;
					memset(m_saAudioTraceRemovalBuffer, TR_TRACE_SENT, sizeof(short) * (m_iDelayFractionOrig));
					memset(m_saAudioTraceRemovalBuffer + m_iDelayFractionOrig, TR_TRACE_RECVD, sizeof(short) * (unLength - m_iDelayFractionOrig));
				}
			}
			else
			{
				memset(m_saAudioTraceRemovalBuffer, TR_TRACE_WONT_BE_RECVD, sizeof(short) * unLength);
			}
			memset(psaEncodingAudioData, 0, sizeof(short) * unLength);
			memset(m_saFarendData, 0, sizeof(short) * unLength);
			m_iDeleteCount--;
		}
	}

	bool AudioNearEndDataProcessor::IsTraceSendingEnabled()
	{
#ifdef USE_AECM
#if defined (__ANDROID__) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
		if (m_pAudioCallSession->GetSpeakerType() == AUDIO_PLAYER_LOUDSPEAKER)
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
		return false;
#endif
	}

	bool AudioNearEndDataProcessor::IsKichCutterEnabled()
	{		
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined (DESKTOP_C_SHARP)
		return false;
#else
		if (IsEchoCancellerEnabled() &&
			(!m_pAudioCallSession->getIsAudioLiveStreamRunning() ||
			(m_pAudioCallSession->getIsAudioLiveStreamRunning() && (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER || m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE))))
		{
			if (IsTraceSendingEnabled() && m_bTraceRecieved)
			{
				return true;
			}
			else if (!IsTraceSendingEnabled())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
#endif
	}

	bool AudioNearEndDataProcessor::IsEchoCancellerEnabled()
	{		
#ifdef USE_AECM
#if defined (__ANDROID__) || defined (DESKTOP_C_SHARP)
		if (!m_pAudioCallSession->getIsAudioLiveStreamRunning() || (m_pAudioCallSession->getIsAudioLiveStreamRunning() && (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER || m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)))
		{
			return true;
		}
		else
		{
			return false;
		}
#endif
#else
		return false;
#endif
	}

	void AudioNearEndDataProcessor::ResetKichCutter()
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Resetting Kich Cutter");
		if (m_pKichCutter != nullptr)
		{
			delete m_pKichCutter;
		}

		m_pKichCutter = new CKichCutter();
	}

	void AudioNearEndDataProcessor::ResetAEC()
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Resetting AEC");
		if (m_pAudioCallSession->GetEchoCanceler().get())
		{
			m_pAudioCallSession->GetEchoCanceler().reset();
		}

		m_pAudioCallSession->SetEchoCanceller(EchoCancellerProvider::GetEchoCanceller(WebRTC_ECM, m_pAudioCallSession->getIsAudioLiveStreamRunning()));
	}

	void AudioNearEndDataProcessor::ResetNS()
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Resetting NS");
		if (m_pAudioCallSession->GetNoiseReducer().get())
		{
			m_pAudioCallSession->GetNoiseReducer()->Reset();
		}
	}

	void AudioNearEndDataProcessor::ResetRecorderGain()
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Resetting Recorder Gain");
		if (m_pAudioCallSession->GetRecorderGain().get())
		{
			m_pAudioCallSession->GetRecorderGain().reset();
		}

		m_pAudioCallSession->SetRecorderGain(AudioGainInstanceProvider::GetAudioGainInstance(WebRTC_Gain));
		m_pAudioCallSession->GetRecorderGain()->Init(m_nServiceType);
		if (m_nEntityType == ENTITY_TYPE_PUBLISHER && m_nServiceType == SERVICE_TYPE_LIVE_STREAM)
		{
			//Gain level is incremented to recover losses due to noise.
			//And noise is only applied to publisher NOT in call.
			m_pAudioCallSession->GetRecorderGain()->SetGain(DEFAULT_GAIN + 1);
		}
	}

	void AudioNearEndDataProcessor::ResetAudioEffects()
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Resetting Audio Effects");
		ResetAEC();
		ResetNS();
		ResetKichCutter();
		ResetRecorderGain();
		ResetTrace(); //Trace related variables should be reset last to avoid certain race conditions
	}

	void AudioNearEndDataProcessor::SyncRecordingTime()
	{		
		if (m_b1stRecordedDataSinceCallStarted)
		{
			Tools::SOSleep(RECORDER_STARTING_SLEEP_IN_MS);
			m_ll1stRecordedDataTime = Tools::CurrentTimestamp();
			m_llnextRecordedDataTime = m_ll1stRecordedDataTime + 100;
			m_b1stRecordedDataSinceCallStarted = false;
			MediaLog(LOG_CODE_TRACE, "[NE][ACS][TS] SyncRecordingTime , 1st time,  ts = %lld", m_ll1stRecordedDataTime);
		}
		else
		{
			long long llNOw = Tools::CurrentTimestamp();
			if (llNOw + 20 < m_llnextRecordedDataTime)
			{
				MediaLog(LOG_CODE_TRACE, "[NE][ACS][TS] SyncRecordingTime , nth time,  ts = %lld sleeptime = %lld", llNOw, m_llnextRecordedDataTime - llNOw - 20);
				Tools::SOSleep(m_llnextRecordedDataTime - llNOw - 20);
			}
			else
			{
				MediaLog(LOG_CODE_TRACE, "[NE][ACS][TS] SyncRecordingTime , nth time,  ts = %lld sleeptime = 0", llNOw);
			}
			m_llnextRecordedDataTime += 100;
		}
	}

	int AudioNearEndDataProcessor::PreprocessAudioData(short *psaEncodingAudioData, unsigned int unLength)
	{
		m_pAudioSessionStatistics->UpdateEchoDelay(m_llDelay, m_llDelayFraction);

		m_pRecordedNE->WriteDump(psaEncodingAudioData, 2, unLength);

		long long llCurrentTime = Tools::CurrentTimestamp();

		int nEchoStateFlags = 0;
		bool bIsNsWorking = false;

		if (m_nEntityType == ENTITY_TYPE_PUBLISHER && m_nServiceType == SERVICE_TYPE_LIVE_STREAM)
		{
			if (m_pAudioCallSession->GetNoiseReducer().get())
			{
				bIsNsWorking = true;
				m_pAudioCallSession->GetNoiseReducer()->Denoise(psaEncodingAudioData, unLength, psaEncodingAudioData, 0);
			}
		}


#ifndef DESKTOP_C_SHARP
		bool bIsGainWorking = (m_pAudioCallSession->GetSpeakerType() == AUDIO_PLAYER_LOUDSPEAKER && m_pAudioCallSession->GetRecorderGain().get());
#else
		bool bIsGainWorking = m_pAudioCallSession->GetRecorderGain().get();
#endif
		
		bool bIsEchoCanceller = IsEchoCancellerEnabled();
		bool bIsKitchCutter = IsKichCutterEnabled();
		bool bIsTraceSending = IsTraceSendingEnabled();

		MediaLog(LOG_DEBUG, "[NE][ACS][GAIN][NS] PreprocessAudioData# CurrentTime=%lld, IsGainWorking=%d, IsNS=%d, IsWebRtcAECM=%d, IsKitchCutter=%d, IsTraceSending=%d"
			, llCurrentTime, bIsGainWorking, bIsNsWorking, bIsEchoCanceller, bIsKitchCutter, bIsTraceSending);

		if (IsEchoCancellerEnabled())
		{
			MediaLog(LOG_CODE_TRACE, "[NE][ACS][ECHO] AECM Working!!! IsTimeSyncEnabled = %d", m_bEnableRecorderTimeSyncDuringEchoCancellation);

			if (m_bNeedToResetAudioEffects)
			{
				MediaLog(LOG_DEBUG, "[NE][ACS][TS] Resetting AudioEffects.");
				ResetAudioEffects();
				m_bNeedToResetAudioEffects = false;
			}
			//Sleep to maintain 100 ms recording time diff
			long long llb4Time = Tools::CurrentTimestamp();
			if (m_bEnableRecorderTimeSyncDuringEchoCancellation)
			{
				SyncRecordingTime();
			}

			//Handle Trace
			HandleTrace(psaEncodingAudioData, unLength);
			//Some frames are deleted after detectiing trace, whether or not detection succeeds
			memcpy(m_saAudioTraceRemovalBuffer, psaEncodingAudioData, unLength * sizeof(short));
			DeleteDataB4TraceIsReceived(psaEncodingAudioData, unLength);

			if (m_pAudioCallSession->GetEchoCanceler().get() && (m_bTraceRecieved || m_bTraceWillNotBeReceived))
			{
				DeleteDataAfterTraceIsReceived(psaEncodingAudioData, unLength);
			}
			m_pTraceRemoved->WriteDump(m_saAudioTraceRemovalBuffer, 2, unLength);

#ifdef DUMP_FILE
			fwrite(psaEncodingAudioData, 2, unLength, FileInputWithEcho);
#endif //DUMP_FILE

			if (m_pAudioCallSession->GetEchoCanceler().get() && (m_bTraceRecieved || m_bTraceWillNotBeReceived))
			{
				long long llTS;
				if (m_iStartingBufferSize == -1)
				{
					m_iStartingBufferSize = m_pAudioCallSession->m_FarendBuffer->GetQueueSize();
					MediaLog(LOG_DEBUG, "[NE][ACS][ECHO][GAIN] First Time Updated m_iStartingBufferSize = %d", m_iStartingBufferSize);
					m_pAudioSessionStatistics->UpdateStartingBufferSize(m_iStartingBufferSize);
				}

				int iFarendDataLength = m_pAudioCallSession->m_FarendBuffer->DeQueue(m_saFarendData, llTS);
				int nFarEndBufferSize = m_pAudioCallSession->m_FarendBuffer->GetQueueSize();

				MediaLog(LOG_DEBUG, "[NE][ACS][ECHO][GAIN] DataLength=%dS, FarBufSize=%d[%d], IsGainWorking=%d", iFarendDataLength, nFarEndBufferSize, m_iStartingBufferSize, bIsGainWorking);


				if (iFarendDataLength > 0)
				{
					//If trace is received, current and next frames are deleted
					
					
#if !defined(TARGET_OS_IPHONE) && !defined(TARGET_IPHONE_SIMULATOR)
					if (bIsGainWorking)
					{
						m_pAudioCallSession->GetRecorderGain()->AddFarEnd(m_saFarendData, unLength);
						m_pAudioCallSession->GetRecorderGain()->AddGain(psaEncodingAudioData, unLength, false, 0);
						m_pGainedNE->WriteDump(psaEncodingAudioData, 2, unLength);
					}
#endif
					long long llCurrentTimeStamp = Tools::CurrentTimestamp();
					long long llEchoLogTimeDiff = llCurrentTimeStamp - m_llLastEchoLogTime;
					m_llLastEchoLogTime = llCurrentTimeStamp;
					MediaLog(LOG_DEBUG, "[NE][ACS][ECHO] FarendBufferSize = %d, m_iStartingBufferSize = %d,"
						"m_llDelay = %lld, m_bTraceRecieved = %d llEchoLogTimeDiff = %lld, Time Taken = %lld, iFarendDataLength = %d FarBuffSize = %d",
						m_pAudioCallSession->m_FarendBuffer->GetQueueSize(), m_iStartingBufferSize, m_llDelay, m_bTraceRecieved,
						llEchoLogTimeDiff, llCurrentTimeStamp - llb4Time, iFarendDataLength, nFarEndBufferSize);

					m_pAudioSessionStatistics->UpdateCurrentBufferSize(m_pAudioCallSession->m_FarendBuffer->GetQueueSize());

					m_pAudioCallSession->GetEchoCanceler()->AddFarEndData(m_saFarendData, unLength);



					bool bTaceBasedEcho = true;
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
					bTaceBasedEcho = m_b30VerifiedTrace;					
#elif defined (__ANDROID__)
					bTaceBasedEcho = (m_b30VerifiedTrace && m_nServiceType == SERVICE_TYPE_CALL) || m_nServiceType == SERVICE_TYPE_LIVE_STREAM;
#endif

					if (IsKichCutterEnabled())
					{
						memcpy(m_saNoisyData, psaEncodingAudioData, unLength * sizeof(short));

						if (bIsNsWorking)
						{
							m_pAudioCallSession->GetNoiseReducer()->Denoise(psaEncodingAudioData, unLength, psaEncodingAudioData, m_pAudioCallSession->getIsAudioLiveStreamRunning());
						}
						m_pNoiseReducedNE->WriteDump(psaEncodingAudioData, 2, unLength);

						if (bTaceBasedEcho)
						{
							nEchoStateFlags = m_pAudioCallSession->GetEchoCanceler()->CancelEcho(psaEncodingAudioData, unLength, m_llDelayFraction + 10, m_saNoisyData);
						}
						m_pCancelledNE->WriteDump(psaEncodingAudioData, 2, unLength);
						nEchoStateFlags = m_pKichCutter->Despike(psaEncodingAudioData, nEchoStateFlags);
						m_pKichCutNE->WriteDump(psaEncodingAudioData, 2, unLength);
					}
					else
					{
						if (bTaceBasedEcho)
						{
							nEchoStateFlags = m_pAudioCallSession->GetEchoCanceler()->CancelEcho(psaEncodingAudioData, unLength, m_llDelayFraction + 10);
						}

						m_pCancelledNE->WriteDump(psaEncodingAudioData, 2, unLength);
					}
					//MediaLog(LOG_DEBUG, "[NE][ACS][ECHOFLAG] nEchoStateFlags = %d\n", nEchoStateFlags);

					m_pProcessedNE->WriteDump(psaEncodingAudioData, 2, unLength);
					MediaLog(LOG_DEBUG, "[NE][ACS][ECHO] Successful FarNear Interleave.");
				}
				else
				{
					MediaLog(LOG_DEBUG, "[NE][ACS][ECHO] UnSuccessful FarNear Interleave.");
				}

#ifdef DUMP_FILE
				fwrite(psaEncodingAudioData, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_pAudioCallSession->getIsAudioLiveStreamRunning()), FileInputPreGain);
#endif

			}

			m_pProcessed2NE->WriteDump(psaEncodingAudioData, 2, unLength);
		}
		else
		{
			if (bIsGainWorking)
			{
				MediaLog(LOG_CODE_TRACE, "[NE][ACS][GAIN] Recorder Gain Added.");
				m_pAudioCallSession->GetRecorderGain()->AddGain(psaEncodingAudioData, unLength, false, 0);
			}
		}
		return nEchoStateFlags;
	}

	void AudioNearEndDataProcessor::StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nFrameLengthInByte)
	{
		NearEndLockerStoreDataForChunk lock(*m_pAudioEncodingMutex);
		MediaLog(LOG_DEBUG, "[ANEDP][SDC] Relative time: %lld, Frame length: %d", llRelativeTime, nFrameLengthInByte);

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

	void AudioNearEndDataProcessor::StoreDataForChunk(unsigned char *uchNearData, int nNearFrameLengthInByte,
		unsigned char *uchFarData, int nFarFrameLengthInByte, long long llRelativeTime)
	{
		NearEndLockerStoreDataForChunk lock(*m_pAudioEncodingMutex);
		MediaLog(LOG_DEBUG, "[ANEDP][SDC] Relative time: %lld, Far Frame length: %d, Near Frame Length: %d", llRelativeTime, nFarFrameLengthInByte, nNearFrameLengthInByte);

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

	void AudioNearEndDataProcessor::ClearRecordBuffer()
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Clearing Record Buffer");
		m_recordBuffer->Clear();
	}

	void AudioNearEndDataProcessor::PushDataInRecordBuffer(short *data, int dataLen)
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Pushing data in record buffer, length: %d", dataLen);
		m_recordBuffer->PushData(data, dataLen);
		m_pAudioSessionStatistics->UpdateOnDataArrive(dataLen);
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

		MediaLog(LOG_CODE_TRACE, "[ANEDP] lastFrameRT: %lld, lastChunkLastFrameRT: %lld", m_llLastFrameRT, m_llLastChunkLastFrameRT);

		llAudioChunkDuration = m_llLastFrameRT - m_llLastChunkLastFrameRT;

		MediaLog(LOG_DEBUG, "[ANEDP] Audio Chunk Duration: %lld", llAudioChunkDuration);

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
		MediaLog(LOG_DEBUG, "[ANEDP] Starting call in live, Entity: %d", nEntityType);
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

		if (m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER || m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER)
			m_pAudioSessionStatistics->CallStarted();
	}

	void AudioNearEndDataProcessor::StopCallInLive(int nEntityType)
	{
		MediaLog(LOG_DEBUG, "[ANEDP] Starting call in live, Entity: %d", nEntityType);
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
