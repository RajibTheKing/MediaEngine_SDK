#ifndef AUDIO_FAREND_DATA_PROCESSOR_H
#define AUDIO_FAREND_DATA_PROCESSOR_H

#include "AudioCallSession.h"
#include "AudioDecoderBuffer.h"
#include "AudioMacros.h"
#include "AudioPacketHeader.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioDePacketizer.h"
#include "AudioCodec.h"
#include "Gain.h"
#include "Aac.h"
#include "GomGomGain.h"

class CAudioCallSession;
class AudioPacketizer;
class CCommonElementsBucket;
class CAudioPacketHeader;
class CAudioShortBuffer;
//class CAudioCodec;
class Tools;
class CAudioByteBuffer;
class CGomGomGain;

class CAudioFarEndDataProcessor
{
public:
	CAudioFarEndDataProcessor(long long llFriendID, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning);
	~CAudioFarEndDataProcessor();
	int DecodeAudioData(unsigned char *pucaDecodingAudioData, unsigned int unLength);
	void StartCallInLive();
	void StopCallInLive();

private:

	long long m_llFriendID = -1;
	bool m_bIsLiveStreamingRunning = false;
	int m_iLastDecodedPacketNumber = -1;
	int m_nDecodedFrameSize = 0;
	int m_nDecodingFrameSize = 0;
	int m_iAudioVersionFriend = -1;
	long long m_llDecodingTimeStampOffset = -1;
	//int m_iPrevRecvdSlotID;
	int m_iCurrentRecvdSlotID = -1;
	int m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;
	int m_iOpponentReceivedPackets = AUDIO_SLOT_SIZE;

	CAudioCallSession *m_pAudioCallSession = nullptr;
	CCommonElementsBucket *m_pCommonElementsBucket = nullptr;
	CAudioPacketHeader *m_pAudioPacketHeader = nullptr;
	CAudioPacketHeader *m_ReceivingHeader = nullptr;
	CGomGomGain *m_pGomGomGain = nullptr;

	CAudioByteBuffer m_AudioReceivedBuffer;
	AudioDePacketizer* m_pAudioDePacketizer = nullptr;

	bool m_bAudioDecodingThreadRunning;
	bool m_bAudioDecodingThreadClosed;

	LiveAudioDecodingQueue *m_pLiveAudioReceivedQueue = nullptr;
	LiveReceiver *m_pLiveReceiverAudio = nullptr;
	unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_Length];
	short m_saDecodedFrame[MAX_AUDIO_FRAME_Length];
#ifdef AAC_ENABLED
	CAac *m_cAac = nullptr;
#endif

	void StartDecodingThread();
	static void* CreateAudioDecodingThread(void* param);
	void StopDecodingThread();
	void DecodingThreadProcedure();
	bool IsQueueEmpty(Tools &toolsObject);
	void DequeueData(int &m_nDecodingFrameSize);
	void DecodeAndPostProcessIfNeeded(int &iPacketNumber, int &nCurrentPacketHeaderLength, int &nCurrentAudioPacketType);
	bool IsPacketNumberProcessable(int &iPacketNumber);
	void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llNow, long long &llLastTime, int iCurrentPacketNumber);
	void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
	void ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
		int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength);
	bool IsPacketTypeSupported(int &nCurrentAudioPacketType);
	bool IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType);
	bool IsPacketProcessableInNormalCall(int &nCurrentAudioPacketType, int &nVersion, Tools &toolsObject);
	bool IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType);
	void SetSlotStatesAndDecideToChangeBitRate(int &nSlotNumber);
	void PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
		int &iFrameCounter, long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime);
};

#endif //AUDIO_FAREND_DATA_PROCESSOR_H