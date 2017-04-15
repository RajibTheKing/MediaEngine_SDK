
#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#define OPUS_ENABLED

#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioPacketHeader.h"
#include "LogPrinter.h"
#include "LiveAudioDecodingQueue.h"
#include "AudioNearEndDataProcessor.h"
#include "AudioFarEndDataProcessor.h"


#include <stdio.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <vector>


//#define LOCAL_SERVER_LIVE_CALL
#ifdef LOCAL_SERVER_LIVE_CALL
#include "VideoSockets.h"
#endif

#include "Aac.h"

#ifdef OPUS_ENABLED
#include "AudioCodec.h"
#else
#include "G729CodecNative.h"
#endif


#define AUDIO_CALL_VERSION  0
#define AUDIO_LIVE_VERSION  0

#ifdef __ANDROID__
#define USE_AECM
// #define USE_ANS
#define USE_AGC
//#define USE_VAD
#endif

static string colon = "ALOG:";
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO,colon + a);




//#define AUDIO_SELF_CALL
//#define DUMP_FILE


//#define FIRE_ENC_TIME
//#define AUDIO_FIXED_COMPLEXITY

class CCommonElementsBucket;
class CVideoEncoder;
class CAudioCodec;
class CAac;
class AudioPacketizer;
class AudioDePacketizer;
class CAudioFarEndDataProcessor;

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
class CGomGomGain;

class CAudioCallSession
{
private:
	bool m_bIsAECMFarEndThreadBusy;
	bool m_bIsAECMNearEndThreadBusy;
	long long m_llLastPlayTime;

public:
	
	CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, int nEntityType);
    ~CAudioCallSession();

	void StartCallInLive(int iRole, int nCallInLiveType);
	void EndCallInLive();

	void SetCallInLiveType(int nCallInLiveType);

    CAudioCodec* GetAudioCodec();

    void InitializeAudioCallSession(LongLong llFriendID);
    int EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength);
	int CancelAudioData(short *psaEncodingAudioData, unsigned int unLength);
    
    int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);

	void SetVolume(int iVolume, bool bRecorder);
	void SetLoudSpeaker(bool bOn);
	void SetEchoCanceller(bool bOn);
	bool getIsAudioLiveStreamRunning();
	bool m_bIsPublisher;
#if 0	
	void GetAudioSendToData(unsigned char * pAudioRawDataToSendMuxed, int &RawLengthMuxed, std::vector<int> &vRawDataLengthVectorMuxed,
		std::vector<int> &vRawDataLengthVectorNonMuxed, int &RawLengthNonMuxed, unsigned char * pAudioNonMuxedDataToSend);
#endif

	void GetAudioSendToData(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);
    int GetServiceType();

	int m_iNextPacketType;
	CAudioShortBuffer m_AudioEncodingBuffer, m_AudioDecodedBuffer;
	CAudioShortBuffer  m_ViewerInCallSentDataQueue;
	int m_iPrevRecvdSlotID;
	int m_iReceivedPacketsInPrevSlot;
#ifdef USE_AGC
	CGain * m_pRecorderGain;
	CGain * m_pPlayerGain;
#endif

	CAudioNearEndDataProcessor *m_pNearEndProcessor = NULL;
	CAudioFarEndDataProcessor *m_pFarEndProcessor = NULL;

#ifdef DUMP_FILE
	FILE *FileInput;
	FILE *FileOutput;
	FILE *File18BitType;
	FILE *File18BitData;
	FILE *FileInputWithEcho;
	FILE *FileInputMuxed;
	FILE *FileInputPreGain;
#endif

private:

#ifdef LOCAL_SERVER_LIVE_CALL
	VideoSockets *m_clientSocket;
#endif


	int m_iRole;
    Tools m_Tools;
    LongLong m_FriendID;
	bool m_bEchoCancellerEnabled;

    CCommonElementsBucket* m_pCommonElementsBucket;
	
	bool m_bUsingLoudSpeaker;


    bool m_bLiveAudioStreamRunning;
    int m_nServiceType;
	int m_nEntityType;
    
	int m_iVolume;

    int m_iAudioVersionFriend;
    int m_iAudioVersionSelf;

	int m_nCallInLiveType;

	CAac *m_cAac;

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

#ifdef USE_VAD
	CVoice *m_pVoice;
#endif

#ifdef OPUS_ENABLED
	CAudioCodec *m_pAudioCodec = nullptr;
#else
	G729CodecNative *m_pG729CodecNative;
#endif

public: 
	void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
	void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llNow, long long &llLastTime, int iCurrentPacketNumber);
	int GetRole();

protected:

    SmartPointer<CLockHandler> m_pAudioCallSessionMutex;
    SmartPointer<std::thread> m_pAudioEncodingThread;
    SmartPointer<std::thread> m_pAudioDecodingThread;
};


#endif
