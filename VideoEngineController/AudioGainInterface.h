#ifndef AUDIO_GAIN_INTERFACE_H
#define AUDIO_GAIN_INTERFACE_H


#define MAX_GAIN 10
#define DEFAULT_GAIN 3

class AudioGainInterface
{

public:

	virtual int SetGain(int iGain) = 0;

	virtual int AddFarEnd(short *sInBuf, int nBufferSize) = 0;

	virtual int AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning) = 0;

	virtual ~AudioGainInterface() { }
};


#endif  // !AUDIO_GAIN_INTERFACE_H


// AudioGainInterface <-- WebRTCGain / CGomGomGain / NaiveGain <-- AudioGainInstanceProvider 