
#ifndef _SYNCHRONIZED_MAP_H_
#define _SYNCHRONIZED_MAP_H_

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include "Size.h"

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

private:

	map<int, int> m_STLMapSynchronizedMap;

	SmartPointer<CLockHandler> m_pSynchronizedMapMutex;
};

#endif 
