#include "AudioMacros.h"
#include "vector"

class AudioMixer {
public:
	AudioMixer(int m_iNumberOfBitsPerSample, int iFrameSize);
	~AudioMixer();
	void addAudioData(unsigned char* uchCalleeAudio);
	int getAudioData(unsigned char* uchMixedAudioData);
	int removeAudioData(unsigned char* uchAudioDataToPlay, unsigned char* uchMixedAudioData, unsigned char* uchCalleeAudioData, int calleeId);
	void reset(int iNumberOfBitsPerSample, int iFrameSize);
	void static genCalleeChunkHeader(unsigned char* uchDestinaton, int iStartIndex, int iEndIndex,int iCalleeId,int iFrameNumber,int iFrameSize, int iBlock, std::vector<std::pair<int, int>> &vMissingBlocks);
private:
	int m_iTotalCallee;
	int m_iNumberOfBitsPerSample;
	int m_iCalleeMaskFlag;
	int m_iAudioFrameSize;
	int m_iTotalBlock;
	int m_iCalleeFrameInfoSize;

	unsigned char m_uchCalleeBlockInfo[MAX_AUDIO_DECODER_FRAME_SIZE];

	int m_iMixedData[MAX_AUDIO_DECODER_FRAME_SIZE];

	int readValue(unsigned char *uchByteArray, int &iIndexOffset, int &iBitOffset, int iReadBitLength);
	void writeValue(unsigned char *uchByteArray, int &iIndexOffset, int &iBitOffset, int iReadBitLength, int iValue);
};