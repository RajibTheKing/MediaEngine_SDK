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

		void ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &packetNumber, int &packetLength, 
			int &channel, int &version, long long &timestamp, unsigned char* header, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength, int &nEchoStateFlags);

		bool IsPacketTypeSupported(int &nCurrentAudioPacketType);
		bool IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType);
		bool IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType);

		//void SetSlotStatesAndDecideToChangeBitRate(int &nSlotNumber);
		void DecodeAndPostProcessIfNeeded(const int iPacketNumber, const int nCurrentPacketHeaderLength, const int nCurrentAudioPacketType);
		void DecideToChangeBitrate(int iNumPacketRecvd);

		void PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec, long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime);

	
	public:

		bool m_b1stPlaying;
		long long m_llNextPlayingTime;
	
		long long m_llDecodingTimeStampOffset = -1;
		short tmpBuffer[2048];
		short m_saPlayingData[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];

		//LiveReceiver *m_pLiveReceiverAudio = nullptr;
		ILiveAudioParser* m_pLiveAudioParser;
		AudioDePacketizer* m_pAudioDePacketizer = nullptr;
		SharedPointer<CAudioByteBuffer> m_AudioReceivedBuffer;		
	protected:

		AudioAlarmListener* m_pAudioAlarmListener;

		bool m_bIsLiveStreamingRunning = false;

		int m_nEntityType;
		int m_nDecodingFrameSize = 0;
		int m_nDecodedFrameSize = 0;

		long long m_llLastTime;

		unsigned char m_ucaDecodingFrame[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		short m_saDecodedFrame[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];

		std::vector<std::pair<int, int>> m_vFrameMissingBlocks;
		std::vector<LiveAudioDecodingQueue*> m_vAudioFarEndBufferVector;

		CAudioCallSession *m_pAudioCallSession = nullptr;
		

	private:
		
		DataEventListener* m_pDataEventListener;
		NetworkChangeListener* m_pNetworkChangeListener;

		bool m_bAudioDecodingThreadRunning;
		bool m_bAudioDecodingThreadClosed;
		bool m_bAudioQualityLowNotified;
		bool m_bAudioQualityHighNotified;
		bool m_bAudioShouldStopNotified;

		int m_inoLossSlot;
		int m_ihugeLossSlot;
		int m_nServiceType;
		int m_iAudioVersionFriend = -1;
		int m_iCurrentRecvdSlotID = -1;
		int m_iLastDecodedPacketNumber = -1;
		int m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;
	

		int m_nPacketsRecvdTimely = 0;
		int m_nPacketPlayed = 0;
		int m_nPacketsRecvdNotTimely = 0;
		int m_nPacketsLost = 0;
		int m_nExpectedNextPacketNumber = 0;

		SharedPointer<AudioPacketHeader> m_pAudioFarEndPacketHeader = nullptr;
		SharedPointer<GomGomGain> m_pGomGomGain;
		SharedPointer<AudioEncoderInterface> m_pAudioEncoder;
		SharedPointer<AudioDecoderInterface> m_cAacDecoder;

	private:

		void SyncPlayingTime();

	};

} //namespace MediaSDK

#endif //AUDIO_FAREND_DATA_PROCESSOR_H