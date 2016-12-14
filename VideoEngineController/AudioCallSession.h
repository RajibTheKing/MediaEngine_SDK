
#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#define OPUS_ENABLE

#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioPacketHeader.h"
#include "LogPrinter.h"
#include "LiveAudioDecodingQueue.h"
#include "LiveReceiver.h"
#include "AudioHeader.h"

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


//#ifdef __ANDROID__
#define USE_AECM
// #define USE_ANS
#define USE_AGC
//#define USE_VAD
//#endif

static string colon = "ALOG:";
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO,colon + a);




//#define __AUDIO_FIXED_COMPLEXITY__

class CCommonElementsBucket;
class CVideoEncoder;
class CAudioPacketHeader;
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

    CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject,int nServiceType, bool bIsCheckCall=false);
    ~CAudioCallSession();

    CAudioCodec* GetAudioCodec();

    void InitializeAudioCallSession(LongLong llFriendID);
    int EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength);
	int CancelAudioData(short *psaEncodingAudioData, unsigned int unLength);
    int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
    
    int DecodeAudioDataVector(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);

    void EncodingThreadProcedure();
    void StopEncodingThread();
    void StartEncodingThread();

    void DecodingThreadProcedure();
    void StopDecodingThread();
    void StartDecodingThread();

	void SetVolume(int iVolume, bool bRecorder);
	void SetLoudSpeaker(bool bOn);
	void SetEchoCanceller(bool bOn);

    static void *CreateAudioEncodingThread(void* param);
    static void *CreateAudioDecodingThread(void* param);
	int m_iNextPacketType;
	long long m_llMaxAudioPacketNumber;
    void getAudioSendToData(unsigned char * pAudioDataToSend, int &length, std::vector<int> &vDataLengthVector);
    int GetServiceType();

private:

    Tools m_Tools;
    LongLong m_FriendID;
	bool m_bEchoCancellerEnabled;
	long long m_llEncodingTimeStampOffset;
	long long m_llDecodingTimeStampOffset;

    CAudioPacketHeader *SendingHeader;
    CAudioPacketHeader *ReceivingHeader;

	AudioHeader m_sendingHeaderOld;
	AudioHeader m_receivingHeaderOld;

	void BuildAndGetHeaderInArray(int packetType, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header);

	void ParseHeaderAndGetValues(int &packetType, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
		int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header);

    int m_AudioHeadersize;

    CCommonElementsBucket* m_pCommonElementsBucket;
    CAudioCodecBuffer m_AudioEncodingBuffer;
    CAudioDecoderBuffer m_AudioDecodingBuffer;

    std::vector<int> m_vEncodedFrameLenght;
	bool m_bUsingLoudSpeaker;
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
//    int m_iNextPacketType;
    int m_iLastDecodedPacketNumber;    
    int m_iPacketNumber;
	int m_iSlotID;
	int m_iPrevRecvdSlotID, m_iCurrentRecvdSlotID;
	int m_iReceivedPacketsInPrevSlot, m_iReceivedPacketsInCurrentSlot;
	int m_iOpponentReceivedPackets;
	

	bool m_bIsCheckCall, m_bLoudSpeakerEnabled;

    short m_saAudioEncodingFrame[MAX_AUDIO_FRAME_LENGHT];
    unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_LENGHT];
    unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_LENGHT];
    short m_saDecodedFrame[MAX_AUDIO_FRAME_LENGHT];

    unsigned char m_ucaAudioDataToSend[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	int m_iAudioDataSendIndex;
#ifdef USE_ANS
	short m_saAudioEncodingDenoisedFrame[MAX_AUDIO_FRAME_LENGHT];
#endif
#if defined(USE_AECM) || defined(USE_ANS) || defined(USE_AGC)
	short m_saAudioEncodingTempFrame[MAX_AUDIO_FRAME_LENGHT];
#endif

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

protected:

    SmartPointer<CLockHandler> m_pAudioCallSessionMutex;
    SmartPointer<std::thread> m_pAudioEncodingThread;
    SmartPointer<std::thread> m_pAudioDecodingThread;
};


#endif
