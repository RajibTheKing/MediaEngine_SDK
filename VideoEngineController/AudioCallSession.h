
#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#include "AudioResources.h"
#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "AudioPacketHeader.h"
#include "LogPrinter.h"
#include "LiveAudioDecodingQueue.h"
#include "AudioNearEndDataProcessor.h"
#include "AudioFarEndDataProcessor.h"
#include "AudioShortBufferForPublisherFarEnd.h"
#include "AudioMacros.h"
#include "AudioLinearBuffer.h"

#include <stdio.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <vector>
#include <thread>


//#define LOCAL_SERVER_LIVE_CALL
#ifdef LOCAL_SERVER_LIVE_CALL
#include "VideoSockets.h"
#endif

#define AUDIO_CALL_VERSION  0
#define AUDIO_LIVE_VERSION  0

namespace MediaSDK
{

	static string colon = "ALOG:";
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO,colon + a);



	//#define AUDIO_SELF_CALL
	//#define DUMP_FILE
	//#define PCM_DUMP


	//#define FIRE_ENC_TIME
	//#define AUDIO_FIXED_COMPLEXITY

	class CCommonElementsBucket;
	class CVideoEncoder;
	class CAudioCodec;
	class AudioDePacketizer;
	class AudioFarEndDataProcessor;

	class AudioEncoderInterface;
	class AudioDecoderInterface;
	class EchoCancellerInterface;
	class NoiseReducerInterface;
	class AudioGainInterface;
	class CEventNotifier;

	class AudioNearEndProcessorThread;
	class AudioFarEndProcessorThread;
	class AudioPlayingThread;
	class CTrace;


#ifdef USE_VAD
	class CVoice;
#endif

	class CAudioCallSession : public DataReadyListenerInterface,
		public PacketEventListener,
		public DataEventListener,
		public NetworkChangeListener,
		public AudioAlarmListener
	{
	private:
		bool m_bIsAECMFarEndThreadBusy;
		bool m_bIsAECMNearEndThreadBusy;
		long long m_llLastPlayTime;

		AudioNearEndProcessorThread *m_cNearEndProcessorThread;
		AudioFarEndProcessorThread *m_cFarEndProcessorThread;

		SmartPointer<CLockHandler> m_pAudioCallSessionMutex;


	public:

		CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, int nEntityType, AudioResources &audioResources, int nAudioSpeakerType);
		~CAudioCallSession();

		void StartCallInLive(int iRole, int nCallInLiveType);
		void EndCallInLive();

		void SetCallInLiveType(int nCallInLiveType);
		long long GetBaseOfRelativeTime();


		void PreprocessAudioData(short *psaEncodingAudioData, unsigned int unLength);

		bool m_bEnableRecorderTimeSyncDuringEchoCancellation;
		bool m_bEnablePlayerTimeSyncDuringEchoCancellation;

		int PushAudioData(short *psaEncodingAudioData, unsigned int unLength);
		int CancelAudioData(short *psaEncodingAudioData, unsigned int unLength);

		int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);

		void SetVolume(int iVolume, bool bRecorder);
		void SetSpeakerType(int iSpeakerType);
		void SetEchoCanceller(bool bOn);
		void ResetTrace();
		void ResetAEC();
		bool m_bIsPublisher;

		bool m_bRecordingStarted;
		long long m_llTraceSendingTime;
		long long m_llTraceReceivingTime;
		bool m_bTraceSent, m_bTraceRecieved, m_bTraceWillNotBeReceived;
		bool m_bTraceTailRemains;
		long long m_llDelay, m_llDelayFraction;
		int  m_iDeleteCount;
		int m_nFramesRecvdSinceTraceSent;
		bool m_b1stRecordedDataSinceCallStarted;
		long long m_ll1stRecordedDataTime;
		long long m_llnextRecordedDataTime;
		CAudioShortBuffer  m_FarendBuffer;
		short m_saFarendData[MAX_AUDIO_FRAME_Length];
		AudioLinearBuffer* m_recordBuffer = nullptr;

		CTrace * m_pTrace;

		void GetAudioSendToData(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
			int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);

		void SetSendFunction(SendFunctionPointerType cbClientSendFunc)
		{
			m_cbClientSendFunction = cbClientSendFunc;
		}

		void SetEventNotifier(CEventNotifier *pEventNotifier)
		{
			m_pEventNotifier = pEventNotifier;
		}

		bool IsTraceSendingEnable(){ return m_bTraceSendingEnabled; }

		int m_iNextPacketType;
		CAudioShortBuffer m_AudioNearEndBuffer;
		CAudioShortBuffer  m_ViewerInCallSentDataQueue;

		AudioShortBufferForPublisherFarEnd m_PublisherBufferForMuxing;
		int m_iPrevRecvdSlotID;
		int m_iReceivedPacketsInPrevSlot;

		AudioNearEndDataProcessor *m_pNearEndProcessor = NULL;
		AudioFarEndDataProcessor *m_pFarEndProcessor = NULL;

