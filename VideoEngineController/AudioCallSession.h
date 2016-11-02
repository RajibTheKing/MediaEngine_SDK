
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
#include <vector>
#ifdef OPUS_ENABLE
#include "AudioCodec.h"
#else
#include "G729CodecNative.h"
#endif

#define ALOG(a)     CLogPrinter_WriteSpecific(CLogPrinter::INFO,a);

//#define __AUDIO_FIXED_COMPLEXITY__
#define __AUDIO_FIXED_BITRATE__

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
	int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);

    void EncodingThreadProcedure();
    void StopEncodingThread();
    void StartEncodingThread();

    void DecodingThreadProcedure();
    void StopDecodingThread();
    void StartDecodingThread();

    static void *CreateAudioEncodingThread(void* param);
    static void *CreateAudioDecodingThread(void* param);
	int m_iNextPacketType;
#ifdef ONLY_FOR_LIVESTREAMING
	void getAudioSendToData(unsigned char * pAudioDataToSend, int &length, std::vector<int> &vDataLengthVector);
#endif

private:

    Tools m_Tools;
    LongLong m_FriendID;

    CAudioPacketHeader *SendingHeader;
    CAudioPacketHeader *ReceivingHeader;
    int m_AudioHeadersize;

    CCommonElementsBucket* m_pCommonElementsBucket;
    CAudioCodecBuffer m_AudioEncodingBuffer;
    CAudioDecoderBuffer m_AudioDecodingBuffer;
#ifdef ONLY_FOR_LIVESTREAMING
    std::vector<int> m_vEncodedFrameLenght;
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
#ifdef ONLY_FOR_LIVESTREAMING
	unsigned char m_ucaAudioDataToSend[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	int m_iAudioDataSendIndex;
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
