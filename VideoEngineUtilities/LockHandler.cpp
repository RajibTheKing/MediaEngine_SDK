#include "LockHandler.h"

CLockHandler::CLockHandler() :
m_pSingleSemaphore(NULL)
{
	/*if (m_pSingleSemaphore != NULL)
	{
		delete m_pSingleSemaphore;

		m_pSingleSemaphore = NULL;
	}*/

	m_pSingleSemaphore = new std::mutex;
}

CLockHandler::~CLockHandler()
{
/*	if (m_pSingleSemaphore != NULL)
	{
		delete m_pSingleSemaphore;

		m_pSingleSemaphore = NULL;
	}*/
}

std::mutex* CLockHandler::GetMutex()
{
	if (NULL == m_pSingleSemaphore)
		return NULL;

	return m_pSingleSemaphore;
}


void CLockHandler::Lock()
{
	if (NULL == m_pSingleSemaphore)
		return;

	m_pSingleSemaphore->lock();
}

void CLockHandler::UnLock()
{
	if (NULL == m_pSingleSemaphore)
		return;

	m_pSingleSemaphore->unlock();
}