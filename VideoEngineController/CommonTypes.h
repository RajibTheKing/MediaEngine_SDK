#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H


#include <mutex>

namespace MediaSDK
{

	typedef std::mutex CLockHandler;
	typedef std::lock_guard<CLockHandler> Locker;

} //namespace MediaSDK

#endif // !COMMON_TYPES_H
