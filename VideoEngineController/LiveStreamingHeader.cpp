#include "LiveStreamingHeader.h"

void LiveStreamHeader::setAudioFramesSizeList(const std::vector<int> list)
{
	m_vAudioFrameSizeList = list;
}

void LiveStreamHeader::setVideoFramesSizeList(const std::vector<int> list)
{
	m_vVideoFrameSizeList = list;
}

std::vector<int> LiveStreamHeader::getAudioFramesSizeList()
{
	return m_vAudioFrameSizeList;
}

std::vector<int> LiveStreamHeader::getVideoFramesSizeList()
{
	return m_vVideoFrameSizeList;
}

int LiveStreamHeader::getHeaderSize()
{
	return m_iStaticHeaderSize + ((m_vVideoFrameSizeList.size() + m_vAudioFrameSizeList.size()) * singleFrameSizeFieldLength);
}

void LiveStreamHeader::writeHeaderTo(unsigned char* destination)
{
	destination[versionFieldPosition] = (unsigned char)m_iVersion;
	Tools::SetIntegerIntoUnsignedChar(destination, headerLengthPosition, headerLengthFieldLength, m_iHeaderLength);
	Tools::SetIntegerIntoUnsignedChar(destination, chunkDurationPosition, chunkDurationFieldLength, m_iChunkDuration);
	Tools::SetIntegerIntoUnsignedChar(destination, relativeTimestampFieldPosition, relativeTimestampFieldLength, m_iRelativeTimestamp);
	Tools::SetIntegerIntoUnsignedChar(destination, audioSizeFieldPosition, audioSizeFieldLength, m_iAudioDataSize);
	Tools::SetIntegerIntoUnsignedChar(destination, videoSizeFieldPosition, videoSizeFieldLength, m_iVideoDataSize);

	Tools::SetIntegerIntoUnsignedChar(destination, audioFrameCountFieldPosition, audioFramesCountFieldLength, m_vAudioFrameSizeList.size());
	int dynamicPosition = audioFrameCountFieldPosition + audioFramesCountFieldLength;

	for (int a : m_vAudioFrameSizeList)
	{
		Tools::SetIntegerIntoUnsignedChar(destination, dynamicPosition, singleFrameSizeFieldLength, a);
		dynamicPosition += singleFrameSizeFieldLength;
	}

	Tools::SetIntegerIntoUnsignedChar(destination, dynamicPosition, videoFrameCountFieldLength, m_vVideoFrameSizeList.size());
	dynamicPosition += videoFrameCountFieldLength;
	for (int a : m_vVideoFrameSizeList)
	{
		Tools::SetIntegerIntoUnsignedChar(destination, dynamicPosition, singleFrameSizeFieldLength, a);
		dynamicPosition += singleFrameSizeFieldLength;
	}
}

void LiveStreamHeader::readHeaderFrom(unsigned char* source)
{
	clear();

	m_iVersion = source[versionFieldPosition];
	m_iHeaderLength = Tools::GetIntegerFromUnsignedChar(source, headerLengthPosition, headerLengthFieldLength);
	m_iChunkDuration = Tools::GetIntegerFromUnsignedChar(source, chunkDurationPosition, chunkDurationFieldLength);
	m_iRelativeTimestamp = Tools::GetIntegerFromUnsignedChar(source, relativeTimestampFieldPosition, relativeTimestampFieldLength);
	m_iAudioDataSize = Tools::GetIntegerFromUnsignedChar(source, audioSizeFieldPosition, audioSizeFieldLength);
	m_iVideoDataSize = Tools::GetIntegerFromUnsignedChar(source, videoSizeFieldPosition, videoSizeFieldLength);

	int nNumberOfAudioFrame = Tools::GetIntegerFromUnsignedChar(source, audioFrameCountFieldPosition, audioFramesCountFieldLength);
	int dynamicPosition = audioFrameCountFieldPosition + audioFramesCountFieldLength;
	for (int i = 0; i < nNumberOfAudioFrame; i++)
	{
		m_vAudioFrameSizeList.push_back(Tools::GetIntegerFromUnsignedChar(source, dynamicPosition, singleFrameSizeFieldLength));
		dynamicPosition += singleFrameSizeFieldLength;
	}

	int nNumberOfVideoFrame = Tools::GetIntegerFromUnsignedChar(source, dynamicPosition, videoFrameCountFieldLength);
	dynamicPosition += videoFrameCountFieldLength;
	for (int i = 0; i < nNumberOfVideoFrame; i++)
	{
		m_vVideoFrameSizeList.push_back(Tools::GetIntegerFromUnsignedChar(source, dynamicPosition, singleFrameSizeFieldLength));
		dynamicPosition += singleFrameSizeFieldLength;
	}
}

void LiveStreamHeader::clear()
{
	m_iVersion = m_iRelativeTimestamp = m_iAudioDataSize = m_iVideoDataSize = m_iChunkDuration = m_iHeaderLength = 0;
	m_vVideoFrameSizeList.clear();
	m_vAudioFrameSizeList.clear();
}