
#include "AverageCalculator.h"


CAverageCalculator::CAverageCalculator()
{
    m_nCounter = 0;
    m_dAvg = 0.0;
}
void CAverageCalculator::Reset()
{
    m_nCounter = 0;
    m_dAvg = 0.0;
}
void CAverageCalculator::UpdateData(long long nValue)
{
    m_nCounter++;
    m_nTotalValue+=nValue;
}
double CAverageCalculator::GetAverage()
{
    m_dAvg = (m_nTotalValue*1.0)/(m_nCounter*1.0);
    return m_dAvg;
}

