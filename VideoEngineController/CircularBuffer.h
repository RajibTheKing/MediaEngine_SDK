
#ifndef IPV_CIRCULAR_BUFFER_H
#define IPV_CIRCULAR_BUFFER_H

#include "SmartPointer.h"
#include "Tools.h"

namespace MediaSDK
{

	class CLockHandler;

	class CCircularBuffer
	{

	public:

		CCircularBuffer(int nBufferSize);
		~CCircularBuffer();

		void IncreamentIndex(int &riIndex);
		int GetQueueSize();
		void ResetBuffer();

	private:

		int m_iPushIndex;
		int m_iPopIndex;
		int m_nQueueCapacity;
		int m_nQueueSize;

		Tools m_Tools;
		SmartPointer<CLockHandler> m_pRenderingBufferMutex;
	};

} //namespace MediaSDK

#endif 
