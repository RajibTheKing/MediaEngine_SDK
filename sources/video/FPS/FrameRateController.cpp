//
// Created by ipvision on 1/2/2016.
//

#include "FrameRateController.h"
#include "Size.h"
#include <math.h>
#include <map>

namespace MediaSDK
{
	CFrameRateController::CFrameRateController(int fps)
	{
		m_FPS = fps;
		m_TimePerFrame = 1000 / fps;
		m_PreviousFrameTimeStamp = m_Tools.CurrentTimestamp();
		m_TimeThreshold = 0;
	}

	CFrameRateController::~CFrameRateController()
	{
		
	}

	void CFrameRateController::Reset(int fps)
	{
		m_FPS = fps;
		m_TimePerFrame = 1000 / fps;
		m_PreviousFrameTimeStamp = m_Tools.CurrentTimestamp();
		m_TimeThreshold = 0;
	}

	void CFrameRateController::SetFPS(int fps)
	{
		m_FPS = fps;
		m_TimePerFrame = 1000 / fps;
		m_TimeThreshold = 0;
	}

	int CFrameRateController::GetFrameStatus()
	{
		long long currentTimeStamp = m_Tools.CurrentTimestamp();

		int timeDiff = (int)(currentTimeStamp - m_PreviousFrameTimeStamp);

		CLogPrinter_LOG(CHUNK_RECIVE_LOG, "CFrameRateController::GetFrameStatus currentTimeStamp %lld m_PreviousFrameTimeStamp %lld timeDiff %d", currentTimeStamp, m_PreviousFrameTimeStamp, timeDiff);
		
		if (timeDiff >= m_TimePerFrame * 41 / 50 - m_TimeThreshold)
		{
			m_TimeThreshold = max(0, timeDiff - m_TimePerFrame * 41 / 50);
			m_PreviousFrameTimeStamp = currentTimeStamp;
			
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
} //namespace MediaSDK
