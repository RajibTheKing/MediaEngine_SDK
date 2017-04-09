#ifndef SPEEX_ECHO_CANCELLER_H
#define SPEEX_ECHO_CANCELLER_H


#include "EchoCancellerInterface.h"
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#include "Tools.h"
#include "AudioMacros.h"


class SpeexEchoCanceller : public EchoCancellerInterface
{
private:
	bool m_bFarendArrived;
	bool m_bReadingFarend, m_bWritingFarend, m_bWritingDump;

	short m_sSpeexFarendBuf[MAX_AUDIO_FRAME_SAMPLE_SIZE];

	SpeexEchoState *st;
	SpeexPreprocessState *den;
	Tools m_Tools;


public:

	SpeexEchoCanceller();
	
	~SpeexEchoCanceller();

	int AddFarEndData(short *farEndData, int dataLen, bool isLiveStreamRunning);
	
	int CancelEchoFromNearEndData(short *nearEndData, int dataLen, bool isLiveStreamRunning);

};


#endif  // !SPEEX_ECHO_CANCELLER_H