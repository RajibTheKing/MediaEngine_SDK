//
// Created by ipvision on 10/23/2016.
//

#include "LiveReceiver.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"
#include "Tools.h"
#include "ThreadTools.h"
#include "CommonElementsBucket.h"


LiveReceiver::LiveReceiver(CCommonElementsBucket* sharedObject):
m_pCommonElementsBucket(sharedObject)
{
    //m_pAudioDecoderBuffer = pAudioDecoderBuffer;
    m_pLiveAudioReceivedQueue = NULL;
    m_pLiveVideoDecodingQueue = NULL;
    m_pLiveReceiverMutex.reset(new CLockHandler);
}

LiveReceiver::~LiveReceiver(){
    SHARED_PTR_DELETE(m_pLiveReceiverMutex);
}
void LiveReceiver::SetVideoDecodingQueue(LiveVideoDecodingQueue *pQueue)
{
    m_pLiveVideoDecodingQueue = pQueue;
}
void LiveReceiver::SetAudioDecodingQueue(LiveAudioDecodingQueue *pQueue)
{
    m_pLiveAudioReceivedQueue = pQueue;
}


void LiveReceiver::PushVideoData(unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
    Locker lock(*m_pLiveReceiverMutex);
    int iUsedLen = 0, nFrames = 0;
	int packetSizeOfNetwork = m_pCommonElementsBucket->GetPacketSizeOfNetwork();
	int offset = packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING;
	int tillIndex = offset;
    //int frameCounter = 0;

	if (packetSizeOfNetwork < 0)
		return;

	for (int j = 0; iUsedLen < iLen;j++)
    {
        nFrames++;

		int indexOfThisFrame = tillIndex;
		int endOfThisFrame = indexOfThisFrame + frameSizes[j] - 1;
		int commonLeft, commonRight;
		bool bBroken = false;

//        LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 1");



		for (int i = 0; i < numberOfMissingFrames; i++)
		{
			commonLeft = max(indexOfThisFrame, missingFrames[i] * packetSizeOfNetwork);
			commonRight = min(endOfThisFrame,( (missingFrames[i] + 1) * packetSizeOfNetwork) - 1);

			if (commonLeft <= commonRight)
			{
				bBroken = true;
				break;
			}
		}

//        LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 2");

		if (bBroken && j == 0)	//If I frame is missing.
			return;
		
		
		/*
		if (j == 0)
		{
			if (0 < numberOfMissingFrames &&  endOfThisFrame >= missingFrames[0] * packetSizeOfNetwork && missingFrames[0] >= NUMBER_OF_HEADER_FOR_STREAMING)
				return;
		}
		else
		{
			for (int i = 0; i < numberOfMissingFrames-1; i++)
			{
				if (endOfThisFrame >= missingFrames[i] * packetSizeOfNetwork && endOfThisFrame < missingFrames[i+1] * packetSizeOfNetwork)
					continue;
			}

			if (numberOfMissingFrames > 0 && endOfThisFrame >= missingFrames[numberOfMissingFrames - 1] * packetSizeOfNetwork && endOfThisFrame < (iLen + offset))
				continue;
		}	
		*/
//      packetHeaderObj.setPacketHeader(uchVideoData + iUsedLen);

		if (!bBroken)	//If the current frame is not broken.
		{
//            LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 3 - before access data");
			int nCurrentFrameLen = ((int)uchVideoData[1 + iUsedLen + 13] << 8) + uchVideoData[1 + iUsedLen + 14];
            
			//LLG("#IV#    LiveReceiver::PushVideoData , nCurrentFrameLen = " + Tools::IntegertoStringConvert(nCurrentFrameLen));
//            LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 3 - get current frame");

            //int frameNumber = packetHeader.GetFrameNumberDirectly(uchVideoData + (iUsedLen +1) );
			//LOGEF("THeKing--> receive #####  [%d] Video FrameCounter = %d, FrameLength  = %d, UsedLen: %d iLen = %d\n",j, frameNumber, nCurrentFrameLen + PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE, iUsedLen, iLen);

			m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + PACKET_HEADER_LENGTH);
//			iUsedLen += nCurrentFrameLen + PACKET_HEADER_LENGTH + 1;
			//iUsedLen += LIVE_STREAMING_PACKETIZATION_PACKET_SIZE * ((nCurrentFrameLen + PACKET_HEADER_LENGTH + 1 + LIVE_STREAMING_PACKETIZATION_PACKET_SIZE - 1) / LIVE_STREAMING_PACKETIZATION_PACKET_SIZE);
		}
        else
        {
            //LOGEF("THeKing--> receive #####  [%d] Broken## UsedLen: %d iLen = %d\n",j, iUsedLen, iLen);
        }


        iUsedLen += frameSizes[j];
		tillIndex = endOfThisFrame + 1;
	}

//    m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen, iLen + PACKET_HEADER_LENGTH);
}

