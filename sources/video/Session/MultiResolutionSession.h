//
// Created by Fahad-PC on 9/27/2017.
//

#ifndef ANDROIDTESTCLIENTVE_FTEST_MULTIRESOLUTIONSESSION_H
#define ANDROIDTESTCLIENTVE_FTEST_MULTIRESOLUTIONSESSION_H

#include "ColorConverter.h"
#include "VideoFrameBuffer.h"
#include "CommonElementsBucket.h"
#include "MultiResolutionThread.h"

namespace MediaSDK {

    class MultiResolutionSession {

    public:
        MultiResolutionSession(CCommonElementsBucket *pCommonElementsBucket);

        ~MultiResolutionSession();

        void Initialize(int *targetHeight, int *targetWidth, int iLen);

        int PushIntoBuffer(unsigned char *in_data, int iLen );


    private:

        VideoFrameBuffer *m_pVideoFrameBuffer;
        MultiResolutionThread *m_pMultiResolutionThread;

        CCommonElementsBucket *m_pCommonElementsBucket;

    };

}
#endif //ANDROIDTESTCLIENTVE_FTEST_MULTIRESOLUTIONSESSION_H
