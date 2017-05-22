
#ifndef IPV_PAIRMAP_H
#define IPV_PAIRMAP_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Tools.h"
#include <map>

namespace MediaSDK
{
	using namespace std;

	class PairMap {
	public:
		PairMap();
		void setTime(int nFrameNumber, int nPacketNumber);
		long long getTimeDiff(int nFrameNumber, int nPacketNumber);
		~PairMap();
	private:
		Tools m_Tools;
    	map< pair<int,int>,long long> m_map;
		SmartPointer<CLockHandler> m_pMutex;
	};

} //namespace MediaSDK

#endif //ANDROIDTESTCLIENTVE_FTEST_PAIRMAP_H
