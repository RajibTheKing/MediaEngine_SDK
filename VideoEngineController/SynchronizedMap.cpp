
#include "SynchronizedMap.h"
#include "ThreadTools.h"

CSynchronizedMap::CSynchronizedMap()
{
	m_pSynchronizedMapMutex.reset(new CLockHandler);
}

CSynchronizedMap::~CSynchronizedMap()
{
	SHARED_PTR_DELETE(m_pSynchronizedMapMutex);
}

void CSynchronizedMap::clear()
{
	Locker lock(*m_pSynchronizedMapMutex);

	m_STLMapSynchronizedMap.clear();
}

int CSynchronizedMap::find(int iIndex)
{
	Locker lock(*m_pSynchronizedMapMutex);

	if (m_STLMapSynchronizedMap.find(iIndex) == m_STLMapSynchronizedMap.end())
	{
		return -1;
	}
	else
		return m_STLMapSynchronizedMap[iIndex];
}

int CSynchronizedMap::getElementAt(int iIndex)
{
	Locker lock(*m_pSynchronizedMapMutex);

	return m_STLMapSynchronizedMap[iIndex];
}

int CSynchronizedMap::end()
{
	return -1;
}

void CSynchronizedMap::insert(int iIndex, int nElement)
{
	Locker lock(*m_pSynchronizedMapMutex);

	m_STLMapSynchronizedMap[iIndex] = nElement;
}

void CSynchronizedMap::erase(int iIndex)
{
	Locker lock(*m_pSynchronizedMapMutex);
    //printf("map size = %ld, iIndex = %d\n", m_STLMapSynchronizedMap.size(), iIndex);
    while(m_STLMapSynchronizedMap.begin()->first <= iIndex)
    {
        m_STLMapSynchronizedMap.erase(m_STLMapSynchronizedMap.begin());
    }
}