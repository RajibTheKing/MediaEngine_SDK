
#ifndef IPV_AVERAGE_CALCULATOR_H
#define IPV_AVERAGE_CALCULATOR_H

#include<iostream>
#include "Tools.h"

namespace MediaSDK
{

	using namespace std;

	class CVideoCallSession;

	class CAverageCalculator
	{
	public:
		CAverageCalculator();
		void Reset();
		void UpdateData(long long nValue);
		double GetAverage();
        
		long long GetTotal()
        {
            return m_llTotalValue;
        }

		void OperationTheatre(long long llOperationStartTime, CVideoCallSession *pVideoCallSession, string sOperationType);

		void CalculateFPS(string sTag);

		void StartingOperation(string sOperationName);
		void EndingOperation();
        
        int GetDeviceFPS()
        {
            return m_iDeviceFPS;
        }
        
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
        
        int m_iDeviceFPS;
	};

} //namespace MediaSDK

#endif //_AVERAGE_CALCULATOR_
