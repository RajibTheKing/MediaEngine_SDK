#ifndef AUDIO_FAREND_DATA_PROCESSOR_H
#define AUDIO_FAREND_DATA_PROCESSOR_H

#include "AudioCallSession.h"
#include "AudioDecoderBuffer.h"
#include "AudioMacros.h"
#include "AudioPacketHeader.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioDePacketizer.h"


namespace MediaSDK
{
	class AudioGainInterface;
	class CAudioCallSession;
	class AudioPacketizer;
	class CCommonElementsBucket;
	class AudioPacketHeader;
	class CAudioShortBuffer;
	class Tools;
	class CAudioByteBuffer;
	class GomGomGain;
	class ILiveAudioParser;
	class AudioMixer;
	class AudioDecoderInterface;

	class AudioFarEndDataProcessor
	{
	public:
		AudioFarEndDataProcessor(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning);
		~AudioFarEndDataProcessor();

		virtual	void ProcessFarEndData() = 0;
		void ProcessPlayingData();

		bool m_b1stPlaying;
		long long m_llNextPlayingTime;

		int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > &vMissingFrames);
		void StartCallInLive(int nEntityType);
		void StopCallInLive(int nEntityType);
		void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
		void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llLastTime, int iCurrentPacketNumber);

		void SetEventCallback(OnFireDataEventCB dataCB, OnFireNetworkChangeCB networkCB, OnFireAudioAlarmCB alarmEvent)
		{
			m_cbOnDataEvent = dataCB;
			m_cbOnNetworkChange = networkCB;
			m_cbOnAudioAlarm = alarmEvent;
		}


		//LiveReceiver *m_pLiveReceiverAudio = nullptr;
		ILiveAudioParser* m_pLiveAudioParser;
		long long m_llDecodingTimeStampOffset = -1;
		AudioDePacketizer* m_pAudioDePacketizer = nullptr;
		CAudioByteBuffer m_AudioReceivedBuffer;
		CAudioShortBuffer m_AudioPlayingBuffer;
		short m_saPlayingData[MAX_AUDIO_FRAME_Length];
		int m_iPlayedSinceRecordingStarted;

		short tmpBuffer[2048];


	private:

		int m_nServiceType;
		long long m_llFriendID = -1;
		bool m_bIsLiveStreamingRunning = false;
		int m_iLastDecodedPacketNumber = -1;
		int m_iAudioVersionFriend = -1;
		//int m_iPrevRecvdSlotID;
		int m_iCurrentRecvdSlotID = -1;
		int m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;

		OnFireDataEventCB m_cbOnDataEvent;
		OnFireNetworkChangeCB m_cbOnNetworkChange;
		OnFireAudioAlarmCB m_cbOnAudioAlarm;

		CCommonElementsBucket *m_pCommonElementsBucket = nullptr;
		//AudioPacketHeader *m_pAudioPacketHeader = nullptr;
		SmartPointer<AudioPacketHeader> m_pAudioFarEndPacketHeader = nullptr;
		SmartPointer<GomGomGain> m_pGomGomGain;


		SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
		SmartPointer<AudioDecoderInterface> m_cAacDecoder;

		int m_inoLossSlot;
		int m_ihugeLossSlot;

		bool m_bAudioQualityLowNotified;
		bool m_bAudioQualityHighNotified;
		bool m_bAudioShouldStopNotified;


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

		void DecideToChangeBitrate(int iNumPacketRecvd);


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

		void PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
			long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime);

	};

} //namespace MediaSDK

#endif //AUDIO_FAREND_DATA_PROCESSOR_H