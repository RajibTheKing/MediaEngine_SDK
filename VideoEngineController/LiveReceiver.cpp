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

void LiveReceiver::PushAudioData(unsigned char* uchAudioData, int iLen, int numberOfFrames, int *frameSizes){
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

void LiveReceiver::PushVideoData(unsigned char* uchVideoData,int iLen){
    Locker lock(*m_pLiveReceiverMutex);
    int iUsedLen = 0, nFrames = 0;
    CPacketHeader packetHeaderObj;
    
    while(iUsedLen < iLen)
    {
        nFrames++;
//        packetHeaderObj.setPacketHeader(uchVideoData + iUsedLen);
        
        int nCurrentFrameLen = ((int)uchVideoData[1+iUsedLen+13] << 8) + uchVideoData[1+iUsedLen+14];
        
        printf("THeKing--> Video FrameCounter = %d, FrameLength  = %d, iLen = %d\n", nFrames, nCurrentFrameLen, iLen);
        
        m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen+1, nCurrentFrameLen + PACKET_HEADER_LENGTH);
        iUsedLen += nCurrentFrameLen + PACKET_HEADER_LENGTH + 1;
		//iUsedLen += LIVE_STREAMING_PACKETIZATION_PACKET_SIZE * ((nCurrentFrameLen + PACKET_HEADER_LENGTH + 1 + LIVE_STREAMING_PACKETIZATION_PACKET_SIZE - 1) / LIVE_STREAMING_PACKETIZATION_PACKET_SIZE);
    }

//    m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen, iLen + PACKET_HEADER_LENGTH);
}

bool LiveReceiver::GetVideoFrame(unsigned char* uchVideoFrame,int iLen)
{
    Locker lock(*m_pLiveReceiverMutex);
    return false;
}
