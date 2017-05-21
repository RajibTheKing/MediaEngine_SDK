#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H


#include <mutex>


typedef std::mutex CLockHandler;
typedef std::lock_guard<CLockHandler> Locker;



#endif // !COMMON_TYPES_H
