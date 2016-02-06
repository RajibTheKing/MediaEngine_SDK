
#include "SynchronizedMap.h"

#include <string.h>
#include "LogPrinter.h"

CSynchronizedMap::CSynchronizedMap()
{
	m_pSynchronizedMapMutex.reset(new CLockHandler);
}

CSynchronizedMap::~CSynchronizedMap()
{
/*	if (m_pSynchronizedMapMutex.get())
		m_pSynchronizedMapMutex.reset();*/
}

void CSynchronizedMap::clear()
{
	Locker lock(*m_pSynchronizedMapMutex);

	m_STLMap.clear();
}

int CSynchronizedMap::find(int index)
{
	Locker lock(*m_pSynchronizedMapMutex);

	if(m_STLMap.find(index) == m_STLMap.end())
	{
		return -1;
	}
	else
		return m_STLMap[index];
}

int CSynchronizedMap::getElementAt(int index)
{
	Locker lock(*m_pSynchronizedMapMutex);

	return m_STLMap[index];
}

int CSynchronizedMap::end()
{
	return -1;
}

int CSynchronizedMap::insert(int index, int element)
{
	Locker lock(*m_pSynchronizedMapMutex);

	m_STLMap[index] = element;
}

void CSynchronizedMap::erase(int index)
{
	Locker lock(*m_pSynchronizedMapMutex);

	if(m_STLMap.find(index) != m_STLMap.end())
	{
		m_STLMap.erase(index);
	}
}