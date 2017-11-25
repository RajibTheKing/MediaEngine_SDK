
#ifndef IPV_FRAME_RATE_CONTROLLER_H
#define IPV_FRAME_RATE_CONTROLLER_H


#include "SmartPointer.h"
#include "EventNotifier.h"
#include "ThreadTools.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "VideoEncoder.h"

#include <queue>

namespace MediaSDK
{
	class CFrameRateController 
	{

		public:

			CFrameRateController(int fps);
			~CFrameRateController();

			void Reset(int fps);

			void SetFPS(int fps);
			int GetFrameStatus();

		private:

			int m_FPS;
			int m_TimePerFrame;
			int m_TimeThreshold;
			long long m_PreviousFrameTimeStamp;

			Tools m_Tools;
	};

} //namespace MediaSDK

#endif //ANDROIDTESTCLIENTVE_FTEST_FPSCONTROLLER_H
