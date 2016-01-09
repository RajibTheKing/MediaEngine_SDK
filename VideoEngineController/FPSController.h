//
// Created by ipvision on 1/2/2016.
//

#ifndef ANDROIDTESTCLIENTVE_FTEST_FPSCONTROLLER_H
#define ANDROIDTESTCLIENTVE_FTEST_FPSCONTROLLER_H


#include "SmartPointer.h"
#include "EventNotifier.h"
#include "ThreadTools.h"
#include "LockHandler.h"
#include "Tools.h"

#define FPS_SHOULD_SAME 0
#define FPS_SHOULD_INCREASE 1
#define FPS_SHOULD_DECREASE 2


class CFPSController {

public:
    CFPSController();
    ~CFPSController();

    void Reset();
    int GetOpponentFPS() const;
    void SetOpponentFPS(int OpponentFPS);
    int GetOwnFPS() const;
    void SetOwnFPS(int nOwnFPS);
    void SetClientFPS(double fps);

    unsigned char GetFPSSignalByte();
    void SetFPSSignalByte(unsigned char signal);
    bool IsProcessableFrame();

    int NotifyFrameComplete(int framNumber);
    int NotifyFrameDropped(int framNumber);

private:
    queue<int>m_SignalQue;
    int m_EncodingFrameCounter;
    double m_DropSum;
    double m_ClientFPS;
    int m_nFPSChangeSignal;
    int m_iFrameDropIntervalCounter;
//    int m_iFrameCompletedIntervalCounter;
    int m_nOwnFPS;
    int m_nOpponentFPS;
    long long m_LastIntervalStartingTime;
    SmartPointer<CLockHandler> m_pMutex;
    Tools m_Tools;
};


#endif //ANDROIDTESTCLIENTVE_FTEST_FPSCONTROLLER_H
