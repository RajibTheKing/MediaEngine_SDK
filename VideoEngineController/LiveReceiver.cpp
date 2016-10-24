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

void LiveReceiver::PushAudioData(unsigned char* uchAudioData,int iLen){
    Locker lock(*m_pLiveReceiverMutex);
    int iUsedLen = 0, nFrames = 0;
    CAudioPacketHeader audioPacketHeaderObject;

    while(iUsedLen < iLen)
    {
        nFrames++;
        audioPacketHeaderObject.CopyHeaderToInformation(uchAudioData + iUsedLen);
        int nCurrentFrameLen = audioPacketHeaderObject.GetInformation(PACKETLENGTH);
        m_pAudioDecoderBuffer->Queue(uchAudioData + iUsedLen, nCurrentFrameLen + __AUDIO_HEADER_LENGTH__);
        iUsedLen += nCurrentFrameLen + __AUDIO_HEADER_LENGTH__;
    }
}

void LiveReceiver::PushVideoData(unsigned char* uchVideoData,int iLen){
    Locker lock(*m_pLiveReceiverMutex);
    int iUsedLen = 0, nFrames = 0;
    CPacketHeader packetHeaderObj;
    
    while(iUsedLen < iLen)
    {
        nFrames++;
        packetHeaderObj.setPacketHeader(uchVideoData + iUsedLen);

        int nCurrentFrameLen = packetHeaderObj.getPacketLength();
        m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen, nCurrentFrameLen + PACKET_HEADER_LENGTH);
        iUsedLen += nCurrentFrameLen + PACKET_HEADER_LENGTH;
    }

//    m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen, iLen + PACKET_HEADER_LENGTH);
}

bool LiveReceiver::GetVideoFrame(unsigned char* uchVideoFrame,int iLen){
    Locker lock(*m_pLiveReceiverMutex);
}