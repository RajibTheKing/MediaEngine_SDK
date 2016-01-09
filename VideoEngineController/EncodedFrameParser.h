#ifndef __ENCODED_FRAME_PARSER_H_
#define __ENCODED_FRAME_PARSER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "AudioVideoEngineDefinitions.h"
#include "EncodingBuffer.h"
#include "SendingBuffer.h"
#include "Size.h"
#include "Tools.h"

namespace IPV
{
	class thread;
}

class CCommonElementsBucket;

class CEncodedFrameParser
{

public:

	CEncodedFrameParser(CCommonElementsBucket* sharedObject);
	~CEncodedFrameParser();

	int ParseFrameIntoPackets(LongLong lFriendID, unsigned char *in_data, unsigned int in_size, int frameNumber,
							  unsigned int iTimeStampDiff);
	void StartEncodedFrameParsingThread();
	void StopEncodedFrameParsingThread();

	static void *CreateEncodedFrameParsingThread(void* param);
    
    void StartSendingThread();//
    void StopSendingThread();//
    void SendingThreadProcedure();//
    static void *CreateVideoSendingThread(void* param);//

private:
	Tools m_Tools;
	int m_PacketSize;
    
    CSendingBuffer m_SendingBuffer;//
    
    unsigned char m_EncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];//

	CCommonElementsBucket* m_pCommonElementsBucket;
    
    bool bSendingThreadRunning;//
    bool bSendingThreadClosed;//

	unsigned char m_Packet[MAX_VIDEO_PACKET_SIZE];

protected:

	std::thread* m_pEncodedFrameParsingThread;
    
    SmartPointer<std::thread> pSendingThread;//

	SmartPointer<CLockHandler> m_pEncodedFrameParsingMutex;

};

#endif