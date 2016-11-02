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
	int tillIndex = 0;
    
	for (int j = 0; iUsedLen < iLen;j++)
    {
        nFrames++;

		int indexOfThisFrame = tillIndex;
		int endOfThisFrame = indexOfThisFrame + frameSizes[j] - 1;

		if (j == 0)
		{

			if ( 0 < numberOfMissingFrames &&  endOfThisFrame >= missingFrames[0] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__)
				return;
		}
		else
		{
			for (int i = 0; i < numberOfMissingFrames-1; i++)
			{
				if (endOfThisFrame >= missingFrames[i] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ && endOfThisFrame < missingFrames[i+1] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__)
					continue;
			}

			if ( numberOfMissingFrames > 0 &&   endOfThisFrame >= missingFrames[numberOfMissingFrames - 1] * __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ && endOfThisFrame < iLen)
				continue;
		}	

//      packetHeaderObj.setPacketHeader(uchVideoData + iUsedLen);
        
        int nCurrentFrameLen = ((int)uchVideoData[1+iUsedLen+13] << 8) + uchVideoData[1+iUsedLen+14];
        LLG("#IV#    LiveReceiver::PushVideoData , nCurrentFrameLen = " + Tools::IntegertoStringConvert(nCurrentFrameLen));
        printf("THeKing--> Video FrameCounter = %d, FrameLength  = %d, iLen = %d\n", nFrames, nCurrentFrameLen, iLen);
        
        m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen+1, nCurrentFrameLen + PACKET_HEADER_LENGTH);
        iUsedLen += nCurrentFrameLen + PACKET_HEADER_LENGTH + 1;
		//iUsedLen += LIVE_STREAMING_PACKETIZATION_PACKET_SIZE * ((nCurrentFrameLen + PACKET_HEADER_LENGTH + 1 + LIVE_STREAMING_PACKETIZATION_PACKET_SIZE - 1) / LIVE_STREAMING_PACKETIZATION_PACKET_SIZE);
   
		tillIndex += endOfThisFrame + 1;
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

        LLG("#IV# THeKing--> Audio  left, right, iframenum  = "+ Tools::IntegertoStringConvert(nFrameLeftRange)+","+Tools::IntegertoStringConvert(nFrameRightRange)+","+Tools::IntegertoStringConvert(iFrameNumber));

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

        if( !bCompleteFrame )
        {
            LLG("#IV#    LiveReceiver::ProcessAudioStream Audio FRAME not Completed");
            continue;
        }

        nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;
        ++ iFrameNumber;

        m_pAudioDecoderBuffer->Queue(uchAudioData + nFrameLeftRange +1 , nCurrentFrameLenWithMediaHeader - 1);

//        iUsedLen += nCurrentFrameLenWithMediaHeader;
//        audioPacketHeaderObject.CopyHeaderToInformation(uchAudioData + iUsedLen + 1);
//        int nCurrentFrameLen = audioPacketHeaderObject.GetInformation(PACKETLENGTH);
//        int nCurrentFrameLen = audioPacketHeaderObject.GetInformation(PACKETLENGTH);
//        LLG("#IV# THeKing--> Audio  FrameLength  = "+ Tools::IntegertoStringConvert(nCurrentFrameLenWithMediaHeader));

    }
}

