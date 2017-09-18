//
// Created by ipvision on 1/2/2016.
//

#include "FPSController.h"
#include "Size.h"
#include <math.h>
#include <map>

#define FORCE_FPS_BIT_INDEX 5
#define FORCE_FPS_SIGNAL_TIMES 2

namespace MediaSDK
{

	CFPSController::CFPSController(int nFPS){
		m_pMutex.reset(new CLockHandler);

		m_nCallFPS = nFPS;

		m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
		m_ClientFPS = DEVICE_FPS_MAXIMUM;
		m_nOwnFPS = m_nOpponentFPS = m_nCallFPS;
		m_iFrameDropIntervalCounter = 0;
		m_EncodingFrameCounter = 0;
		m_DropSum = 0;
		m_nForceFPSFlag = 0;
		m_nMaxOwnProcessableFPS = m_nMaxOpponentProcessableFPS = m_nCallFPS;
		m_nFPSForceSignalCounter = 0;
	}

	CFPSController::~CFPSController(){
		while (!m_SignalQue.empty())
			m_SignalQue.pop();
		SHARED_PTR_DELETE(m_pMutex);
	}

	void CFPSController::Reset(int nFPS){
		m_nCallFPS = nFPS;
		m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
		m_ClientFPS = DEVICE_FPS_MAXIMUM;
		m_nOwnFPS = m_nOpponentFPS = m_nCallFPS;
		m_iFrameDropIntervalCounter = 0;
		m_EncodingFrameCounter = 0;
		m_DropSum = 0;
		m_nForceFPSFlag = 0;
		m_nMaxOwnProcessableFPS = m_nMaxOpponentProcessableFPS = m_nCallFPS;
		m_nFPSForceSignalCounter = 0;
	}

	int CFPSController::GetOpponentFPS() const {
		return m_nOpponentFPS;
	}

	void CFPSController::SetOpponentFPS(int OpponentFPS) {
		FPSControllerLocker lock(*m_pMutex);
		if (OpponentFPS > 0)
		{
			m_nOpponentFPS = OpponentFPS;
		}
		else
		{
			printf("Tried to set opp fps to 0\n");
		}
	}

	int CFPSController::GetOwnFPS() const {
		return m_nOwnFPS;
	}

	void CFPSController::SetOwnFPS(int nOwnFPS){

		if (nOwnFPS > 0)
		{
			m_nOwnFPS = nOwnFPS;
		}
		else
		{
			printf("Tried to set own fps to 0\n");
		}

	}

	void CFPSController::SetMaxOwnProcessableFPS(int fps)
	{
		FPSControllerLocker lock(*m_pMutex);
		m_nFPSForceSignalCounter = FORCE_FPS_SIGNAL_TIMES;
		m_nMaxOwnProcessableFPS = fps;
	}

	int CFPSController::GetMaxOwnProcessableFPS(){
		return m_nMaxOwnProcessableFPS;
	}


	void CFPSController::SetClientFPS(double fps){
		if (1 > fps)
		{
			return;
		}
		FPSControllerLocker lock(*m_pMutex);
		m_ClientFPS = fps;
	}

	double CFPSController::GetClientFPS(){
		return m_ClientFPS;
	}

	unsigned char CFPSController::GetFPSSignalByte()
	{
		unsigned char ret = m_nOwnFPS;
		unsigned char changeSignal = 0;

		if (m_nFPSForceSignalCounter > 0)
		{
			--m_nFPSForceSignalCounter;
			ret = m_nMaxOwnProcessableFPS;
			ret |= (1 << FORCE_FPS_BIT_INDEX);  //Set Force FPS Flag.
		}
		else if (m_ClientFPS < m_nCallFPS)
		{
			int tmp = (int)(m_ClientFPS + 0.5);
			if (m_nOwnFPS > tmp)
				ret = tmp;
		}

		//    if(changeSignal == 0)
		//        ret |= 0xC0;    //Version Detectability Flag.

		return ret;
	}


	void CFPSController::SetFPSSignalByte(unsigned char signalByte)
	{
		if (0 == signalByte)   return;
		int bIsForceFPS = (signalByte & (1 << FORCE_FPS_BIT_INDEX)) >> FORCE_FPS_BIT_INDEX;
		int opponentFPS = 0x1F & signalByte;    //5 Bits is used for FPS.

		if (bIsForceFPS)
		{
			FPSControllerLocker lock(*m_pMutex);

			if (m_nCallFPS >= opponentFPS)
				m_nMaxOpponentProcessableFPS = opponentFPS;

			if (m_nOwnFPS > m_nMaxOpponentProcessableFPS) {
				SetOwnFPS(m_nMaxOpponentProcessableFPS);
			}

		}
		else if (opponentFPS != m_nOpponentFPS) {
			FPSControllerLocker lock(*m_pMutex);
			m_nOpponentFPS = opponentFPS;
		}

		if (m_nOwnFPS > m_ClientFPS)
		{
			FPSControllerLocker lock(*m_pMutex);
			SetOwnFPS((int)m_ClientFPS);
		}
		else if (m_nOwnFPS < m_ClientFPS && m_nOwnFPS < m_nCallFPS)
		{
			FPSControllerLocker lock(*m_pMutex);
			SetOwnFPS(min(m_nCallFPS, (int)m_ClientFPS));
		}
	}

	bool CFPSController::IsProcessableFrame()
	{
		Tools tools;
		//printf(" CFPSController::IsProcessableFrame--> m_nOwnFPS = %d, m_ClientFPS = %lf\n", m_nOwnFPS, m_ClientFPS);
		if (m_nOwnFPS + FPS_COMPARISON_EPS > m_ClientFPS) return true;

		double diff = m_ClientFPS - m_nOwnFPS;

		double ratio = (double)(m_ClientFPS*1.0) / (diff*1.0);

		if (m_EncodingFrameCounter == 0)
		{
			m_DropSum += ratio;
		}

		int indx = (int)floor(m_DropSum + 0.5);
		m_EncodingFrameCounter++;

		if (m_EncodingFrameCounter == indx)
		{
			m_DropSum += ratio;
			return false;
		}

		return true;
	}

	void CFPSController::SetEncoder(CVideoEncoder *videoEncoder)
	{
		m_pVideoEncoder = videoEncoder;
	}

} //namespace MediaSDK
