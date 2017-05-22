#include "Trace.h"

void CTrace::GenerateTrace(short *sBuffer, int iTraceLength)
{
	for (int i = 0; i < iTraceLength; i++)
	{
		if (i % 8 < 4)
		{
			sBuffer[i] = 30000;
		}
		else
		{
			sBuffer[i] = -30000;
		}
	}
}

bool CTrace::DetectTrace(short *sBuffer, int iTraceSearchLength, int iTraceDetectionLength)
{
	for (int i = 0; i < iTraceSearchLength - iTraceDetectionLength; i++)
	{
		if (sBuffer[i] > 0 && sBuffer[i + 1] > 0 && sBuffer[i + 2] > 0 && sBuffer[i + 3] > 0) //maybe trace started
		{
			int j = 0;
			for (; j < iTraceDetectionLength; j++)
			{
				if (j % 8 < 4 && sBuffer[i + j] < 0)
				{
					break;
				}
				if (j % 8 >= 4 && sBuffer[i + j] > 0)
				{
					break;
				}

			}
			if (j == iTraceDetectionLength)
			{
				return true;
			}
		}
	}
	return false;
}