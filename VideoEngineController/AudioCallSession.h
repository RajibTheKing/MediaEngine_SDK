
#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#define OPUS_ENABLED
#define AAC_ENABLED

#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioPacketHeader.h"
#include "LogPrinter.h"
#include "LiveAudioDecodingQueue.h"
#include "LiveReceiver.h"

#include <stdio.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <vector>


//#define LOCAL_SERVER_LIVE_CALL
#ifdef LOCAL_SERVER_LIVE_CALL
#include "VideoSockets.h"
#endif


#ifdef AAC_ENABLED
#include "Aac.h"
#endif

#ifdef OPUS_ENABLED
#include "AudioCodec.h"
#else
#include "G729CodecNative.h"
#endif


#define __AUDIO_CALL_VERSION__  0
#define __AUDIO_LIVE_VERSION__  0

#ifdef __ANDROID__
#define USE_AECM
// #define USE_ANS
#define USE_AGC
//#define USE_VAD
#endif

static string colon = "ALOG:";
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO,colon + a);




//#define __AUDIO_FIXED_COMPLEXITY__

class CCommonElementsBucket;
class CVideoEncoder;
class CAudioCodec;
class CAac;

#ifdef USE_AECM
class CEcho;
#endif
#ifdef USE_ANS
class CNoise;
#endif
#ifdef USE_AGC
class CGain;
#endif
#ifdef USE_VAD
class CVoice;
#endif


class CAudioCallSession
{
private:
	bool m_bIsAECMFarEndThreadBusy;
	bool m_bIsAECMNearEndThreadBusy;
	bool m_bIsCallInLiveRunning;
	long long m_llLastPlayTime;

public:
	int m_iNextPacketType;
	CAudioByteBuffer m_AudioReceivedBuffer;
public:
    CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject,int nServiceType, bool bIsCheckCall=false);
    ~CAudioCallSession();

	void StartCallInLive(int iRole);
	void EndCallInLive();

    CAudioCodec* GetAudioCodec();

    void InitializeAudioCallSession(LongLong llFriendID);
    int EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength);
	int CancelAudioData(short *psaEncodingAudioData, unsigned int unLength);
    
    int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);

    void StopEncodingThread();
    void StartEncodingThread();

    void StopDecodingThread();
    void StartDecodingThread();

	void SetVolume(int iVolume, bool bRecorder);
	void SetLoudSpeaker(bool bOn);
	void SetEchoCanceller(bool bOn);
	bool getIsAudioLiveStreamRunning();

    static void *CreateAudioEncodingThread(void* param);
    static void *CreateAudioDecodingThread(void* param);
#if 0	
	void GetAudioSendToData(unsigned char * pAudioRawDataToSendMuxed, int &RawLengthMuxed, std::vector<int> &vRawDataLengthVectorMuxed,
		std::vector<int> &vRawDataLengthVectorNonMuxed, int &RawLengthNonMuxed, unsigned char * pAudioNonMuxedDataToSend);
#endif

	void GetAudioSendToData(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector);

    int GetServiceType();

private:

#ifdef LOCAL_SERVER_LIVE_CALL
	VideoSockets *m_clientSocket;
