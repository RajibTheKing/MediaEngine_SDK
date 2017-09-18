//
// Created by ipvision on 1/16/2016.
//

#ifndef ANDROIDTESTCLIENTVE_FTEST_DEPACKETIZATIONBUFFERINDEX_H
#define ANDROIDTESTCLIENTVE_FTEST_DEPACKETIZATIONBUFFERINDEX_H


class DepacketizationBufferIndex {
public:
    DepacketizationBufferIndex();
    ~DepacketizationBufferIndex();
    int GetIndex(int nFrame);
    void DeleteFrame(int nFrame);
    void Reset(int nBaseFrame);
    void Initialize(int nSize);  //Should be set before using.

private:
    int m_nBaseFrame;
    int m_iBaseIndex;
    int m_nBufferSize;

};


#endif //ANDROIDTESTCLIENTVE_FTEST_DEPACKETIZATIONBUFFERINDEX_H
