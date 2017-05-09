
#ifndef IPV_SYNCHRONIZED_MAP_H
#define IPV_SYNCHRONIZED_MAP_H

#include "SmartPointer.h"
#include "LockHandler.h"

#include <map>

namespace MediaSDK
{

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

} //namespace MediaSDK

#endif 
