#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_


#include <vector>

#include "SmartPointer.h"
#include "AudioTypes.h"
#include "CommonTypes.h"
#include "Size.h"
#include "MediaLogger.h"
#include "AudioMacros.h"



namespace MediaSDK
{
    #define AUDIO_CALL_VERSION  0
    #define AUDIO_LIVE_VERSION  0
		
	//#define LOCAL_SERVER_LIVE_CALL
	//#define AUDIO_SELF_CALL
	//#define DUMP_FILE
	//#define FIRE_ENC_TIME
	//#define AUDIO_FIXED_COMPLEXITY

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
	class CAudioByteBuffer;


	class CAudioCallSession : 
		public DataReadyListenerInterface,
		public PacketEventListener,
		public DataEventListener,
		public NetworkChangeListener,
		public AudioAlarmListener
	{

	public:

		CAudioCallSession(const bool& isVideoCallRunning, long long llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, int nEntityType, AudioResources &audioResources, int nAudioSpeakerType, bool bOpusCodec);
		~CAudioCallSession();

		void StartCallInLive(int iRole, int nCallInLiveType);
		void EndCallInLive();

		int PreprocessAudioData(short *psaEncodingAudioData, unsigned int unLength);
		int PushAudioData(short *psaEncodingAudioData, unsigned int unLength);
		int CancelAudioData(short *psaEncodingAudioData, unsigned int unLength);
		int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
		void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
		void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llNow, long long &llLastTime, int iCurrentPacketNumber, int nEchoStateFlags);

		long long GetBaseOfRelativeTime();
		void GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector, int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);
		unsigned int GetNumberOfFrameForChunk();

		void SetCallInLiveType(int nCallInLiveType);
		void SetVolume(int iVolume, bool bRecorder);
		void SetSpeakerType(int iSpeakerType);
		void SetEchoCanceller(bool bOn);

		/**
		Sets the quality of the audio. Quality adaption is done when server signals network strength.
		Server considers strength as STRONG(3) when packet loss is 0%-3%, MEDIUM(2) when loss is 4%-12% and WEAK(1) when loss is 13%-25%

		@param [in] level The current quality level of the audio. Possible values are 3 for best, 2 for good, 1 for normal.

		@return Returns true when sets quality successfully, false otherwise.
		*/
		bool SetAudioQuality(int level);

		void ResetTrace();
		void ResetAEC();
		void HandleTrace(short *psaEncodingAudioData, unsigned int unLength);
		void DeleteBeforeHandlingTrace(short *psaEncodingAudioData, unsigned int unLength);
		void DeleteAfterHandlingTrace(short *psaEncodingAudioData, unsigned int unLength);
		bool IsEchoCancellerEnabled();
		bool IsTraceSendingEnabled();
		
		void SetSendFunction(SendFunctionPointerType cbClientSendFunc) { m_cbClientSendFunction = cbClientSendFunc; }
		void SetEventNotifier(CEventNotifier *pEventNotifier)          { m_pEventNotifier = pEventNotifier; }

		int GetRole()        { return m_iRole; }
		int GetServiceType() { return m_nServiceType; }
		int GetEntityType()  { return m_nEntityType; }
		bool getIsAudioLiveStreamRunning() { return m_bLiveAudioStreamRunning; }
		bool GetIsVideoCallRunning() { return m_bIsVideoCallRunning; }

		bool IsOpusEnable()	{ return m_bIsOpusCodec; }
		

		SharedPointer<AudioPacketHeader> GetAudioNearEndPacketHeader() { return m_pAudioNearEndPacketHeader; }
		SharedPointer<AudioPacketHeader> GetAudioFarEndPacketHeader()  { return m_pAudioFarEndPacketHeader; }
		SharedPointer<AudioEncoderInterface> GetAudioEncoder()         { return m_pAudioEncoder; }
		SharedPointer<AudioDecoderInterface> GetAudioDecoder()         { return m_pAudioDecoder; }
		SharedPointer<EchoCancellerInterface> GetEchoCanceler()        { return m_pEcho; }
		SharedPointer<NoiseReducerInterface> GetNoiseReducer()         { return m_pNoiseReducer; }
		SharedPointer<AudioGainInterface> GetRecorderGain()            { return m_pRecorderGain; }
		SharedPointer<AudioGainInterface> GetPlayerGain()              { return m_pPlayerGain; }


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
		long long m_llDelay, m_llDelayFraction, m_llLastEchoLogTime = 0;
		int  m_iDeleteCount;
		int m_nFramesRecvdSinceTraceSent;
		bool m_b1stRecordedDataSinceCallStarted;
		long long m_ll1stRecordedDataTime;
		long long m_llnextRecordedDataTime;
		short m_saFarendData[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		
		int m_iNextPacketType;
		int m_iPrevRecvdSlotID;
		int m_iReceivedPacketsInPrevSlot;

		AudioNearEndDataProcessor *m_pNearEndProcessor = NULL;
		AudioFarEndDataProcessor *m_pFarEndProcessor = NULL;
		SharedPointer<CAudioShortBuffer> m_FarendBuffer;
		SharedPointer<CAudioShortBuffer> m_AudioNearEndBuffer;
		SharedPointer<CAudioShortBuffer> m_ViewerInCallSentDataQueue;
		SharedPointer<AudioShortBufferForPublisherFarEnd> m_PublisherBufferForMuxing;
		SharedPointer<CAudioByteBuffer> m_FarEndBufferOpus;

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
		
		bool m_bIsOpusCodec;
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
		SharedPointer<CLockHandler> m_pAudioCallSessionMutex;

		SharedPointer<AudioPacketHeader> m_pAudioNearEndPacketHeader;
		SharedPointer<AudioPacketHeader> m_pAudioFarEndPacketHeader;
		SharedPointer<AudioEncoderInterface> m_pAudioEncoder;
		SharedPointer<AudioDecoderInterface> m_pAudioDecoder;
		SharedPointer<EchoCancellerInterface> m_pEcho;
		SharedPointer<NoiseReducerInterface> m_pNoiseReducer;
		SharedPointer<AudioGainInterface> m_pRecorderGain;
		SharedPointer<AudioGainInterface> m_pPlayerGain;
		
	#ifdef USE_VAD
		CVoice *m_pVoice;
	#endif
	#ifdef LOCAL_SERVER_LIVE_CALL
		VideoSockets *m_clientSocket;
	#endif


	};

} //namespace MediaSDK

#endif
