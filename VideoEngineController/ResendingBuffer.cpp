
#include "ResendingBuffer.h"

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include <string.h>
#include "LogPrinter.h"


CResendingBuffer::CResendingBuffer() :
m_iPushIndex(0),
m_iQueueCapacity(RESENDING_BUFFER_SIZE)
{
	memset(m_BufferFrameNumber, -1, sizeof(m_BufferFrameNumber));
	m_pChannelMutex.reset(new CLockHandler);
}

void CResendingBuffer::Reset()
{
	m_iPushIndex = 0;
	m_iQueueCapacity = RESENDING_BUFFER_SIZE;
	resendingMap.clear();
	memset(m_BufferFrameNumber, -1, sizeof(m_BufferFrameNumber));
}

CResendingBuffer::~CResendingBuffer()
{
	SHARED_PTR_DELETE(m_pChannelMutex);
}

void CResendingBuffer::Queue(unsigned char *frame, int length, int frameNumber, int packetNumber)
{
	Locker lock(*m_pChannelMutex);

	memcpy(m_Buffer[m_iPushIndex], frame, length);

	resendingMapIterator = resendingMap.find(make_pair(m_BufferFrameNumber[m_iPushIndex], m_BufferPacketNumber[m_iPushIndex]));

	if (resendingMapIterator != resendingMap.end())
		resendingMap.erase(resendingMapIterator);

	m_BufferDataLength[m_iPushIndex] = length;
	m_BufferFrameNumber[m_iPushIndex] = frameNumber;
	m_BufferPacketNumber[m_iPushIndex] = packetNumber;

	resendingMap[make_pair(frameNumber, packetNumber)] = m_iPushIndex;

	m_BufferInsertionTime[m_iPushIndex] = m_Tools.CurrentTimestamp();

	IncreamentIndex(m_iPushIndex);
}

int CResendingBuffer::DeQueue(unsigned char *decodeBuffer, int frameNumber, int packetNumber, int &timeStampDiff)
{
	Locker lock(*m_pChannelMutex);

	resendingMapIterator = resendingMap.find(make_pair(frameNumber, packetNumber));
	if (resendingMapIterator == resendingMap.end())
		return -1;

	m_iPopIndex = resendingMapIterator->second;
	resendingMap.erase(resendingMapIterator);
	memcpy(decodeBuffer, m_Buffer[m_iPopIndex], m_BufferDataLength[m_iPopIndex]);

	timeStampDiff = m_Tools.CurrentTimestamp() - m_BufferInsertionTime[m_iPopIndex];

	return m_BufferDataLength[m_iPopIndex];
}

void CResendingBuffer::IncreamentIndex(int &index)
{
	index++;
	if (index >= m_iQueueCapacity)
		index = 0;
}
