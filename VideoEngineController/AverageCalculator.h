
#ifndef _AVERAGE_CALCULATOR_
#define _AVERAGE_CALCULATOR_
#include<iostream>
#include "Tools.h"


using namespace std;

class CVideoCallSession;

class CAverageCalculator
{
public:
    CAverageCalculator();
    void Reset();
    void UpdateData(long long nValue);
    double GetAverage();
    long long GetTotal();
    
    void OperationTheatre(long long llOperationStartTime, CVideoCallSession *pVideoCallSession, string sOperationType);

    
private:
    double m_dAvg;
    int m_nCounter;
    long long m_llTotalValue;
    Tools m_Tools;
};


#endif //_AVERAGE_CALCULATOR_