#endif

	int m_iRole;
    Tools m_Tools;
    LongLong m_FriendID;
	bool m_bEchoCancellerEnabled;
	long long m_llEncodingTimeStampOffset;
	long long m_llDecodingTimeStampOffset;

    CAudioPacketHeader *m_SendingHeader;
    CAudioPacketHeader *m_ReceivingHeader;

    int m_MyAudioHeadersize;

    CCommonElementsBucket* m_pCommonElementsBucket;
	CAudioShortBuffer m_AudioEncodingBuffer, m_AudioDecodedBuffer;
	

	std::vector<int> m_vRawFrameLengthViewer, m_vRawFrameLengthCallee;
	bool m_bUsingLoudSpeaker;

    int m_iLastDecodedPacketNumber;    
    int m_iPacketNumber;
	int m_iSlotID;
	int m_iPrevRecvdSlotID, m_iCurrentRecvdSlotID;
	int m_iReceivedPacketsInPrevSlot, m_iReceivedPacketsInCurrentSlot;
	int m_iOpponentReceivedPackets;
	

	bool m_bIsCheckCall;

	///////////Pre Encoding Data///////
    short m_saAudioRecorderFrame[MAX_AUDIO_FRAME_Length];//Always contains UnMuxed Data
	short m_saAudioMUXEDFrame[MAX_AUDIO_FRAME_Length];//Always contains data for VIEWER_NOT_IN_CALL, MUXED data if m_saAudioPrevDecodedFrame is available
	short m_saAudioPrevDecodedFrame[MAX_AUDIO_FRAME_Length];
	short m_saEvenPacketStorage[MAX_AUDIO_FRAME_Length];
	int m_iLastEvenStoredPacket;

	///////////Post Encoding Data///////
	/*
	m_ucaEncodedFrame is an Encoded frame.
	It comes from m_saAudioRecorderFrame after encoding, during non-live-call.
	Must not be used during live-streaming.
	*/
    unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_Length];
	/*
	m_ucaRawFrame is a Raw frame.
	It comes from m_saAudioRecorderFrame without encoding during livestream.
	Must not be used during non-live-call.
	*/
	unsigned char m_ucaRawFrameMuxed[MAX_AUDIO_FRAME_Length], m_ucaRawFrameNonMuxed[MAX_AUDIO_FRAME_Length];


	int m_nEncodedFrameSize, m_nRawFrameSize;


    unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_Length];
    short m_saDecodedFrame[MAX_AUDIO_FRAME_Length];
	int m_nDecodingFrameSize, m_nDecodedFrameSize;

    unsigned char m_ucaRawDataToSendCallee[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	unsigned char m_ucaRawDataToSendViewer[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	int m_iRawDataSendIndexViewer, m_iRawDataSendIndexCallee;
	long long m_llMaxAudioPacketNumber;

    bool m_bAudioEncodingThreadRunning;
    bool m_bAudioEncodingThreadClosed;

    bool m_bAudioDecodingThreadRunning;
    bool m_bAudioDecodingThreadClosed;
    
    LiveAudioDecodingQueue *m_pLiveAudioReceivedQueue;
    LiveReceiver *m_pLiveReceiverAudio;
    
    bool m_bLiveAudioStreamRunning;
    int m_nServiceType;
    
	int m_iVolume;

    int m_iAudioVersionFriend;
    int m_iAudioVersionSelf;

#ifdef AAC_ENABLED
	CAac *m_cAac;
#endif

#ifdef USE_ANS
	short m_saAudioEncodingDenoisedFrame[MAX_AUDIO_FRAME_Length];
#endif

#if defined(USE_AECM) || defined(USE_ANS) || defined(USE_AGC)
	short m_saAudioEncodingTempFrame[MAX_AUDIO_FRAME_Length];
#endif

#ifdef USE_AECM
	CEcho *m_pEcho, *m_pEcho2;
#endif

#ifdef USE_ANS
	CNoise *m_pNoise;
#endif

#ifdef USE_AGC
	CGain * m_pRecorderGain;
	CGain * m_pPlayerGain;
#endif

#ifdef USE_VAD
	CVoice *m_pVoice;
#endif

#ifdef OPUS_ENABLED
	CAudioCodec *m_pAudioCodec;
#else
	G729CodecNative *m_pG729CodecNative;
#endif

private:

	void EncodingThreadProcedure();
	///////Methods Called From EncodingThreadProcedure/////
	void MuxIfNeeded();
	void DumpEncodingFrame();
	void PrintRelativeTime(int &cnt, long long &llLasstTime, int &countFrame, long long &nCurrentTimeStamp, long long &timeStamp);
	bool PreProcessAudioBeforeEncoding();
	void EncodeIfNeeded(long long &timeStampm, long long &encodingTime, double &avgCountTimeStamp);
	void AddHeader(int &version, long long &nCurrentTimeStamp);
	void SetAudioIdentifierAndNextPacketType();
	void SendAudioData(Tools toolsObject);
	void MuxAudioData(short * pData1, short * pData2, short * pMuxedData, int iDataLength);
	void BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header);
	///////End of Methods Called From EncodingThreadProcedure/////

	void DecodingThreadProcedure();
	///////Methods Called From DecodingThreadProcedure/////
	bool IsQueueEmpty(Tools &toolsObject);
	void DequeueData(int &nDecodingFrameSize);
	bool IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType);
	bool IsPacketNumberProcessable(int &iPacketNumber);
	bool IsPacketTypeSupported(int &nCurrentAudioPacketType);
	bool IsPacketProcessableInNormalCall(int &nCurrentAudioPacketType, int &nVersion, Tools &toolsObject);
	bool IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType);
	void SetSlotStatesAndDecideToChangeBitRate(int &nSlotNumber);
	void DecodeAndPostProcessIfNeeded(int &iPacketNumber, int &nCurrentPacketHeaderLength, int &nCurrentAudioPacketType);
	void DumpDecodedFrame();
	void PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
		int &iFrameCounter, long long &nDecodingTime, double &dbTotalTime, long long &timeStamp);
	void SendToPlayer(long long &llNow, long long &llLastTime, int iCurrentPacketNumber);
	void ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
		int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header);
	///////End Of Methods Called From DecodingThreadProcedure/////

protected:

    SmartPointer<CLockHandler> m_pAudioCallSessionMutex;
    SmartPointer<std::thread> m_pAudioEncodingThread;
    SmartPointer<std::thread> m_pAudioDecodingThread;
};


#endif
