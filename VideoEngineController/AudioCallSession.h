
#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#define OPUS_ENABLE

#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioPacketHeader.h"
#include "LogPrinter.h"
#include <stdio.h>
#include <string>
#include <map>

#ifdef OPUS_ENABLE
#include "AudioCodec.h"
#else
#include "G729CodecNative.h"
#endif

#define USE_AECM
#define USE_ANS
#define USE_AGC

#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO,a);
#ifdef USE_AECM
#include "echo_control_mobile.h"
#endif
#ifdef USE_ANS
#include "noise_suppression.h"
#endif
#ifdef USE_AGC
#include "gain_control.h"
#endif

#ifdef USE_ANS
#define ANS_SAMPLE_SIZE 80
#define Mild 0
#define Medium 1
#define Aggressive 2
#endif

#ifdef USE_AECM
#define AECM_SAMPLE_SIZE 80
#endif


class CCommonElementsBucket;
class CVideoEncoder;
class CAudioPacketHeader;
class CAudioCodec;

class CAudioCallSession
{

public:

    CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, bool bIsCheckCall=false);
    ~CAudioCallSession();

    CAudioCodec* GetAudioCodec();

    void InitializeAudioCallSession(LongLong llFriendID);
    int EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength);
    int DecodeAudioData(unsigned char *pucaDecodingAudioData, unsigned int unLength);

    void EncodingThreadProcedure();
    void StopEncodingThread();
    void StartEncodingThread();

    void DecodingThreadProcedure();
    void StopDecodingThread();
    void StartDecodingThread();

    static void *CreateAudioEncodingThread(void* param);
    static void *CreateAudioDecodingThread(void* param);
	int m_iNextPacketType;

private:

    Tools m_Tools;
    LongLong m_FriendID;

    CAudioPacketHeader *SendingHeader;
    CAudioPacketHeader *ReceivingHeader;
    int m_AudioHeadersize;

    CCommonElementsBucket* m_pCommonElementsBucket;
    CAudioCodecBuffer m_AudioEncodingBuffer;
    CAudioDecoderBuffer m_AudioDecodingBuffer;

#ifdef USE_AECM
	void* AECM_instance;
	bool bAecmCreated;
	bool bAecmInited;
	bool bNoDataFromFarendYet;
#endif

#ifdef USE_ANS
	NsHandle* NS_instance;
#endif

#ifdef OPUS_ENABLE
    CAudioCodec *m_pAudioCodec;
#else
    G729CodecNative *m_pG729CodecNative;
#endif

    int m_nMaxAudioPacketNumber;
    int m_iPacketNumber;
	int m_iSlotID;
	int m_iPrevRecvdSlotID, m_iCurrentRecvdSlotID;
	int m_iReceivedPacketsInPrevSlot, m_iReceivedPacketsInCurrentSlot;
	int m_iOpponentReceivedPackets;
	

    bool m_bIsCheckCall;

    short m_saAudioEncodingFrame[MAX_AUDIO_FRAME_LENGHT];
    unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_LENGHT];
    unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_LENGHT];
    short m_saDecodedFrame[MAX_AUDIO_FRAME_LENGHT];
#ifdef USE_ANS
	short m_saAudioEncodingDenoisedFrame[MAX_AUDIO_FRAME_LENGHT];
#endif
#if defined(USE_AECM) || defined(USE_ANS)
	short m_saAudioEncodingTempFrame[MAX_AUDIO_FRAME_LENGHT];
#endif


    bool m_bAudioEncodingThreadRunning;
    bool m_bAudioEncodingThreadClosed;

    bool m_bAudioDecodingThreadRunning;
    bool m_bAudioDecodingThreadClosed;



protected:

    SmartPointer<CLockHandler> m_pAudioCallSessionMutex;
    SmartPointer<std::thread> m_pAudioEncodingThread;
    SmartPointer<std::thread> m_pAudioDecodingThread;
};


#endif
