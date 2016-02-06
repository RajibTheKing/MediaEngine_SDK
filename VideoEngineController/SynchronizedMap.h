
#ifndef _BANDWIDTH_RATIO_HELPER_H_
#define _BANDWIDTH_RATIO_HELPER_H_

#include "LockHandler.h"
#include "Size.h"
#include "Tools.h"
#include "SmartPointer.h"

#include <map>

class CSynchronizedMap
{

public:

	CSynchronizedMap();
	~CSynchronizedMap();

	void clear();
	int find(int index);
	int getElementAt(int index);
	int end();
	int insert(int index, int element);

private:

	map<int, int> m_STLMap;

	SmartPointer<CLockHandler> m_pSynchronizedMapMutex;
};

#endif 
