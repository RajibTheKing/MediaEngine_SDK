//
// Created by MD MAKSUD HOSSAIN on 12/31/15.
//

#include "PairMap.h"
#include "ThreadTools.h"

PairMap::PairMap(){
    m_pMutex.reset(new CLockHandler);
}
PairMap::~PairMap(){
    SHARED_PTR_DELETE(m_pMutex);
}
void PairMap::setTime(int nFrameNumber,int nPacketNumber){
    Locker lock(*m_pMutex);
    long long time  = m_Tools.CurrentTimestamp();
    m_map[std::make_pair(nFrameNumber,nPacketNumber)] = time;
}
long long PairMap::getTimeDiff(int nFrameNumber,int nPacketNumber){
    Locker lock(*m_pMutex);
    if(m_map.find(std::make_pair(nFrameNumber,nPacketNumber))==m_map.end()) return -1;
    long long ret = m_Tools.CurrentTimestamp() - m_map[std::make_pair(nFrameNumber,nPacketNumber)];
    m_map.clear();
    return ret;
}
