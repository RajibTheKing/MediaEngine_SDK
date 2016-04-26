
#ifndef _VIDEO_ENCODING_THREAD_H_
#define _VIDEO_ENCODING_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "EncodingBuffer.h"
#include "BitRateController.h"
#include "ColorConverter.h"
#include "EncodedFrameDepacketizer.h"
#include "EncodedFramePacketizer.h"
#include "AverageCalculator.h"

#include <thread>

class CVideoCallSession;

class CVideoEncodingThread
{

public:

	CVideoEncodingThread(LongLong llFriendID, CEncodingBuffer *pEncodingBuffer, BitRateController *pBitRateController, CColorConverter *pColorConverter, CVideoEncoder *pVideoEncoder, CEncodedFramePacketizer *pEncodedFramePacketizer, CVideoCallSession *pVideoCallSession , bool bIsCheckCall = false);
	~CVideoEncodingThread();

	void StartEncodingThread();
	void StopEncodingThread();
	void EncodingThreadProcedure();
	static void *CreateVideoEncodingThread(void* param);

	void SetOrientationType(int nOrientationType);
    void ResetVideoEncodingThread(BitRateController *pBitRateController);

	bool IsThreadStarted();

private:

	CEncodingBuffer *m_pEncodingBuffer;						
	BitRateController *m_pBitRateController;	
	CColorConverter *m_pColorConverter;
	CVideoEncoder *m_pVideoEncoder;
	CEncodedFramePacketizer *m_pEncodedFramePacketizer;

	unsigned char m_ucaEncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
	unsigned char m_ucaConvertedEncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
	unsigned char m_ucaEncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

	int m_iFrameNumber;
	LongLong m_llFriendID;		
	int m_nOrientationType;
	bool bEncodingThreadRunning;
	bool bEncodingThreadClosed;

	bool m_bIsThisThreadStarted;
    
    int mt_nTotalEncodingTimePerFrameRate;
    int mt_nCheckSlot;
    
	Tools m_Tools;
    
    CAverageCalculator m_CalculatorEncodeTime;
    CAverageCalculator m_TestingEncodeTime;
    CAverageCalculator m_CalculateEncodingTimeDiff;
    
    CVideoCallSession *m_pVideoCallSession;
    long long m_FPS_TimeDiff;
    int m_FpsCounter;
    bool m_bIsCheckCall;
    
	SmartPointer<std::thread> pEncodingThread;
};

#endif 
