#ifndef AUDIO_NEAREND_DATA_PROCESSOR_H
#define AUDIO_NEAREND_DATA_PROCESSOR_H

#include "Size.h"
#include "CommonTypes.h"
#include "AudioTypes.h"
#include "SmartPointer.h"
#include <vector>

namespace MediaSDK
{
	class AudioEncoderInterface;
	class NoiseReducerInterface;

	class AudioMixer;
	class CAudioCallSession;
	class CAudioShortBuffer;
	class AudioPacketHeader;


	class AudioNearEndDataProcessor
	{

	public:

		AudioNearEndDataProcessor(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		virtual ~AudioNearEndDataProcessor();
		virtual void ProcessNearEndData() = 0;

		void StartCallInLive(int nEntityType);
		void StopCallInLive(int nEntityType);

		void GetAudioDataToSend(unsigned char *pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector, int &nDataLengthNear, int &nDataLengthFar, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);
		long long GetBaseOfRelativeTime();

		void SetDataReadyCallback(DataReadyListenerInterface* pDataReady) { m_pDataReadyListener = pDataReady; }
		void SetEventCallback(PacketEventListener* pEventListener) { m_pPacketEventListener = pEventListener; }

		
	protected:

		void DumpEncodingFrame();
		void UpdateRelativeTimeAndFrame(long long &llLasstTime, long long & llRelativeTime, long long & llCapturedTime);
		bool PreProcessAudioBeforeEncoding();
		void BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int packetNumber, int packetLength, int channel, int version, long long timestamp, int echoStateFlags, unsigned char* header);
		void BuildHeaderForLive(int nPacketType, int nHeaderLength, int nVersion, int nPacketNumber, int nPacketLength,
			long long llRelativeTime, int nEchoStateFlags, unsigned char* ucpHeader);
		void StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nFrameLengthInByte);
		void StoreDataForChunk(unsigned char *uchNearData, int nNearFrameLengthInByte, unsigned char *uchFarData, int nFarFrameLengthInByte, long long llRelativeTime);


	public:
		
		CAudioCallSession *m_pAudioCallSession = nullptr;


	protected:

		DataReadyListenerInterface* m_pDataReadyListener;

		bool m_bIsLiveStreamingRunning;

		int m_MyAudioHeadersize;
		int m_iPacketNumber;
		long long m_llMaxAudioPacketNumber;

		short m_saAudioRecorderFrame[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];    //Always contains UnMuxed Data
		unsigned char m_ucaEncodedFrame[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		unsigned char m_ucaRawFrameNonMuxed[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];

		SharedPointer<CAudioShortBuffer> m_pAudioNearEndBuffer;
		SharedPointer<AudioEncoderInterface> m_pAudioEncoder;


	private:

		PacketEventListener* m_pPacketEventListener;

		bool m_bIsReady;
		bool m_bAudioEncodingThreadClosed;
		bool m_bAudioEncodingThreadRunning;

		int m_nServiceType;
		int m_nEntityType;
		int m_nStoredDataLengthNear;		
		int m_nStoredDataLengthFar;

		long long m_llFriendID;
		long long m_llLastChunkLastFrameRT;
		long long m_llLastFrameRT;
		long long m_llEncodingTimeStampOffset;

		unsigned char m_ucaRawDataToSendNear[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
		unsigned char m_ucaRawDataToSendFar[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];

		std::vector<int> m_vRawFrameLengthNear;
		std::vector<int> m_vRawFrameLengthFar;
		std::vector<std::pair<int, int>> m_vFrameMissingBlocks;

		//SharedPointer<NoiseReducerInterface> m_pNoise;
		//SharedPointer<std::thread> m_pAudioEncodingThread;
		SharedPointer<AudioPacketHeader> m_pAudioNearEndPacketHeader = nullptr;
		SharedPointer<CLockHandler> m_pAudioEncodingMutex;

	};

} //namespace MediaSDK

#endif //AUDIO_NEAREND_DATA_PROCESSOR_H