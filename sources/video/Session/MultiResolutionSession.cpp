//
// Created by Fahad-PC on 9/27/2017.
//

#include "MultiResolutionSession.h"
#include "LogPrinter.h"

namespace MediaSDK {


    MultiResolutionSession::MultiResolutionSession(CCommonElementsBucket *pCommonElementsBucket)
    {
        m_pCommonElementsBucket = pCommonElementsBucket;
        m_pMultiResolutionThread = NULL;
    }

    MultiResolutionSession::~MultiResolutionSession() {
        if (NULL != m_pMultiResolutionThread) {
            m_pMultiResolutionThread->StopMultiResolutionThread();
            delete m_pMultiResolutionThread;
            m_pMultiResolutionThread = NULL;
        }

        if (NULL != m_pVideoFrameBuffer) {
            delete m_pVideoFrameBuffer;
            m_pVideoFrameBuffer = NULL;
        }
    }

    void MultiResolutionSession::Initialize(int *targetHeight, int *targetWidth, int iLen)
    {
        m_pVideoFrameBuffer = new VideoFrameBuffer();
        m_pMultiResolutionThread = new MultiResolutionThread(m_pVideoFrameBuffer,  m_pCommonElementsBucket, targetHeight, targetWidth, iLen);

        m_pMultiResolutionThread->StartMultiResolutionThread();
    }

    int MultiResolutionSession::PushIntoBuffer(unsigned char *in_data, int iLen)
    {
        printf("fahad -->>  MultiResolutionSession::PushIntoBuffer == iLen = %d ******* \n", iLen);
        m_pVideoFrameBuffer->Queue(in_data, iLen);
		return 1;
    }
}