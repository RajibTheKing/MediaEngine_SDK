
#include "CircularBuffer.h"
#include "LogPrinter.h"


namespace MediaSDK
{

	CCircularBuffer::CCircularBuffer(int nBufferSize) :

		m_iPushIndex(0),
		m_iPopIndex(0),
		m_nQueueSize(0),
		m_nQueueCapacity(nBufferSize)

	{
		m_pRenderingBufferMutex.reset(new CLockHandler);
	}

	CCircularBuffer::~CCircularBuffer()
	{

	}

	void CCircularBuffer::ResetBuffer()
	{
		Locker lock(*m_pRenderingBufferMutex);

		m_iPushIndex = 0;
		m_iPopIndex = 0;
		m_nQueueSize = 0;
	}

	void CCircularBuffer::IncreamentIndex(int &riIndex)
	{
		riIndex++;

		if (riIndex >= m_nQueueCapacity)
			riIndex = 0;
	}

	int CCircularBuffer::GetQueueSize()
	{
		Locker lock(*m_pRenderingBufferMutex);

		return m_nQueueSize;
	}

} //namespace MediaSDK
