#include "AudioMacros.h"
#include "vector"

class AudioMixer {
public:
	AudioMixer(int m_iNumberOfBitsPerSample, int iFrameSize);
	~AudioMixer();
	void addAudioData(unsigned char* uchCalleeAudio);
	int getAudioData(unsigned char* uchMixedAudioData);
	int removeAudioData(unsigned char* uchAudioDataToPlay, unsigned char* uchMixedAudioData, unsigned char* uchCalleeAudioData, long long calleeId, std::vector<std::pair<int, int>> &vMissingBlock);
	void reset(int iNumberOfBitsPerSample, int iFrameSize);
	void genCalleeChunkHeader(unsigned char* uchDestinaton, int iStartIndex, int iEndIndex, long long iCalleeId, int iFrameNumber, int iFrameSize, int iBlock, std::vector<std::pair<int, int>> &vMissingBlocks);
	int GetAudioFrameByParsingMixHeader(unsigned char *uchByteArray, long long nUserId);
	void Convert18BitTo16Bit(unsigned char* p18BitData, unsigned char* p16BitData, int nSampleSize);
	long long getMax(int bitLength);
	long long getMin(int bitLength);

private:
	int m_iTotalCallee;
	int m_iNumberOfBitsPerSample;
	int m_iCalleeMaskFlag;
	int m_iAudioFrameSize;
	int m_iTotalBlock;
	int m_iCalleeFrameInfoSize;

	int m_iCalleeIdLengthInByte;
	int m_iMissingMaskLengthInByte;
	int m_iFrameNumberLengthInByte;

	unsigned char m_uchCalleeBlockInfo[MAX_AUDIO_DECODER_FRAME_SIZE];

	int m_iMixedData[MAX_AUDIO_DECODER_FRAME_SIZE];

	long long readValue(unsigned char *uchByteArray, int &iIndexOffset, int &iBitOffset, int iReadBitLength);
	void writeValue(unsigned char *uchByteArray, int &iIndexOffset, int &iBitOffset, int iReadBitLength, long long iValue);	
};