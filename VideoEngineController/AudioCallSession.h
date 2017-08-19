#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_


#include <vector>

#include "SmartPointer.h"
#include "AudioTypes.h"
#include "CommonTypes.h"
#include "Size.h"
#include "MediaLogger.h"

#define PCM_DUMP

namespace MediaSDK
{
    #define AUDIO_CALL_VERSION  0
    #define AUDIO_LIVE_VERSION  0
	
	
	
	//#define LOCAL_SERVER_LIVE_CALL
	//#define AUDIO_SELF_CALL
	//#define DUMP_FILE
	//#define FIRE_ENC_TIME
	//#define AUDIO_FIXED_COMPLEXITY
    //#define PCM_DUMP

	class AudioEncoderInterface;
	class AudioDecoderInterface;
	class EchoCancellerInterface;
	class NoiseReducerInterface;
	class AudioGainInterface;

	class CVoice;
	class CEventNotifier;
	class CCommonElementsBucket;
	class AudioPacketHeader;
	class AudioNearEndDataProcessor;
	class AudioFarEndDataProcessor;
	class AudioNearEndProcessorThread;
	class AudioFarEndProcessorThread;
	class VideoSockets;
	class AudioResources;
	class AudioShortBufferForPublisherFarEnd;
	class CAudioShortBuffer;
	class CTrace;
	class AudioLinearBuffer;


	class CAudioCallSession : 
		public DataReadyListenerInterface,
		public PacketEventListener,
		public DataEventListener,
		public NetworkChangeListener,
		public AudioAlarmListener
	{

	public:

		CAudioCallSession(const bool& isVideoCallRunning, long long llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, int nEntityType, AudioResources &audioResources, int nAudioSpeakerType);
		~CAudioCallSession();

		void StartCallInLive(int iRole, int nCallInLiveType);
		void EndCallInLive();

		void PreprocessAudioData(short *psaEncodingAudioData, unsigned int unLength);
		int PushAudioData(short *psaEncodingAudioData, unsigned int unLength);
		int CancelAudioData(short *psaEncodingAudioData, unsigned int unLength);
		int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
		void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
		void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llNow, long long &llLastTime, int iCurrentPacketNumber);

		long long GetBaseOfRelativeTime();
		void GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector, int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);

		void SetCallInLiveType(int nCallInLiveType);
		void SetVolume(int iVolume, bool bRecorder);
		void SetSpeakerType(int iSpeakerType);
		void SetEchoCanceller(bool bOn);
		void ResetTrace();
		void ResetAEC();
		bool IsEchoCancellerEnabled();
		bool IsTraceSendingEnabled();
		
		void SetSendFunction(SendFunctionPointerType cbClientSendFunc) { m_cbClientSendFunction = cbClientSendFunc; }
		void SetEventNotifier(CEventNotifier *pEventNotifier)          { m_pEventNotifier = pEventNotifier; }

		int GetRole()        { return m_iRole; }
		int GetServiceType() { return m_nServiceType; }
		int GetEntityType()  { return m_nEntityType; }
		bool getIsAudioLiveStreamRunning() { return m_bLiveAudioStreamRunning; }
		bool GetIsVideoCallRunning() { return m_bIsVideoCallRunning; }
		

		SmartPointer<AudioPacketHeader> GetAudioNearEndPacketHeader() { return m_pAudioNearEndPacketHeader; }
		SmartPointer<AudioPacketHeader> GetAudioFarEndPacketHeader()  { return m_pAudioFarEndPacketHeader; }
		SmartPointer<AudioEncoderInterface> GetAudioEncoder()         { return m_pAudioEncoder; }
		SmartPointer<AudioDecoderInterface> GetAudioDecoder()         { return m_pAudioDecoder; }
		SmartPointer<EchoCancellerInterface> GetEchoCanceler()        { return m_pEcho; }
		SmartPointer<NoiseReducerInterface> GetNoiseReducer()         { return m_pNoiseReducer; }
		SmartPointer<AudioGainInterface> GetRecorderGain()            { return m_pRecorderGain; }
		SmartPointer<AudioGainInterface> GetPlayerGain()              { return m_pPlayerGain; }


	private:

		void OnDataReadyToSend(int mediaType, unsigned char* dataBuffer, size_t dataLength);
		void FirePacketEvent(int eventType, size_t dataLength, unsigned char* dataBuffer);
		void FireDataEvent(int eventType, size_t dataLength, short* dataBuffer);
		void FireNetworkChange(int eventType);
		void FireAudioAlarm(int eventType);

		void SetResources(AudioResources &audioResources);
		void InitNearEndDataProcessing();
		void InitFarEndDataProcessing();
		void SyncRecordingTime();


	public:

		bool m_bIsPublisher;
		bool m_bRecordingStarted;
		bool m_bEnablePlayerTimeSyncDuringEchoCancellation;
		bool m_bEnableRecorderTimeSyncDuringEchoCancellation;

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
		short m_saFarendData[MAX_AUDIO_FRAME_Length];
		
		int m_iNextPacketType;
		int m_iPrevRecvdSlotID;
		int m_iReceivedPacketsInPrevSlot;

		AudioNearEndDataProcessor *m_pNearEndProcessor = NULL;
		AudioFarEndDataProcessor *m_pFarEndProcessor = NULL;
		SmartPointer<CAudioShortBuffer> m_FarendBuffer;
		SmartPointer<CAudioShortBuffer> m_AudioNearEndBuffer;
		SmartPointer<CAudioShortBuffer> m_ViewerInCallSentDataQueue;
		SmartPointer<AudioShortBufferForPublisherFarEnd> m_PublisherBufferForMuxing;

		CTrace *m_pTrace;
		AudioLinearBuffer* m_recordBuffer = nullptr;

#ifdef PCM_DUMP
		FILE* RecordedFile;
		FILE* RecordedChunckedFile;
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
		
		SendFunctionPointerType m_cbClientSendFunction;

		bool m_bNeedToResetEcho;

		bool m_bUsingLoudSpeaker;
		bool m_bLiveAudioStreamRunning;
		const bool& m_bIsVideoCallRunning;

		int m_iRole;
		int m_iVolume;
		int m_nServiceType;
		int m_nEntityType;
		int m_iSpeakerType;
		int m_iAudioVersionFriend;
		int m_iAudioVersionSelf;
		int m_nCallInLiveType;
		int m_iStartingBufferSize;
		int m_iDelayFractionOrig;

		long long m_llLastPlayTime;
		long long m_FriendID;


		CEventNotifier* m_pEventNotifier;
		AudioNearEndProcessorThread *m_cNearEndProcessorThread;
		AudioFarEndProcessorThread *m_cFarEndProcessorThread;
		CCommonElementsBucket* m_pCommonElementsBucket;
		SmartPointer<CLockHandler> m_pAudioCallSessionMutex;

		SmartPointer<AudioPacketHeader> m_pAudioNearEndPacketHeader;
		SmartPointer<AudioPacketHeader> m_pAudioFarEndPacketHeader;
		SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
		SmartPointer<AudioDecoderInterface> m_pAudioDecoder;
		SmartPointer<EchoCancellerInterface> m_pEcho;
		SmartPointer<NoiseReducerInterface> m_pNoiseReducer;
		SmartPointer<AudioGainInterface> m_pRecorderGain;
		SmartPointer<AudioGainInterface> m_pPlayerGain;
		
	#ifdef USE_VAD
		CVoice *m_pVoice;
	#endif
	#ifdef LOCAL_SERVER_LIVE_CALL
		VideoSockets *m_clientSocket;
	#endif


	};

} //namespace MediaSDK

#endif
