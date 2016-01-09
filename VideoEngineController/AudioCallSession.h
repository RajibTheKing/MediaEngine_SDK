#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#include <stdio.h>
#include <string>
#include <map>

#include "AudioEncoder.h"
#include "AudioDecoder.h"
#include "LockHandler.h"

#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"

#include "G729CodecNative.h"
#include "Tools.h"

#define MAX_AUDIO_FRAME_LENGHT 4096

class CCommonElementsBucket;
class CVideoEncoder;

#ifdef __ANDROID__

static int codec_open = 0;

#endif

class CAudioCallSession
{

public:

	CAudioCallSession(LongLong fname, CCommonElementsBucket* sharedObject);
	~CAudioCallSession();

	LongLong GetFriendID();
	void InitializeAudioCallSession(LongLong lFriendID);
	int EncodeAudioData(short *in_data, unsigned int in_size);
	int DecodeAudioData(unsigned char *in_data, unsigned int in_size);
	CAudioEncoder* GetAudioEncoder();
	CAudioDecoder* GetAudioDecoder();
    
    void EncodingThreadProcedure();
    void StopEncodingThread();
    void StartEncodingThread();
    
    void DecodingThreadProcedure();
    void StopDecodingThread();
    void StartDecodingThread();
    
    static void *CreateAudioEncodingThread(void* param);
    static void *CreateAudioDecodingThread(void* param);

private:
    Tools m_Tools;
	LongLong friendID;
    
    CAudioEncoderBuffer m_EncodingBuffer;
    CAudioDecoderBuffer m_DecodingBuffer;

	short m_EncodingFrame[MAX_AUDIO_FRAME_LENGHT];
    unsigned char m_EncodedFrame[MAX_AUDIO_FRAME_LENGHT];
    unsigned char m_DecodingFrame[MAX_AUDIO_FRAME_LENGHT];
	short m_DecodedFrame[MAX_AUDIO_FRAME_LENGHT];

	CCommonElementsBucket* m_pCommonElementsBucket;
	CAudioEncoder *m_pAudioEncoder;
	CAudioDecoder *m_pAudioDecoder;
    
    bool bEncodingThreadRunning;
    bool bEncodingThreadClosed;
    
    bool bDecodingThreadRunning;
    bool bDecodingThreadClosed;

    G729CodecNative *m_pG729CodecNative;

protected:

	SmartPointer<CLockHandler> m_pSessionMutex;
    
    SmartPointer<std::thread> pEncodingThread;
    
    SmartPointer<std::thread> pDecodingThread;
};


#endif
