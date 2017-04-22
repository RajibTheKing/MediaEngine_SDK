
#ifndef IPV_PAIRMAP_H
#define IPV_PAIRMAP_H

#include "SmartPointer.h"
#include "LockHandler.h"
#include "Tools.h"
#include <map>


class PairMap {
public:
    PairMap();
    void setTime(int nFrameNumber,int nPacketNumber);
    long long getTimeDiff(int nFrameNumber,int nPacketNumber);
    ~PairMap();
private:
    Tools m_Tools;
    std::map<pair<int,int>,long long> m_map;
    SmartPointer<CLockHandler> m_pMutex;
};


#endif //ANDROIDTESTCLIENTVE_FTEST_PAIRMAP_H
