
#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#define OPUS_ENABLE

#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioLiveHeader.h"
#include "LogPrinter.h"
#include "LiveAudioDecodingQueue.h"
#include "LiveReceiver.h"
#include "AudioCallHeader.h"

#include <stdio.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <vector>
#ifdef OPUS_ENABLE
#include "AudioCodec.h"
#else
#include "G729CodecNative.h"
#endif


#define __AUDIO_CALL_VERSION__  1
#define  __DUPLICATE_AUDIO__
#define MULTIPLE_HEADER


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
class CAudioLiveHeader;
class CAudioCodec;

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
public:

	int m_iNextPacketType;

public:

    CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject,int nServiceType, bool bIsCheckCall=false);
    ~CAudioCallSession();

	void StartCallInLive(int iRole);
	void EndCallInLive();

    CAudioCodec* GetAudioCodec();

    void InitializeAudioCallSession(LongLong llFriendID);
    int EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength);
	int CancelAudioData(short *psaEncodingAudioData, unsigned int unLength);
    int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
    
    int DecodeAudioDataVector(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);

    void StopEncodingThread();
    void StartEncodingThread();

    void StopDecodingThread();
    void StartDecodingThread();

	void SetVolume(int iVolume, bool bRecorder);
	void SetLoudSpeaker(bool bOn);
	void SetEchoCanceller(bool bOn);

    static void *CreateAudioEncodingThread(void* param);
    static void *CreateAudioDecodingThread(void* param);
	
    void GetAudioSendToData(unsigned char * pAudioRawDataToSend, int &RawLength, std::vector<int> &vRawDataLengthVector,
		std::vector<int> &vCompressedDataLengthVector, int &CompressedLength, unsigned char * pAudioCompressedDataToSend);

	void GetAudioSendToData(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector);

    int GetServiceType();

private:

	int m_iRole;
    Tools m_Tools;
    LongLong m_FriendID;
	bool m_bEchoCancellerEnabled;
	long long m_llEncodingTimeStampOffset;
	long long m_llDecodingTimeStampOffset;

    CAudioLiveHeader *SendingHeader;
    CAudioLiveHeader *ReceivingHeader;

	CAudioCallHeader m_sendingHeaderOld;
	CAudioCallHeader m_receivingHeaderOld;

    int m_AudioHeadersize;

    CCommonElementsBucket* m_pCommonElementsBucket;
	CAudioShortBuffer m_AudioEncodingBuffer, m_AudioDecodedBuffer;
	CAudioByteBuffer m_AudioDecodingBuffer;

    std::vector<int> m_vRawFrameLength, m_vCompressedFrameLength;
	bool m_bUsingLoudSpeaker;

    int m_iLastDecodedPacketNumber;    
    int m_iPacketNumber;
	int m_iSlotID;
	int m_iPrevRecvdSlotID, m_iCurrentRecvdSlotID;
	int m_iReceivedPacketsInPrevSlot, m_iReceivedPacketsInCurrentSlot;
	int m_iOpponentReceivedPackets;
	

	bool m_bIsCheckCall, m_bLoudSpeakerEnabled;

	///////////Pre Encoding Data///////
    short m_saAudioEncodingFrame[MAX_AUDIO_FRAME_Length];
	short m_saAudioMUXEDFrame[MAX_AUDIO_FRAME_Length];
	short m_saAudioPrevDecodedFrame[MAX_AUDIO_FRAME_Length];


	///////////Post Encoding Data///////
	/*
	m_ucaCompressedFrame is an Encoded frame.
	It comes from m_saAudioEncodingFrame after encoding, during non-live-call or live-call.
	Must not be used during live-streaming.
	*/
    unsigned char m_ucaCompressedFrame[MAX_AUDIO_FRAME_Length];
	/*
	m_ucaRawFrame is a Raw frame.
	It comes from m_saAudioEncodingFrame without encoding during livestream.
	It comes from m_saAudioMUXEDFrame without encoding during live-call.
	Must not be used during non-live-call.
	*/
	unsigned char m_ucaRawFrame[MAX_AUDIO_FRAME_Length];

	int m_nCompressedFrameSize, m_nRawFrameSize;


    unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_Length];
    short m_saDecodedFrame[MAX_AUDIO_FRAME_Length];
	int nDecodingFrameSize, nDecodedFrameSize;

    unsigned char m_ucaRawDataToSend[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	unsigned char m_ucaCompressedDataToSend[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	int m_iRawDataSendIndex, m_iCompressedDataSendIndex;
	long long m_llMaxAudioPacketNumber;

    bool m_bAudioEncodingThreadRunning;
    bool m_bAudioEncodingThreadClosed;

    bool m_bAudioDecodingThreadRunning;
    bool m_bAudioDecodingThreadClosed;
    
    LiveAudioDecodingQueue *m_pLiveAudioDecodingQueue;
    LiveReceiver *m_pLiveReceiverAudio;
    
    bool m_bLiveAudioStreamRunning;
    int m_nServiceType;
    

	int m_iVolume;

    int m_iAudioVersionFriend;
    int m_iAudioVersionSelf;

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

#ifdef OPUS_ENABLE
	CAudioCodec *m_pAudioCodec;
#else
	G729CodecNative *m_pG729CodecNative;
#endif

private:

	void EncodingThreadProcedure();
	///////Methods Called From EncodingThreadProcedure/////
	void MuxIfNeeded();
	void DumpEncodingFrame();
	void PrintRelativeTime(int &cnt, long long &llLasstTime, int &countFrame, int &nCurrentTimeStamp, long long &timeStamp);
	bool PreProcessAudioBeforeEncoding();
	void EncodeIfNeeded(long long &timeStampm, int &encodingTime, double &avgCountTimeStamp);
	void AddHeader(int &version, int &nCurrentTimeStamp);
	void SetAudioIdentifierAndNextPacketType();
	void SendAudioData();
	void MuxAudioData(short * pData1, short * pData2, short * pMuxedData, int iDataLength);
	void BuildAndGetHeaderInArray(int packetType, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header);
	///////End of Methods Directly Called From EncodingThreadProcedure/////

	void DecodingThreadProcedure();
	///////Methods Called From DecodingThreadProcedure/////
	bool IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType);
	bool IsPacketNumberProcessable(int &iPacketNumber);
	bool IsPacketTypeProcessable(int &nCurrentAudioPacketType, int &nVersion, Tools &toolsObject);
	bool IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime);
	void SetSlotStatesAndDecideToChangeBitRate(int &iPacketNumber, int &nSlotNumber);
	void DecodeAndPostProcessIfNeeded();
	void DumpDecodedFrame();
	void PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
		int &iFrameCounter, long long &nDecodingTime, double &dbTotalTime, long long &timeStamp);
	void SendToPlayer(long long &llNow, long long &llLastTime);
	void ParseHeaderAndGetValues(int &packetType, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
		int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header);
	///////End Of Methods Called From DecodingThreadProcedure/////

protected:

    SmartPointer<CLockHandler> m_pAudioCallSessionMutex;
    SmartPointer<std::thread> m_pAudioEncodingThread;
    SmartPointer<std::thread> m_pAudioDecodingThread;
};


#endif
