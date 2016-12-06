#include <vector>

#include "Tools.h"

enum LiveHeaderPositionInsidePacket
{
	versionFieldPosition = 0,             /*size 1*/
	relativeTimestampFieldPosition = 1,   /*size 4*/
	audioSizeFieldPosition = 5,           /*size 3*/
	videoSizeFieldPosition = 8,           /*size 3*/
	audioFrameCountFieldPosition = 11     /*size 1*/
	//numberOfVideoFrameFieldPosition 
};

enum LiveHeaderFieldLength
{
	versionFieldLength = 1,
	relativeTimestampFieldLength = 4,
	audioSizeFieldLength = 3,
	videoSizeFieldLength = 3,
	audioFramesCountFieldLength = 1,
	videoFrameCountFieldLength = 1,
	singleFrameSizeFieldLength = 3
};

class LiveStreamHeader
{
private:
	int m_iVersion;
	int m_iRelativeTimestamp;

	int m_iAudioDataSize;
	int m_iVideoDataSize;

	std::vector<int> m_vAudioFrameSizeList;

	std::vector<int> m_vVideoFrameSizeList;

	int m_iStaticHeaderSize;

public:
	LiveStreamHeader()
	{
		clear();
		m_iStaticHeaderSize = versionFieldLength + relativeTimestampFieldLength + audioSizeFieldLength
			+ videoSizeFieldLength + audioFramesCountFieldLength + videoFrameCountFieldLength;
	}

	void setVersion(int version){ m_iVersion = version; }
	int  getVersion(){ return m_iVersion; }

	void setRelativeTimeStamp(int timestamp) { m_iRelativeTimestamp = timestamp; }
	int  getRelativeTimeStamp() { return m_iRelativeTimestamp; }

	void setAudioDataSize(int size) { m_iAudioDataSize = size; }
	int  getAudioDataSize() { return m_iAudioDataSize; }

	void setVideoDataSize(int size) { m_iVideoDataSize = size; }
	int  getVideoDataSize() { return m_iVideoDataSize; }


	void setAudioFramesSizeList(const std::vector<int> list);
	void setVideoFramesSizeList(const std::vector<int> list);
	std::vector<int> getAudioFramesSizeList();
	std::vector<int> getVideoFramesSizeList();
	int getHeaderSize();
	void writeHeaderTo(unsigned char* destination);
	void readHeaderFrom(unsigned char* source);
	void clear();
};