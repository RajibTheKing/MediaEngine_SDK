
#include "AverageCalculator.h"
#include "VideoCallSession.h"

namespace MediaSDK
{

	CAverageCalculator::CAverageCalculator()
	{
		m_nCounter = 0;
		m_dAvg = 0.0;
		m_llTotalValue = 0;

		m_llPrevFPSTime = -1;
		m_iFpsCounter = 0;
	}
	void CAverageCalculator::Reset()
	{
		m_nCounter = 0;
		m_dAvg = 0.0;
	}
	void CAverageCalculator::UpdateData(long long nValue)
	{
		m_nCounter++;
		m_llTotalValue += nValue;
	}
	double CAverageCalculator::GetAverage()
	{
		m_dAvg = (m_llTotalValue*1.0) / (m_nCounter*1.0);
		return m_dAvg;
	}

	long long CAverageCalculator::GetTotal()
	{
		return m_llTotalValue;
	}

	void CAverageCalculator::OperationTheatre(long long llOperationStartTime, CVideoCallSession *pVideoCallSession, string sOperationType)
	{
		if (pVideoCallSession->GetCalculationStatus() == true)
		{
			long long currentTime = m_Tools.CurrentTimestamp();

			if (currentTime - pVideoCallSession->GetCalculationStartTime() <= 1000)
			{
				UpdateData(currentTime - llOperationStartTime);
			}
			else
			{

				//    CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, sOperationType + "--> TimeAVg = " + m_Tools.DoubleToString(GetAverage()));

				pVideoCallSession->SetCalculationStartMechanism(false);
			}
		}
	}

	void CAverageCalculator::CalculateFPS(string sTag)
	{
		m_iFpsCounter++;
		if (m_llPrevFPSTime == -1)
		{
			m_llPrevFPSTime = m_Tools.CurrentTimestamp();
		}

		if (m_Tools.CurrentTimestamp() - m_llPrevFPSTime >= 1000)
		{
			printf("%s %d\n", sTag.c_str(), m_iFpsCounter);

#ifdef __ANDROID__

			LOGE("%s %d\n", sTag.c_str(), m_iFpsCounter);

#endif

			m_llPrevFPSTime = m_Tools.CurrentTimestamp();
            m_iDeviceFPS = m_iFpsCounter;
			m_iFpsCounter = 0;
		}
	}

	void CAverageCalculator::StartingOperation(string sOperationName)
	{
		m_llStartingTimeStamp = m_Tools.CurrentTimestamp();
		m_sOperationTag = sOperationName;
	}

	void CAverageCalculator::EndingOperation()
	{
		m_llEndingTimeStamp = m_Tools.CurrentTimestamp();
		printf("Operation Time for %s  = %d\n", m_sOperationTag.c_str(), m_llEndingTimeStamp - m_llStartingTimeStamp);
	}
    
    int CAverageCalculator::getDeviceFPS()
    {
        return m_iDeviceFPS;
    }

} //namespace MediaSDK
