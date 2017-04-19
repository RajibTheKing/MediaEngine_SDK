#ifndef AUDIO_ENCODER_OPUS_H
#define AUDIO_ENCODER_OPUS_H


#include <stdio.h>
#include <string>

#include "AudioEncoderInterface.h"
#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "opus.h"
#include "Tools.h"
#include "AudioMacros.h"
#include "size.h"


#define OPUS_MIN_COMPLEXITY 1
#define OPUS_MAX_COMPLEXITY 10
#define BYTES_TO_STORE_AUDIO_EFRAME_LEN 2


namespace IPV
{
	class thread;
}

class CCommonElementsBucket;
class CAudioCallSession;


class AudioEncoderOpus : public AudioEncoderInterface
{
private:

	CCommonElementsBucket* m_pCommonElementsBucket;
	CAudioCallSession* m_pAudioCallSession;
	int m_iCurrentBitRate;
	int m_inoLossSlot;
	int m_ihugeLossSlot;
	int m_iComplexity;

	bool m_bAudioQualityLowNotified;
	bool m_bAudioQualityHighNotified;
	bool m_bAudioShouldStopNotified;

	OpusEncoder	*encoder;
	int 		err;

	opus_int16 m_DummyData[MAX_AUDIO_FRAME_SAMPLE_SIZE + 10];
	unsigned char m_DummyDataOut[MAX_AUDIO_FRAME_SAMPLE_SIZE * 2 + 10];

	LongLong m_FriendID;
	Tools m_Tools;

protected:

	SmartPointer<CLockHandler> m_pMediaSocketMutex;


public:

	AudioEncoderOpus(CCommonElementsBucket* sharedObject, CAudioCallSession * AudioCallSession, LongLong llfriendID);
	~AudioEncoderOpus();

	int CreateAudioEncoder();
	int encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);

	bool SetBitrate(int nBitrate);
	bool SetComplexity(int nComplexity);

	int GetCurrentBitrate();

	void DecideToChangeBitrate(int iNumPacketRecvd);
	void DecideToChangeComplexity(int iEncodingTime);

};


#endif  // !AUDIO_ENCODER_OPUS_H