bool LiveReceiver::GetVideoFrame(unsigned char* uchVideoFrame,int iLen)
{
    Locker lock(*m_pLiveReceiverMutex);
    return false;
}

void LiveReceiver::ProcessAudioStream(int nOffset, unsigned char* uchAudioData,int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, int *pMissingBlocks, int nNumberOfMissingBlocks) {

    Locker lock(*m_pLiveReceiverMutex);

    int nCallSDKPacketLength = m_pCommonElementsBucket->GetPacketSizeOfNetwork();
    bool bCompleteFrame = false;
    int iMissingIndex = 0;
    int iFrameNumber = 0, nUsedLength = 0;
    int iLeftRange, iRightRange, nFrameLeftRange, nFrameRightRange;
    int nCurrentFrameLenWithMediaHeader;
    nFrameLeftRange = nOffset;

	if (nCallSDKPacketLength < 0)
		return;


    while (iFrameNumber < nNumberOfAudioFrames) {
        if (pMissingBlocks == NULL) {
//            LLG("#IV#    LiveReceiver::ProcessAudioStream pMissingBlocks is NULLLLLLL");
        }

        bCompleteFrame = true;
//        nFrameLeftRange = pAudioFramsStartingByte[iFrameNumber];

        nFrameLeftRange = nUsedLength + nOffset;
        nFrameRightRange = nFrameLeftRange + pAudioFramsStartingByte[iFrameNumber] - 1;
        nUsedLength += pAudioFramsStartingByte[iFrameNumber];

        //LLG("#IV# THeKing--> Audio  left, right, iframenum  = "+ Tools::IntegertoStringConvert(nFrameLeftRange)+","+Tools::IntegertoStringConvert(nFrameRightRange)+","+Tools::IntegertoStringConvert(iFrameNumber));

        while (iMissingIndex < nNumberOfMissingBlocks &&
               (pMissingBlocks[iMissingIndex] + 1) * nCallSDKPacketLength <= nFrameLeftRange)
            ++iMissingIndex;

        if (iMissingIndex < nNumberOfMissingBlocks) {
            iLeftRange = pMissingBlocks[iMissingIndex] * nCallSDKPacketLength;
            iRightRange = iLeftRange + nCallSDKPacketLength - 1;

            iLeftRange = max(nFrameLeftRange, iLeftRange);
            iRightRange = min(nFrameRightRange, iRightRange);
            if (iLeftRange <= iRightRange)
                bCompleteFrame = false;
        }

        ++iFrameNumber;

        if (!bCompleteFrame) {

            continue;
        } else {

            //LOGEF("THeKing--> #IV#    LiveReceiver::ProcessAudioStream Audio FRAME Completed -- FrameNumber = %d, CurrentFrameLenWithMediaHeadre = %d, audioFrameLength = %d ",audioFrameNumber , nFrameRightRange - nFrameLeftRange + 1, audioFrameLength);
        }

        nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;

        m_pLiveAudioReceivedQueue->EnQueue(uchAudioData + nFrameLeftRange + 1,
                                         nCurrentFrameLenWithMediaHeader - 1);

    }
}




void LiveReceiver::PushVideoDataVector(int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	Locker lock(*m_pLiveReceiverMutex);
	int numberOfMissingFrames = vMissingFrames.size();

	int iUsedLen = 0, nFrames = 0;
	int tillIndex = offset;
	//int frameCounter = 0;

	for (int j = 0; iUsedLen < iLen; j++)
	{
		nFrames++;

		int indexOfThisFrame = tillIndex;
		int endOfThisFrame = indexOfThisFrame + frameSizes[j] - 1;
		int commonLeft, commonRight;
		bool bBroken = false;

		//        LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 1");



		for (int i = 0; i < numberOfMissingFrames; i++)
		{
			commonLeft = max(indexOfThisFrame, vMissingFrames[i].first);
			commonRight = min(endOfThisFrame, vMissingFrames[i].second);

			if (commonLeft <= commonRight)
			{
				bBroken = true;
				break;
			}
		}

		//        LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 2");

		if (bBroken && j == 0)	//If I frame is missing.
			return;


		/*
		if (j == 0)
		{
		if (0 < numberOfMissingFrames &&  endOfThisFrame >= missingFrames[0] * packetSizeOfNetwork && missingFrames[0] >= NUMBER_OF_HEADER_FOR_STREAMING)
		return;
		}
		else
		{
		for (int i = 0; i < numberOfMissingFrames-1; i++)
		{
		if (endOfThisFrame >= missingFrames[i] * packetSizeOfNetwork && endOfThisFrame < missingFrames[i+1] * packetSizeOfNetwork)
		continue;
		}

		if (numberOfMissingFrames > 0 && endOfThisFrame >= missingFrames[numberOfMissingFrames - 1] * packetSizeOfNetwork && endOfThisFrame < (iLen + offset))
		continue;
		}
		*/
		//      packetHeaderObj.setPacketHeader(uchVideoData + iUsedLen);

		if (!bBroken)	//If the current frame is not broken.
		{
			//int nCurrentFrameLen = ((int)uchVideoData[1 + iUsedLen + 13] << 8) + uchVideoData[1 + iUsedLen + 14];
            
            CVideoHeader videoHeader;
            videoHeader.setPacketHeader(uchVideoData + 1 + iUsedLen);
            int nCurrentFrameLen = videoHeader.getPacketLength();

			//m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + PACKET_HEADER_LENGTH);
            m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + videoHeader.GetHeaderLength());
		}
		else
		{
			//LOGEF("THeKing--> receive #####  [%d] Broken## UsedLen: %d iLen = %d\n",j, iUsedLen, iLen);
		}


		iUsedLen += frameSizes[j];
		tillIndex = endOfThisFrame + 1;
	}

	//    m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen, iLen + PACKET_HEADER_LENGTH);
}


