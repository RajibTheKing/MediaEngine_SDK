
#include "AverageCalculator.h"
#include "VideoCallSession.h"


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

long long CAverageCalculator::GetTotal()
{
    return m_nTotalValue;
}

void CAverageCalculator::OperationTheatre(long long llOperationStartTime, CVideoCallSession *pVideoCallSession, string sOperationType)
{
    if(pVideoCallSession->GetCalculationStatus() == true)
    {
        long long currentTime = m_Tools.CurrentTimestamp();
        
        if(currentTime - pVideoCallSession->GetCalculationStartTime() <= 1000)
        {
            UpdateData(currentTime - llOperationStartTime);
        }
        else
        {
            
            CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, sOperationType + "--> TimeAVg = " + m_Tools.DoubleToString(GetAverage()));
            
            pVideoCallSession->SetCalculationStartMechanism(false);
        }
    }
}

