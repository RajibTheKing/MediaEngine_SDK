
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
    
    void CalculateFPS(string sTag);

    void StartingOperation(string sOperationName);
    void EndingOperation();
private:
    double m_dAvg;
    int m_nCounter;
    long long m_llTotalValue;
    Tools m_Tools;
    
    long long m_llPrevFPSTime;
    int m_iFpsCounter;
    
    
    long long m_llStartingTimeStamp;
    long long m_llEndingTimeStamp;
    string m_sOperationTag;
};


#endif //_AVERAGE_CALCULATOR_