int iExpectedPacketNumber = 0;

void LiveReceiver::ProcessAudioStreamVector(int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int,int> > vMissingBlocks){

        Locker lock(*m_pLiveReceiverMutex);
		CAudioPacketHeader g_LiveReceiverHeader;
        size_t nNumberOfMissingBlocks = vMissingBlocks.size();
        size_t iMissingIndex = 0;

        bool bCompleteFrame = false;
        int iFrameNumber = 0, nUsedLength = 0;
        int iLeftRange, iRightRange, nFrameLeftRange, nFrameRightRange;
        int nCurrentFrameLenWithMediaHeader;
        nFrameLeftRange = nOffset;
		bool done = true;

        while(iFrameNumber < nNumberOfAudioFrames)
        {
            bCompleteFrame = true;

            nFrameLeftRange = nUsedLength + nOffset;
            nFrameRightRange = nFrameLeftRange + pAudioFramsStartingByte[ iFrameNumber ] - 1;
            nUsedLength += pAudioFramsStartingByte[ iFrameNumber ];

            //LLG("#IV# THeKing--> Audio  left, right, iframenum  = "+ Tools::IntegertoStringConvert(nFrameLeftRange)+","+Tools::IntegertoStringConvert(nFrameRightRange)+","+Tools::IntegertoStringConvert(iFrameNumber));

//            while(iMissingIndex < nNumberOfMissingBlocks &&  (pMissingBlocks[iMissingIndex] + 1) * nCallSDKPacketLength  <= nFrameLeftRange)
            while(iMissingIndex < nNumberOfMissingBlocks &&  vMissingBlocks[ iMissingIndex ].second <= nFrameLeftRange)
                ++ iMissingIndex;

            if(iMissingIndex < nNumberOfMissingBlocks)
            {
                iLeftRange = vMissingBlocks[iMissingIndex].first;
                iRightRange = vMissingBlocks[iMissingIndex].second;

                iLeftRange =  max(nFrameLeftRange, iLeftRange);
                iRightRange = min(nFrameRightRange,iRightRange);
                if(iLeftRange <= iRightRange)
                    bCompleteFrame = false;
            }

            ++ iFrameNumber;

			g_LiveReceiverHeader.CopyHeaderToInformation(uchAudioData + nFrameLeftRange + 1);
			int iPacketNumber = g_LiveReceiverHeader.GetInformation(PACKETNUMBER);
			if (iPacketNumber != iExpectedPacketNumber)
			{
				LOGE("live receiver unexpected PACKETNUMBER = %d iFrameNumber = %d nNumberOfAudioFrames = %d\n", iPacketNumber, iFrameNumber, nNumberOfAudioFrames);
			}
			else
			{
				LOGE("live receiver expected PACKETNUMBER = %d iFrameNumber = %d nNumberOfAudioFrames = %d\n", iPacketNumber, iFrameNumber, nNumberOfAudioFrames);
			}
			iExpectedPacketNumber = iPacketNumber + 1;
            if( !bCompleteFrame )
            {				
				LOGE("live receiver continue PACKETNUMBER = %d\n", iPacketNumber);
                continue;
            }else{
                //LOGEF("THeKing--> #IV#    LiveReceiver::ProcessAudioStream Audio FRAME Completed -- FrameNumber = %d, CurrentFrameLenWithMediaHeadre = %d, audioFrameLength = %d ",audioFrameNumber , nFrameRightRange - nFrameLeftRange + 1, audioFrameLength);
            }
			if (done)
			{
				done = false;
				LOGE("live receiver FIRST PACKETNUMBER = %d\n", iPacketNumber);
			}
			else
			{
				LOGE("live receiver PACKETNUMBER = %d\n", iPacketNumber);
			}
            nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;

            m_pLiveAudioReceivedQueue->EnQueue(uchAudioData + nFrameLeftRange +1 , nCurrentFrameLenWithMediaHeader - 1);

        }
}

