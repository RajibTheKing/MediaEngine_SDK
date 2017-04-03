#include "AudioMacros.h"

class AudioMixer {
public:
	AudioMixer(int m_iNumberOfBitsPerSample, int iFrameSize);
	~AudioMixer();
private:
	int m_iTotalCallee;
	int m_iNumberOfBitsPerSample;
	int m_iCalleeMaskFlag;
	int m_iAudioFrameSize;
	int m_iTotalBlock;
	int m_iCalleeFrameInfoSize;

	unsigned char m_uchCalleeBlockInfo[MAX_AUDIO_DECODER_FRAME_SIZE];

	int m_iMixedData[MAX_AUDIO_DECODER_FRAME_SIZE];
	void addAudioData(unsigned char* uchCalleeAudio);
	int getAudioData(unsigned char* uchMixedAudioData);
};