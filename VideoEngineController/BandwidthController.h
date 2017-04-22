
#ifndef IPV_BANDWIDTH_CONTROLLER_H
#define IPV_BANDWIDTH_CONTROLLER_H

#include <vector>
#include "Tools.h"

#define ROUTER_BUFFER_SIZE 2*1024
#define BAND_WIDTH_IN_BYTE 512*1024
#define UDP_HEADER_SIZE 68


class BandwidthController {
public:
    BandwidthController();
    ~BandwidthController();
    void SetTimeInterval(std::vector<int>&BandWidthList, std::vector<int>&TimePeriod);
    bool IsSendeablePacket(int nPacketLen);

private:
    std::vector<int>m_BandWidthList;
    std::vector<int>m_TimePeriod;

    int m_iNextTimePeriodIndex;
    long long m_BandWidth;
    long long m_LastTimeStamp;
    long long m_NextIntervalStartTime;
    int m_nDataToSendInByte;
    int m_TimePeriodListSize;
    Tools m_Tools;
};


#endif //ANDROIDTESTCLIENTVE_FTEST_BANDWIDTHCONTROLLER_H
