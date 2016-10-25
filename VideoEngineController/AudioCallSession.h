
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

//#ifdef __ANDROID__
//#define USE_AECM
//#define USE_ANS
#define USE_AGC
//#define USE_VAD
//#endif

#ifdef USE_AGC
#define USE_WEBRTC_AGC
#ifndef USE_WEBRTC_AGC
#define USE_NAIVE_AGC
#endif
#endif

static string colon = "ALOG:";
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO,colon + a);
#ifdef USE_AECM
#include "echo_control_mobile.h"
#endif
#ifdef USE_ANS
#include "noise_suppression.h"
#endif
#ifdef USE_WEBRTC_AGC
#include "gain_control.h"
#include "signal_processing_library.h"
#endif
#ifdef USE_VAD
#include "webrtc_vad.h"
#endif


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

	void SetVolume(int iVolume);
	void SetLoudSpeaker(bool bOn);

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

	bool m_bUsingLoudSpeaker;
	bool m_bNoDataFromFarendYet;
#ifdef USE_AECM
	void* AECM_instance;
	bool bAecmCreated;
	bool bAecmInited;
#endif

#ifdef USE_ANS
	NsHandle* NS_instance;
#endif

#ifdef USE_WEBRTC_AGC
	void* AGC_instance;
#endif

#ifdef USE_VAD
	VadInst* VAD_instance;
	int nNextFrameMayHaveVoice;
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
#ifdef USE_VAD
	short m_saAudioBlankFrame[MAX_AUDIO_FRAME_LENGHT];
#endif
#ifdef USE_ANS
	short m_saAudioEncodingDenoisedFrame[MAX_AUDIO_FRAME_LENGHT];
#endif
#if defined(USE_AECM) || defined(USE_ANS)
	short m_saAudioEncodingTempFrame[MAX_AUDIO_FRAME_LENGHT];
#endif
#ifdef USE_WEBRTC_AGC
	short m_saAudioEncodingTempFrameLow[MAX_AUDIO_FRAME_LENGHT];
	short m_saAudioEncodingTempFrameHigh[MAX_AUDIO_FRAME_LENGHT];
#endif
#ifdef USE_WEBRTC_AGC
	short m_saAudioEncodingFrameLow[MAX_AUDIO_FRAME_LENGHT];
	short m_saAudioEncodingFrameHi[MAX_AUDIO_FRAME_LENGHT];
	int m_Filter_state1[MAX_AUDIO_FRAME_LENGHT];
	int m_Filter_state2[MAX_AUDIO_FRAME_LENGHT];
	int m_PostFilter_state1[MAX_AUDIO_FRAME_LENGHT];
	int m_PostFilter_state2[MAX_AUDIO_FRAME_LENGHT];
#endif


    bool m_bAudioEncodingThreadRunning;
    bool m_bAudioEncodingThreadClosed;

    bool m_bAudioDecodingThreadRunning;
    bool m_bAudioDecodingThreadClosed;

	float m_fVolume;

protected:

    SmartPointer<CLockHandler> m_pAudioCallSessionMutex;
    SmartPointer<std::thread> m_pAudioEncodingThread;
    SmartPointer<std::thread> m_pAudioDecodingThread;
};


#endif
