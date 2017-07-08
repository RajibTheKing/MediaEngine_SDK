#ifndef AUDIO_FAREND_DATA_PROCESSOR_H
#define AUDIO_FAREND_DATA_PROCESSOR_H


#include "Size.h"
#include "AudioMacros.h"
#include "AudioTypes.h"
#include "SmartPointer.h"
#include <vector>


namespace MediaSDK
{
	class AudioEncoderInterface;
	class AudioDecoderInterface;

	class LiveAudioDecodingQueue;
	class CAudioCallSession;
	class CCommonElementsBucket;
	class AudioPacketHeader;
	class CAudioByteBuffer;
	class GomGomGain;
	class ILiveAudioParser;
	class AudioMixer;
	class AudioDePacketizer;


	class AudioFarEndDataProcessor
	{

	public:

		AudioFarEndDataProcessor(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning);
		virtual	~AudioFarEndDataProcessor();

		virtual	void ProcessFarEndData() = 0;
		void ProcessPlayingData();

		void StartCallInLive(int nEntityType);
		void StopCallInLive(int nEntityType);
		void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
		int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > &vMissingFrames);
		void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llLastTime, int iCurrentPacketNumber);
		void SetEventCallback(DataEventListener* pDataListener, NetworkChangeListener* networkListener, AudioAlarmListener* alarmListener);


	protected:

		bool IsQueueEmpty();
		void DequeueData(int &m_nDecodingFrameSize);
		void ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
			int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength);

		bool IsPacketTypeSupported(int &nCurrentAudioPacketType);
		bool IsPacketProcessableInNormalCall(int &nCurrentAudioPacketType, int &nVersion);
		bool IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType);
		bool IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType);

		void SetSlotStatesAndDecideToChangeBitRate(int &nSlotNumber);
		void DecodeAndPostProcessIfNeeded(int &iPacketNumber, int &nCurrentPacketHeaderLength, int &nCurrentAudioPacketType);
		void DecideToChangeBitrate(int iNumPacketRecvd);

		void PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec, long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime);

	
	public:

		bool m_b1stPlaying;
		long long m_llNextPlayingTime;
	
		long long m_llDecodingTimeStampOffset = -1;
		short tmpBuffer[2048];
		short m_saPlayingData[MAX_AUDIO_FRAME_Length];

		//LiveReceiver *m_pLiveReceiverAudio = nullptr;
		ILiveAudioParser* m_pLiveAudioParser;
		AudioDePacketizer* m_pAudioDePacketizer = nullptr;
		SmartPointer<CAudioByteBuffer> m_AudioReceivedBuffer;

		
	protected:

		int m_nEntityType;
		int m_nDecodingFrameSize = 0;
		int m_nDecodedFrameSize = 0;
		int m_iOpponentReceivedPackets = AUDIO_SLOT_SIZE;

		long long m_llLastTime;

		unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_Length];
		short m_saDecodedFrame[MAX_AUDIO_FRAME_Length];
		short m_saCalleeSentData[MAX_AUDIO_FRAME_Length];

		std::vector<std::pair<int, int>> m_vFrameMissingBlocks;
		std::vector<LiveAudioDecodingQueue*> m_vAudioFarEndBufferVector;

		AudioMixer* m_pAudioMixer;
		CAudioCallSession *m_pAudioCallSession = nullptr;
	
	
	private:
		
		DataEventListener* m_pDataEventListener;
		NetworkChangeListener* m_pNetworkChangeListener;
		AudioAlarmListener* m_pAudioAlarmListener;

		bool m_bAudioDecodingThreadRunning;
		bool m_bAudioDecodingThreadClosed;
		bool m_bAudioQualityLowNotified;
		bool m_bAudioQualityHighNotified;
		bool m_bAudioShouldStopNotified;
		bool m_bIsLiveStreamingRunning = false;

		int m_inoLossSlot;
		int m_ihugeLossSlot;
		int m_nServiceType;
		int m_iAudioVersionFriend = -1;
		int m_iCurrentRecvdSlotID = -1;
		int m_iLastDecodedPacketNumber = -1;
		int m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;

		SmartPointer<AudioPacketHeader> m_pAudioFarEndPacketHeader = nullptr;
		SmartPointer<GomGomGain> m_pGomGomGain;
		SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
		SmartPointer<AudioDecoderInterface> m_cAacDecoder;

	};

} //namespace MediaSDK

#endif //AUDIO_FAREND_DATA_PROCESSOR_H