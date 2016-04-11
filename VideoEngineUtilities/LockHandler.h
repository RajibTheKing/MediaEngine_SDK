
#ifndef _LOCK_HANDLER_H_
#define _LOCK_HANDLER_H_

#include <stdio.h>
#include <mutex>

#ifdef __APPLE__
#include <pthread.h>
#endif

#include "AudioVideoEngineDefinitions.h"

class Locker;

class CLockHandler
{

public:

	CLockHandler();
	~CLockHandler();

	std::mutex* GetMutex();
	void Lock();
	void UnLock();

private:

	std::mutex *m_pSingleSemaphore;

};

class Locker
{
	CLockHandler& mutex;

public:

	Locker(CLockHandler& m):
	mutex(m) 
	{ 
		mutex.Lock(); 
	}

	~Locker()
	{ 
		mutex.UnLock(); 
	}
};


#endif

