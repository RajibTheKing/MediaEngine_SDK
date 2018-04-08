#ifndef AUDIO_NEAREND_DATA_PROCESSOR_H
#define AUDIO_NEAREND_DATA_PROCESSOR_H

#include "Size.h"
#include "CommonTypes.h"
#include "AudioTypes.h"
#include "SmartPointer.h"
#include "AudioMacros.h"
#include "AudioDumper.h"
#include <vector>
#include "AudioSessionStatistics.h"

namespace MediaSDK
{
	class AudioEncoderInterface;
	class NoiseReducerInterface;

	class AudioMixer;
	class CAudioCallSession;
	class CAudioShortBuffer;
	class AudioPacketHeader;
	class AudioLinearBuffer;
	class CTrace;
	class CKichCutter;
	class AudioSessionStatistics;
	class SessionStatisticsInterface;


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
		unsigned int GetNumberOfFrameForChunk();
		void ClearRecordBuffer();
		void PushDataInRecordBuffer(short *data, int dataLen);
		void SetNeedToResetAudioEffects(bool flag);
		void SetEnableRecorderTimeSyncDuringEchoCancellation(bool flag);
		void ResetTrace();
		void DeleteDataB4TraceIsReceived(short *psaEncodingAudioData, unsigned int unLength);
		void DeleteDataAfterTraceIsReceived(short *psaEncodingAudioData, unsigned int unLength);
		void HandleTrace(short *psaEncodingAudioData, unsigned int unLength);
		bool IsKichCutterEnabled();
		bool IsEchoCancellerEnabled();
		bool IsTraceSendingEnabled();
		void ResetKichCutter(); 
		void ResetAEC();
		void ResetNS();
		void ResetRecorderGain();
		void ResetAudioEffects();
		void SyncRecordingTime();

		int GetTraceReceivedCount();
		void SetTraceReceived();

	protected:

		int PreprocessAudioData(short *psaEncodingAudioData, unsigned int unLength);

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
		short m_saNoisyData[MAX_AUDIO_FRAME_SAMPLE_SIZE];
		bool m_bTraceSent, m_bTraceRecieved, m_bTraceWillNotBeReceived, m_b30VerifiedTrace, m_bJustWrittenTraceDump;
		int  m_iDeleteCount;
		CTrace *m_pTrace = nullptr;
		long long m_llTraceSendingTime;
		long long m_llTraceReceivingTime;
		int m_nFramesRecvdSinceTraceSent;
		bool m_bTraceTailRemains;
		short m_saFarendData[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		CKichCutter *m_pKichCutter;
		bool m_b1stRecordedDataSinceCallStarted;
		long long m_ll1stRecordedDataTime;
		long long m_llnextRecordedDataTime;

		SessionStatisticsInterface *GetSessionStatListener() { return m_pAudioSessionStatistics; }

	protected:

		AudioLinearBuffer* m_recordBuffer = nullptr;

		DataReadyListenerInterface* m_pDataReadyListener;

		bool m_bIsLiveStreamingRunning;

		int m_MyAudioHeadersize;
		int m_iPacketNumber;
		long long m_llMaxAudioPacketNumber;

		short m_saAudioRecorderFrame[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];    //Always contains UnMuxed Data
		unsigned char m_ucaEncodedFrame[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		unsigned char m_ucaRawFrameNonMuxed[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		unsigned char m_ucaRawFrameForInformation[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];

		SharedPointer<CAudioShortBuffer> m_pAudioNearEndBuffer;
		SharedPointer<AudioEncoderInterface> m_pAudioEncoder;

		AudioSessionStatistics *m_pAudioSessionStatistics = nullptr;

	private:

		PacketEventListener* m_pPacketEventListener;

		bool m_bIsReady;
		bool m_bAudioEncodingThreadClosed;
		bool m_bAudioEncodingThreadRunning;

		int m_nServiceType;
		int m_nEntityType;
		int m_nStoredDataLengthNear;		
		int m_nStoredDataLengthFar;

		int m_iTraceReceivedCount = 0;

		long long m_llFriendID;
		long long m_llLastChunkLastFrameRT;
		long long m_llLastFrameRT;
		long long m_llEncodingTimeStampOffset;

		unsigned char m_ucaRawDataToSendNear[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
		unsigned char m_ucaRawDataToSendFar[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];

		short m_saAudioTraceRemovalBuffer[MAX_AUDIO_FRAME_SAMPLE_SIZE];

		std::vector<int> m_vRawFrameLengthNear;
		std::vector<int> m_vRawFrameLengthFar;		

		//SharedPointer<NoiseReducerInterface> m_pNoise;
		//SharedPointer<std::thread> m_pAudioEncodingThread;
		SharedPointer<AudioPacketHeader> m_pAudioNearEndPacketHeader = nullptr;
		SharedPointer<CLockHandler> m_pAudioEncodingMutex;


		bool m_bNeedToResetAudioEffects;
		bool m_bEnableRecorderTimeSyncDuringEchoCancellation;
		int m_iStartingBufferSize;
		int m_iDelayFractionOrig;

		long long m_llDelay, m_llDelayFraction, m_llLastEchoLogTime = 0;
		CAudioDumper *m_pRecordedNE = nullptr, *m_pGainedNE = nullptr, *m_pProcessed2NE = nullptr, *m_pNoiseReducedNE = nullptr, 
			*m_pCancelledNE = nullptr, *m_pKichCutNE = nullptr, *m_pProcessedNE = nullptr, *m_pTraceRemoved = nullptr,  *m_pTraceDetectionDump = nullptr;			
;
	};

} //namespace MediaSDK

#endif //AUDIO_NEAREND_DATA_PROCESSOR_H