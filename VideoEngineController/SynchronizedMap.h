
#ifndef _SYNCHRONIZED_MAP_H_
#define _SYNCHRONIZED_MAP_H_

#include "SmartPointer.h"
#include "LockHandler.h"

#include <map>

class CSynchronizedMap
{

public:

	CSynchronizedMap();
	~CSynchronizedMap();

	void clear();
	int find(int iIndex);
	int getElementAt(int iIndex);
	int end();
	void insert(int iIndex, int nElement);
	void erase(int iIndex);
	void eraseAllSmallerEqual(int iIndex);

private:

	std::map<int, int> m_STLMapSynchronizedMap;

	SmartPointer<CLockHandler> m_pSynchronizedMapMutex;
};

#endif 
