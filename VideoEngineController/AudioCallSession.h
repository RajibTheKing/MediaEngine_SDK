
#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioPacketHeader.h"
#include "LogPrinter.h"
#include "LiveAudioDecodingQueue.h"
#include "AudioNearEndDataProcessor.h"
#include "AudioFarEndDataProcessor.h"
#include "AudioShortBufferForPublisherFarEnd.h"

#include <stdio.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <vector>
#include <thread>


//#define LOCAL_SERVER_LIVE_CALL
#ifdef LOCAL_SERVER_LIVE_CALL
#include "VideoSockets.h"
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
class AudioDePacketizer;
class CAudioFarEndDataProcessor;

class AudioEncoderInterface;
class AudioDecoderInterface;
class EchoCancellerInterface;
class NoiseReducerInterface;
class AudioGainInterface;

#ifdef USE_VAD
class CVoice;
#endif

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
	long long GetBaseOfRelativeTime();

	SmartPointer<AudioEncoderInterface> GetAudioEncoder()
	{
		return m_pAudioEncoder;
	}

	SmartPointer<AudioDecoderInterface> GetAudioDecoder()
	{
		return m_pAudioDecoder;
	}

	//SmartPointer<NoiseReducerInterface> GetNoiseReducer()
	//{
	//	return m_pNoise;
	//}

    void InitializeAudioCallSession();
    int EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength);
	int CancelAudioData(short *psaEncodingAudioData, unsigned int unLength);
    
    int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);

	void SetVolume(int iVolume, bool bRecorder);
	void SetLoudSpeaker(bool bOn);
	void SetEchoCanceller(bool bOn);
	bool getIsAudioLiveStreamRunning();
	int GetEntityType();
	bool m_bIsPublisher;

	void GetAudioSendToData(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);
    int GetServiceType();

	int m_iNextPacketType;
	CAudioShortBuffer m_AudioEncodingBuffer;
	CAudioShortBuffer  m_ViewerInCallSentDataQueue;

	AudioShortBufferForPublisherFarEnd m_PublisherBufferForMuxing;
	int m_iPrevRecvdSlotID;
	int m_iReceivedPacketsInPrevSlot;

	SmartPointer<AudioGainInterface> m_pRecorderGain;
	SmartPointer<AudioGainInterface> m_pPlayerGain;

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

	SmartPointer<EchoCancellerInterface> m_pEcho;

//	SmartPointer<NoiseReducerInterface> m_pNoise;

	SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
	SmartPointer<AudioDecoderInterface> m_pAudioDecoder;

#ifdef USE_VAD
	CVoice *m_pVoice;
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
