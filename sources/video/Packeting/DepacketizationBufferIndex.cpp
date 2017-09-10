//
// Created by ipvision on 1/16/2016.
//

#include "DepacketizationBufferIndex.h"

DepacketizationBufferIndex::DepacketizationBufferIndex() { }

DepacketizationBufferIndex::~DepacketizationBufferIndex() { }

void DepacketizationBufferIndex::Initialize(int nSize)
{
    m_nBufferSize = nSize;
    m_iBaseIndex = 0;
}

void DepacketizationBufferIndex::Reset(int nBaseFrame)
{
    m_nBaseFrame = nBaseFrame;
    m_iBaseIndex = 0;
}

int DepacketizationBufferIndex::GetIndex(int nFrame){
    return (nFrame - m_nBaseFrame + m_iBaseIndex) % m_nBufferSize;
}

void DepacketizationBufferIndex::DeleteFrame(int nFrame){
    if(nFrame != m_nBaseFrame) {
        //Invalid use;
        ;
    }
    ++m_iBaseIndex;
    ++m_nBaseFrame;
}

