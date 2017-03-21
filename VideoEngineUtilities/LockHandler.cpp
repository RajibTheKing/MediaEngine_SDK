
#include "LockHandler.h"

CLockHandler::CLockHandler() :
m_pMutex(NULL)

{
	m_pMutex = new std::mutex;
}

CLockHandler::~CLockHandler()
{
/*	
	if (m_pMutex != NULL)
	{
		delete m_pMutex;

		m_pMutex = NULL;
	}
*/
}

std::mutex* CLockHandler::GetMutex()
{
	if (NULL == m_pMutex)
		return NULL;

	return m_pMutex;
}

void CLockHandler::Lock()
{
	if (NULL == m_pMutex)
		return;

	m_pMutex->lock();
}

void CLockHandler::UnLock()
{
	if (NULL == m_pMutex)
		return;

	m_pMutex->unlock();
}