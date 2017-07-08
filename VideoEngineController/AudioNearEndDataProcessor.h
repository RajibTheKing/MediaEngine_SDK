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

	private:

		DataReadyListenerInterface* m_pDataReadyListener;
		PacketEventListener* m_pPacketEventListener;

		bool m_bIsReady;
		bool m_bAudioEncodingThreadClosed;
		bool m_bAudioEncodingThreadRunning;

		int m_nServiceType;
		int m_nEntityType;
		int m_iRawDataSendIndexViewer;

		long long m_llFriendID;
		long long m_llLastChunkLastFrameRT;
		long long m_llLastFrameRT;
		long long m_llEncodingTimeStampOffset;

		short m_saAudioPrevDecodedFrame[MAX_AUDIO_FRAME_Length];
		unsigned char m_ucaRawDataToSendViewer[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];

		std::vector<int> m_vRawFrameLengthViewer;
		std::vector<std::pair<int, int>> m_vFrameMissingBlocks;

		//SmartPointer<NoiseReducerInterface> m_pNoise;
		//SmartPointer<std::thread> m_pAudioEncodingThread;
		AudioMixer *m_pAudioMixer = nullptr;
		CAudioCallSession *m_pAudioCallSession = nullptr;
		SmartPointer<CAudioShortBuffer> m_pAudioNearEndBuffer = nullptr;
		SmartPointer<AudioPacketHeader> m_pAudioNearEndPacketHeader = nullptr;
		SmartPointer<CLockHandler> m_pAudioEncodingMutex;


	protected:

		bool m_bIsLiveStreamingRunning;

		int m_MyAudioHeadersize;
		int m_iPacketNumber;
		int m_nRawFrameSize;
		int m_nEncodedFrameSize;
		long long m_llMaxAudioPacketNumber;

		short m_saAudioRecorderFrame[MAX_AUDIO_FRAME_Length];    //Always contains UnMuxed Data
		short m_saSendingDataPublisher[MAX_AUDIO_FRAME_Length];  //Always contains data for VIEWER_NOT_IN_CALL, MUXED data if m_saAudioPrevDecodedFrame is available
		unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_Length];
		unsigned char m_ucaRawFrameNonMuxed[MAX_AUDIO_FRAME_Length];

		SmartPointer<AudioEncoderInterface> m_pAudioEncoder;


	protected:

		bool MuxIfNeeded(short* shPublisherData, short *shMuxedData, int &nDataSizeInByte, int nPacketNumber);
		void DumpEncodingFrame();
		void UpdateRelativeTimeAndFrame(long long &llLasstTime, long long & llRelativeTime, long long & llCapturedTime);
		bool PreProcessAudioBeforeEncoding();
		void BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber, int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header);
		void StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nDataLengthInByte);
		void SentToNetwork(long long llRelativeTime);

		void DecideToChangeComplexity(int iEncodingTime);


	public:

		AudioNearEndDataProcessor(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SmartPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		virtual ~AudioNearEndDataProcessor();

		void StartCallInLive(int nEntityType);
		void StopCallInLive(int nEntityType);

		void GetAudioDataToSend(unsigned char *pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector, int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);
		long long GetBaseOfRelativeTime();

		void SetDataReadyCallback(DataReadyListenerInterface* pDataReady) { m_pDataReadyListener = pDataReady; }
		void SetEventCallback(PacketEventListener* pEventListener) { m_pPacketEventListener = pEventListener; }

		virtual void ProcessNearEndData() = 0;
	};

} //namespace MediaSDK

#endif //AUDIO_NEAREND_DATA_PROCESSOR_H