#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_


#include <vector>

#include "SmartPointer.h"
#include "AudioTypes.h"
#include "CommonTypes.h"
#include "Size.h"
#include "MediaLogger.h"
#include "AudioMacros.h"
#include "AudioDumper.h"


namespace MediaSDK
{
    #define AUDIO_CALL_VERSION  0
    #define AUDIO_LIVE_VERSION  1
		
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
	class CAudioByteBuffer;
	class SessionStatisticsInterface;
	class CAudioCallInfo;

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
		void SetCameraMode(bool bCameraEnable);
		void SetMicrophoneMode(bool bMicrophoneEnable);
		void SetEchoCanceller(bool bOn);

		void NotifyTraceInfo(int nTR, int nNTR, int sDelay);
		void SetTraceInfo(int nTraceInfoLength, int * npTraceInfo);

		/**
		Sets the quality of the audio. Quality adaption is done when server signals network strength.
		Server considers strength as STRONG(3) when packet loss is 0%-3%, MEDIUM(2) when loss is 4%-12% and WEAK(1) when loss is 13%-25%

		@param [in] level The current quality level of the audio. Possible values are 3 for best, 2 for good, 1 for normal.

		@return Returns true when sets quality successfully, false otherwise.
		*/
		bool SetAudioQuality(int level);

		void SetSendFunction(SendFunctionPointerType cbClientSendFunc) { m_cbClientSendFunction = cbClientSendFunc; }
		void SetEventNotifier(CEventNotifier *pEventNotifier)          { m_pEventNotifier = pEventNotifier; }

		int GetRole()        { return m_iRole; }
		int GetServiceType() { return m_nServiceType; }
		int GetEntityType()  { return m_nEntityType; }
		int GetSpeakerType() { return m_iSpeakerType;  }
		bool getIsAudioLiveStreamRunning() { return m_bLiveAudioStreamRunning; }
		bool GetIsVideoCallRunning() { return m_bIsVideoCallRunning; }
		void SetRecordingStarted(bool flag) { m_bRecordingStarted = flag; }

		bool IsOpusEnable()	{ return m_bIsOpusCodec; }
		

		SharedPointer<AudioPacketHeader> GetAudioNearEndPacketHeader() { return m_pAudioNearEndPacketHeader; }
		SharedPointer<AudioPacketHeader> GetAudioFarEndPacketHeader()  { return m_pAudioFarEndPacketHeader; }
		SharedPointer<AudioEncoderInterface> GetAudioEncoder()         { return m_pAudioEncoder; }
		SharedPointer<AudioDecoderInterface> GetAudioDecoder()         { return m_pAudioDecoder; }
		SharedPointer<EchoCancellerInterface> GetEchoCanceler()        { return m_pEcho; }
		SharedPointer<NoiseReducerInterface> GetNoiseReducer()         { return m_pNoiseReducer; }
		SharedPointer<AudioGainInterface> GetRecorderGain()            { return m_pRecorderGain; }
		SharedPointer<AudioGainInterface> GetPlayerGain()              { return m_pPlayerGain; }
		SessionStatisticsInterface *GetSessionStatListener();
		void SetNoiseReducer(SharedPointer<NoiseReducerInterface> Noise)   { m_pNoiseReducer = Noise; }
		void SetEchoCanceller(SharedPointer<EchoCancellerInterface> Echo)  { m_pEcho = Echo; }
		void SetRecorderGain(SharedPointer<AudioGainInterface> Gain)	   { m_pRecorderGain = Gain; }

	private:

		void OnDataReadyToSend(int mediaType, unsigned char* dataBuffer, size_t dataLength);
		void FirePacketEvent(int eventType, size_t dataLength, unsigned char* dataBuffer);
		void FireDataEvent(int eventType, size_t dataLength, short* dataBuffer);
		void FireNetworkChange(int eventType);
		void FireAudioAlarm(int eventType, size_t dataLength, int* dataBuffer);

		void SetResources(AudioResources &audioResources);
		void InitNearEndDataProcessing();
		void InitFarEndDataProcessing();


	public:

		bool m_bRecordingStarted;
		bool m_bEnablePlayerTimeSyncDuringEchoCancellation;
		
		//int m_iPrevRecvdSlotID;
		int m_iReceivedPacketsInPrevSlot;

		int m_nNumTraceReceived = 0, m_nNumTraceNotReceived = 0, m_nSumDelay = 0;
		float m_fAvgDelay = 0;

		AudioNearEndDataProcessor *m_pNearEndProcessor = NULL;
		AudioFarEndDataProcessor *m_pFarEndProcessor = NULL;
		SharedPointer<CAudioShortBuffer> m_FarendBuffer;
		SharedPointer<CAudioShortBuffer> m_AudioNearEndBuffer;
		SharedPointer<CAudioShortBuffer> m_ViewerInCallSentDataQueue;
		SharedPointer<AudioShortBufferForPublisherFarEnd> m_PublisherBufferForMuxing;
		SharedPointer<CAudioByteBuffer> m_FarEndBufferOpus;

		CAudioDumper *m_pChunckedNE = nullptr, *m_pPlayedFE = nullptr, *m_pPlayerSidePreGain = nullptr, *m_pPlayedPublisherFE = nullptr,
			*m_pPlayedCalleeFE = nullptr;
		int m_iSpeakerType;
		CAudioCallInfo* m_pAudioCallInfo = nullptr;
		
		const bool& m_bIsVideoCallRunning;

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

		bool m_bUsingLoudSpeaker;
		bool m_bLiveAudioStreamRunning;
		

		int m_iRole;
		int m_iVolume;
		int m_nServiceType;
		int m_nEntityType;
		
		int m_iAudioVersionFriend;
		int m_iAudioVersionSelf;
		int m_nCallInLiveType;

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
