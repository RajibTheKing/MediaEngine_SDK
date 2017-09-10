#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H


#include <mutex>
#include "CustomLocker.h"

namespace MediaSDK
{

	typedef std::mutex CLockHandler;

	//New locker implementation for common purpose
	class MediaLocker 
	{
	public:
		MediaLocker(CLockHandler* mediaMutex)
			: m_mutex(mediaMutex)
		{
			m_mutex->lock();
		}

		~MediaLocker()
		{
			m_mutex->unlock();
		}

	private:
		CLockHandler* m_mutex;
	};

} //namespace MediaSDK

#endif // !COMMON_TYPES_H
