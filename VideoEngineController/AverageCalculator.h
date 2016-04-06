
#ifndef _AVERAGE_CALCULATOR_
#define _AVERAGE_CALCULATOR_
#include<iostream>
using namespace std;

class CAverageCalculator
{
public:
    CAverageCalculator();
    void Reset();
    void UpdateData(long long nValue);
    double GetAverage();
private:
    double m_dAvg;
    int m_nCounter;
    long long m_nTotalValue;
    
};


#endif //_AVERAGE_CALCULATOR_