#ifdef PCM_DUMP
		FILE* RecordedFile;
		FILE* EchoCancelledFile;
		FILE* AfterEchoCancellationFile;
		FILE* PlayedFile;
#endif

#ifdef DUMP_FILE
		FILE *FileInput;
		FILE *FileOutput;
		FILE *File18BitType;
		FILE *File18BitData;
		FILE *FileInputWithEcho;
		FILE *FileInputMuxed;
		FILE *FileInputPreGain;
#endif

	private:

		int m_iRole;
		Tools m_Tools;
		LongLong m_FriendID;
		bool m_bEchoCancellerEnabled;
		bool m_bTraceSendingEnabled;

		CCommonElementsBucket* m_pCommonElementsBucket;

		int m_iSpeakerType;

		bool m_bLiveAudioStreamRunning;
		int m_nServiceType;
		int m_nEntityType;

		int m_iVolume;

		int m_iAudioVersionFriend;
		int m_iAudioVersionSelf;

		int m_nCallInLiveType;

		void OnDataReadyToSend(int mediaType, unsigned char* dataBuffer, size_t dataLength);

		void FirePacketEvent(int eventType, size_t dataLength, unsigned char* dataBuffer);
		void FireDataEvent(int eventType, size_t dataLength, short* dataBuffer);
		void FireNetworkChange(int eventType);
		void FireAudioAlarm(int eventType);

		SendFunctionPointerType m_cbClientSendFunction;
		CEventNotifier* m_pEventNotifier;

#ifdef USE_VAD
		CVoice *m_pVoice;
#endif
#ifdef LOCAL_SERVER_LIVE_CALL
		VideoSockets *m_clientSocket;
#endif

	private:

		SmartPointer<AudioPacketHeader> m_pAudioNearEndPacketHeader;
		SmartPointer<AudioPacketHeader> m_pAudioFarEndPacketHeader;

		SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
		SmartPointer<AudioDecoderInterface> m_pAudioDecoder;

		SmartPointer<EchoCancellerInterface> m_pEcho;
		SmartPointer<NoiseReducerInterface> m_pNoiseReducer;

		SmartPointer<AudioGainInterface> m_pPlayerGain;


	protected:

		void SetResources(AudioResources &audioResources);

		void InitNearEndDataProcessing();
		void InitFarEndDataProcessing(CCommonElementsBucket* pSharedObject);


	public:

		void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
		void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llNow, long long &llLastTime, int iCurrentPacketNumber);

		int GetRole()        { return m_iRole; }
		int GetServiceType() { return m_nServiceType; }
		int GetEntityType()  { return m_nEntityType; }
		bool getIsAudioLiveStreamRunning() { return m_bLiveAudioStreamRunning; }

		SmartPointer<AudioPacketHeader> GetAudioNearEndPacketHeader() { return m_pAudioNearEndPacketHeader; }
		SmartPointer<AudioPacketHeader> GetAudioFarEndPacketHeader() { return m_pAudioFarEndPacketHeader; }

		SmartPointer<AudioEncoderInterface> GetAudioEncoder() { return m_pAudioEncoder; }
		SmartPointer<AudioDecoderInterface> GetAudioDecoder() { return m_pAudioDecoder; }

		SmartPointer<EchoCancellerInterface> GetEchoCanceler() { return m_pEcho; }
		SmartPointer<NoiseReducerInterface> GetNoiseReducer() { return m_pNoiseReducer; }

		SmartPointer<AudioGainInterface> GetPlayerGain() { return m_pPlayerGain; }
	};

} //namespace MediaSDK

#endif
