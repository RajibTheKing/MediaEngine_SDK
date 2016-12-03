
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
#include <stdlib.h>

#ifdef OPUS_ENABLE
#include "AudioCodec.h"
#else
#include "G729CodecNative.h"
#endif


#define __AUDIO_CALL_VERSION__  1
#define  __DUPLICATE_AUDIO__


//#ifdef __ANDROID__
#define USE_AECM
// #define USE_ANS
#define USE_AGC
//#define USE_VAD
//#endif

static string colon = "ALOG:";
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO,colon + a);

#ifdef USE_AECM
#include "Echo.h"
#endif
#ifdef USE_ANS
#include "Noise.h"
#endif
#ifdef USE_AGC
#include "Gain.h"
#endif
#ifdef USE_VAD
#include "Voice.h"
#endif

#define __AUDIO_CALL_VERSION__  1


class CCommonElementsBucket;
class CVideoEncoder;
class CAudioPacketHeader;
class CAudioCodec;


class CAudioCallSession
{

public:

	CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, bool bIsCheckCall = false);
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

	void SetVolume(int iVolume, bool bRecorder);
	void SetLoudSpeaker(bool bOn);
	void SetEchoCanceller(bool bOn);

    static void *CreateAudioEncodingThread(void* param);
    static void *CreateAudioDecodingThread(void* param);
	int m_iNextPacketType;

private:
    Tools m_Tools;
    LongLong m_FriendID;
	bool m_bEchoCancellerEnabled;

    CAudioPacketHeader *SendingHeader;
    CAudioPacketHeader *ReceivingHeader;
    int m_AudioHeadersize;

    CCommonElementsBucket* m_pCommonElementsBucket;
    CAudioCodecBuffer m_AudioEncodingBuffer;
    CAudioDecoderBuffer m_AudioDecodingBuffer;

	bool m_bUsingLoudSpeaker;
	bool m_bNoDataFromFarendYet;
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
#if defined(USE_AECM) || defined(USE_ANS) || defined(USE_AGC)
	short m_saAudioEncodingTempFrame[MAX_AUDIO_FRAME_LENGHT];
#endif

    bool m_bAudioEncodingThreadRunning;
    bool m_bAudioEncodingThreadClosed;

    bool m_bAudioDecodingThreadRunning;
    bool m_bAudioDecodingThreadClosed;

	int m_iVolume;

    int m_iAudioVersionFriend;
    int m_iAudioVersionSelf;

protected:

    SmartPointer<CLockHandler> m_pAudioCallSessionMutex;
    SmartPointer<std::thread> m_pAudioEncodingThread;
    SmartPointer<std::thread> m_pAudioDecodingThread;
};


#endif
