//
// Created by ipvision on 10/23/2016.
//

#include "LiveReceiver.h"
#include "AudioPacketHeader.h"
#include "PacketHeader.h"
#include "Tools.h"
#include "ThreadTools.h"



LiveReceiver::LiveReceiver(CAudioDecoderBuffer *pAudioDecoderBuffer, LiveVideoDecodingQueue *pLiveVideoDecodingQueue){
    m_pAudioDecoderBuffer = pAudioDecoderBuffer;
    m_pLiveVideoDecodingQueue = pLiveVideoDecodingQueue;
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
    m_pLiveAudioDecodingQueue = pQueue;
}

void LiveReceiver::PushAudioData(unsigned char* uchAudioData, int iLen, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames){
    Locker lock(*m_pLiveReceiverMutex);
    int iUsedLen = 0, nFrames = 0;
    CAudioPacketHeader audioPacketHeaderObject;

    while(iUsedLen < iLen)
    {
        nFrames++;
        audioPacketHeaderObject.CopyHeaderToInformation(uchAudioData + iUsedLen+1);
        int nCurrentFrameLen = audioPacketHeaderObject.GetInformation(PACKETLENGTH);
        printf("THeKing--> Audio FrameCounter = %d, FrameLength  = %d, iLen = %d\n", nFrames, nCurrentFrameLen, iLen);
        m_pAudioDecoderBuffer->Queue(uchAudioData + iUsedLen+1, nCurrentFrameLen + __AUDIO_HEADER_LENGTH__);
        iUsedLen += nCurrentFrameLen + __AUDIO_HEADER_LENGTH__+1;
    }
}

void LiveReceiver::PushVideoData(unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
    Locker lock(*m_pLiveReceiverMutex);
    int iUsedLen = 0, nFrames = 0;
    CPacketHeader packetHeaderObj;
	int offset = __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ * NUMBER_OF_HEADER_FOR_STREAMING;
	int tillIndex = offset;
    int frameCounter = 0;

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
			commonLeft = max(indexOfThisFrame, missingFrames[i] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__);
			commonRight = min(endOfThisFrame,( (missingFrames[i] + 1) * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__) - 1);

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
			if (0 < numberOfMissingFrames &&  endOfThisFrame >= missingFrames[0] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ && missingFrames[0] >= NUMBER_OF_HEADER_FOR_STREAMING)
				return;
		}
		else
		{
			for (int i = 0; i < numberOfMissingFrames-1; i++)
			{
				if (endOfThisFrame >= missingFrames[i] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ && endOfThisFrame < missingFrames[i+1] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__)
					continue;
			}

			if (numberOfMissingFrames > 0 && endOfThisFrame >= missingFrames[numberOfMissingFrames - 1] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ && endOfThisFrame < (iLen + offset))
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

            CPacketHeader packetHeader;
            int frameNumber = packetHeader.GetFrameNumberDirectly(uchVideoData + (iUsedLen +1) );
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

void LiveReceiver::ProcessAudioStream(int nOffset, unsigned char* uchAudioData,int nDataLenght, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, int *pMissingBlocks, int nNumberOfMissingBlocks){

    Locker lock(*m_pLiveReceiverMutex);

    int nCallSDKPacketLength = __MEDIA_DATA_SIZE_IN_LIVE_PACKET__;
    bool bCompleteFrame = false;
    int iMissingIndex = 0;
    int iFrameNumber = 0, nUsedLength = 0;
    int iLeftRange, iRightRange, nFrameLeftRange, nFrameRightRange;
    int nCurrentFrameLenWithMediaHeader;
    nFrameLeftRange = nOffset;


    while(iFrameNumber < nNumberOfAudioFrames)
    {
        if(pMissingBlocks == NULL)
        {
//            LLG("#IV#    LiveReceiver::ProcessAudioStream pMissingBlocks is NULLLLLLL");
        }

        bCompleteFrame = true;
//        nFrameLeftRange = pAudioFramsStartingByte[iFrameNumber];

        nFrameLeftRange = nUsedLength + nOffset;
        nFrameRightRange = nFrameLeftRange + pAudioFramsStartingByte[ iFrameNumber ] - 1;
        nUsedLength += pAudioFramsStartingByte[ iFrameNumber ];

        //LLG("#IV# THeKing--> Audio  left, right, iframenum  = "+ Tools::IntegertoStringConvert(nFrameLeftRange)+","+Tools::IntegertoStringConvert(nFrameRightRange)+","+Tools::IntegertoStringConvert(iFrameNumber));

        while(iMissingIndex < nNumberOfMissingBlocks &&  (pMissingBlocks[iMissingIndex] + 1) * nCallSDKPacketLength  <= nFrameLeftRange)
            ++ iMissingIndex;

        if(iMissingIndex < nNumberOfMissingBlocks)
        {
            iLeftRange = pMissingBlocks[iMissingIndex] * nCallSDKPacketLength;
            iRightRange =  iLeftRange + nCallSDKPacketLength - 1;

            iLeftRange =  max(nFrameLeftRange, iLeftRange);
            iRightRange = min(nFrameRightRange,iRightRange);
            if(iLeftRange <= iRightRange)
                bCompleteFrame = false;
        }

        ++ iFrameNumber;

        if( !bCompleteFrame )
        {
            CAudioPacketHeader audioPacketHeader;
            int audioFrameNumber = audioPacketHeader.GetInformation(PACKETNUMBER);
            //LOGEF("THeKing--> #IV#    LiveReceiver::ProcessAudioStream Audio FRAME not Completed -- FrameNumber = %d", audioFrameNumber);
            continue;
        }else{
            CAudioPacketHeader audioPacketHeader;
            int audioFrameNumber = audioPacketHeader.GetInformation(PACKETNUMBER);
            int audioFrameLength = audioPacketHeader.GetInformation(PACKETLENGTH);
            //LOGEF("THeKing--> #IV#    LiveReceiver::ProcessAudioStream Audio FRAME Completed -- FrameNumber = %d, CurrentFrameLenWithMediaHeadre = %d, audioFrameLength = %d ",audioFrameNumber , nFrameRightRange - nFrameLeftRange + 1, audioFrameLength);
        }

        nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;


        //m_pAudioDecoderBuffer->Queue(uchAudioData + nFrameLeftRange +1 , nCurrentFrameLenWithMediaHeader - 1);
        m_pLiveAudioDecodingQueue->Queue(uchAudioData + nFrameLeftRange +1 , nCurrentFrameLenWithMediaHeader - 1);

//        iUsedLen += nCurrentFrameLenWithMediaHeader;
//        audioPacketHeaderObject.CopyHeaderToInformation(uchAudioData + iUsedLen + 1);
//        int nCurrentFrameLen = audioPacketHeaderObject.GetInformation(PACKETLENGTH);
//        int nCurrentFrameLen = audioPacketHeaderObject.GetInformation(PACKETLENGTH);
//        LLG("#IV# THeKing--> Audio  FrameLength  = "+ Tools::IntegertoStringConvert(nCurrentFrameLenWithMediaHeader));

    }
}

