
#ifndef IPV_FPS_CONTROLLER_H
#define IPV_FPS_CONTROLLER_H


#include "SmartPointer.h"
#include "EventNotifier.h"
#include "ThreadTools.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "VideoEncoder.h"

#include <queue>

#define FPS_SHOULD_SAME 0
#define FPS_SHOULD_INCREASE 1
#define FPS_SHOULD_DECREASE 2

namespace MediaSDK
{

	class CFPSController {

	public:
		CFPSController(int nFPS);
		~CFPSController();

		void Reset(int nFPS);
		int GetOpponentFPS() const;
		void SetOpponentFPS(int OpponentFPS);
		int GetOwnFPS() const;
		void SetOwnFPS(int nOwnFPS);
		void SetClientFPS(double fps);
		double GetClientFPS();

		unsigned char GetFPSSignalByte();
		void SetFPSSignalByte(unsigned char signal);
		bool IsProcessableFrame();

		void SetMaxOwnProcessableFPS(int fps);
		int GetMaxOwnProcessableFPS();

		void SetEncoder(CVideoEncoder *videoEncoder);

	private:
		int m_nFPSForceSignalCounter;
		queue<int>m_SignalQue;
		int m_EncodingFrameCounter;
		double m_DropSum;
		double m_ClientFPS;
		int m_nFPSChangeSignal;
		int m_nForceFPSFlag;
		int m_nMaxOwnProcessableFPS;
		int m_nMaxOpponentProcessableFPS;
		int m_iFrameDropIntervalCounter;
		//    int m_iFrameCompletedIntervalCounter;
		int m_nOwnFPS;
		int m_nOpponentFPS;
		int m_nCallFPS;

		long long m_LastIntervalStartingTime;
		SharedPointer<CLockHandler> m_pMutex;
		Tools m_Tools;

		CVideoEncoder *m_pVideoEncoder;
	};

} //namespace MediaSDK

#endif //ANDROIDTESTCLIENTVE_FTEST_FPSCONTROLLER_H
