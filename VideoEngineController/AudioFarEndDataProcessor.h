#ifndef AUDIO_FAREND_DATA_PROCESSOR_H
#define AUDIO_FAREND_DATA_PROCESSOR_H

#include "AudioCallSession.h"
#include "AudioDecoderBuffer.h"
#include "AudioMacros.h"
#include "AudioPacketHeader.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioDePacketizer.h"
#include "AudioCodec.h"
#include "Aac.h"
#include "GomGomGain.h"

#ifdef USE_AGC
#include "Gain.h"
#endif

class CAudioCallSession;
class AudioPacketizer;
class CCommonElementsBucket;
class CAudioPacketHeader;
class CAudioShortBuffer;
//class CAudioCodec;
class Tools;
class CAudioByteBuffer;
class CGomGomGain;
class ILiveAudioParser;
class AudioMixer;

class CAudioFarEndDataProcessor
{
public:
	CAudioFarEndDataProcessor(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning);
	~CAudioFarEndDataProcessor();
	int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > &vMissingFrames);
	void StartCallInLive();
	void StopCallInLive();
	void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
	void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llLastTime, int iCurrentPacketNumber);


	//LiveReceiver *m_pLiveReceiverAudio = nullptr;
	ILiveAudioParser* m_pLiveAudioParser;
	long long m_llDecodingTimeStampOffset = -1;
	AudioDePacketizer* m_pAudioDePacketizer = nullptr;
	CAudioByteBuffer m_AudioReceivedBuffer;


private:
	int m_nServiceType;
	int m_nEntityType;
	long long m_llLastTime;
	long long m_llFriendID = -1;
	bool m_bIsLiveStreamingRunning = false;
	int m_iLastDecodedPacketNumber = -1;
	int m_nDecodedFrameSize = 0;
	int m_nDecodingFrameSize = 0;
	int m_iAudioVersionFriend = -1;
	//int m_iPrevRecvdSlotID;
	int m_iCurrentRecvdSlotID = -1;
	int m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;
	int m_iOpponentReceivedPackets = AUDIO_SLOT_SIZE;

	std::vector<std::pair<int, int>> m_vFrameMissingBlocks;

	AudioMixer* m_pAudioMixer;
	CAudioCallSession *m_pAudioCallSession = nullptr;
	CCommonElementsBucket *m_pCommonElementsBucket = nullptr;
	CAudioPacketHeader *m_pAudioPacketHeader = nullptr;
	CAudioPacketHeader *m_ReceivingHeader = nullptr;
	CGomGomGain *m_pGomGomGain = nullptr;

	std::vector<LiveAudioDecodingQueue*> m_vAudioFarEndBufferVector;


	bool m_bAudioDecodingThreadRunning;
	bool m_bAudioDecodingThreadClosed;

	LiveAudioDecodingQueue *m_pLiveAudioReceivedQueue = nullptr;
	unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_Length];
	short m_saDecodedFrame[MAX_AUDIO_FRAME_Length];
	short m_saCalleeSentData[MAX_AUDIO_FRAME_Length];

	CAac *m_cAac = nullptr;

	void StartDecodingThread();
	static void* CreateAudioDecodingThread(void* param);
	void StopDecodingThread();
	void DecodingThreadProcedure();

	void AudioCallFarEndProcedure();
	void LiveStreamFarEndProcedureViewer();
	void LiveStreamFarEndProcedure();
	bool IsQueueEmpty();
	void DequeueData(int &m_nDecodingFrameSize);
	void DecodeAndPostProcessIfNeeded(int &iPacketNumber, int &nCurrentPacketHeaderLength, int &nCurrentAudioPacketType);
	bool IsPacketNumberProcessable(int &iPacketNumber);
	void ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
		int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength);
	bool IsPacketTypeSupported(int &nCurrentAudioPacketType);
	bool IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType);
	bool IsPacketProcessableInNormalCall(int &nCurrentAudioPacketType, int &nVersion);
	bool IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType);
	void SetSlotStatesAndDecideToChangeBitRate(int &nSlotNumber);
	void PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
		 long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime);
};

#endif //AUDIO_FAREND_DATA_PROCESSOR_H