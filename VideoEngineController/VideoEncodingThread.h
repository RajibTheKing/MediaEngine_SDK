
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
#include "../VideoEngineUtilities/VideoBeautificationer.h"

#include <thread>

class CVideoCallSession;
class CCommonElementsBucket;

class CVideoEncodingThread
{

public:

	CVideoEncodingThread(LongLong llFriendID, CEncodingBuffer *pEncodingBuffer, CCommonElementsBucket *commonElementsBucket, BitRateController *pBitRateController, CColorConverter *pColorConverter, CVideoEncoder *pVideoEncoder, CEncodedFramePacketizer *pEncodedFramePacketizer, CVideoCallSession *pVideoCallSession, int nFPS, bool bIsCheckCall);
	~CVideoEncodingThread();

	void StartEncodingThread();
	void StopEncodingThread();
	void EncodingThreadProcedure();
	static void *CreateVideoEncodingThread(void* param);

	void ResetForViewerCallerCallEnd();

	void SetOrientationType(int nOrientationType);
    void ResetVideoEncodingThread(BitRateController *pBitRateController);

	void SetCallFPS(int nFPS);

	bool IsThreadStarted();

	void SetNotifierFlag(bool flag);
    
    void SetFrameNumber(int nFrameNumber);

	CEncodingBuffer *m_pEncodingBuffer;

private:

	CVideoCallSession *m_pVideoCallSession;
							
	BitRateController *m_pBitRateController;	
	CColorConverter *m_pColorConverter;
	CVideoEncoder *m_pVideoEncoder;
	CEncodedFramePacketizer *m_pEncodedFramePacketizer;

	bool m_bResetForViewerCallerCallEnd;

	unsigned char m_ucaEncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
	unsigned char m_ucaConvertedEncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
	unsigned char m_ucaEncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

	unsigned char m_ucaMirroredFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
    unsigned char m_ucaCropedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

    unsigned char m_ucaDummmyFrame[3][MAX_VIDEO_ENCODER_FRAME_SIZE];

#if defined(_DESKTOP_C_SHARP_)
	unsigned char m_RenderingRGBFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_pSmallFrame[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 1];
#endif

	int m_iFrameNumber;
	LongLong m_llFriendID;		
	int m_nOrientationType;
	bool bEncodingThreadRunning;
	bool bEncodingThreadClosed;
	bool m_bNotifyToClientVideoQuality;

	CCommonElementsBucket *m_pCommonElementBucket;


	bool m_bIsThisThreadStarted;

	int m_nCallFPS;
    
    int mt_nTotalEncodingTimePerFrameRate;
    int mt_nCheckSlot;
    
	Tools m_Tools;
    
    CAverageCalculator *m_pCalculatorEncodeTime;
    CAverageCalculator *m_pCalculateEncodingTimeDiff;
	CVideoBeautificationer *m_VideoBeautificationer;
    
    long long m_FPS_TimeDiff;
    int m_FpsCounter;
    bool m_bIsCheckCall;
    
	SmartPointer<std::thread> pEncodingThread;
};

#endif 
