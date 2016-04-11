
#ifndef _VIDEO_CALL_SESSION_H_
#define _VIDEO_CALL_SESSION_H_

#include "Size.h"
#include "VideoEncoder.h"
#include "VideoDecoder.h"
#include "VideoEncoderListHandler.h"
#include "LockHandler.h"
#include "ColorConverter.h"
#include "EncodingBuffer.h"
#include "RenderingBuffer.h"
#include "EncodedFrameDepacketizer.h"
#include "Tools.h"
#include "BitRateController.h"
#include "VideoEncodingThread.h"
#include "RenderingThread.h"
#include "VideoDecodingThread.h"
#include "DepacketizationThread.h"
#include "SendingThread.h"

using namespace std;

class CCommonElementsBucket;
class CVideoEncoder;

class CVideoCallSession
{

public:

	CVideoCallSession(LongLong fname, CCommonElementsBucket* sharedObject);
	~CVideoCallSession();

	LongLong GetFriendID();
	void InitializeVideoSession(LongLong lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType);
	CVideoEncoder* GetVideoEncoder();
	int PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size);
	CVideoDecoder* GetVideoDecoder();
	CColorConverter* GetColorConverter();

	bool PushPacketForMerging(unsigned char *in_data, unsigned int in_size);
	CEncodedFrameDepacketizer * GetEncodedFrameDepacketizer();

	void PushFrameForDecoding(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int timeStampDiff);

	void CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber);

	CSendingThread *m_pSendingThread;
	CVideoEncodingThread *m_pVideoEncodingThread;

	CVideoRenderingThread *m_pVideoRenderingThread;
	CVideoDecodingThread *m_pVideoDecodingThread;
	CVideoDepacketizationThread *m_pVideoDepacketizationThread;
	long long GetFirstVideoPacketTime();
	void SetFirstVideoPacketTime(long long llTimeStamp);

	void SetFirstFrameEncodingTime(int time);
	int GetFirstFrameEncodingTime();
	void SetShiftedTime(long long llTime);
	long long GetShiftedTime();

private:

	LongLong m_LastTimeStampClientFPS;
	double m_ClientFPSDiffSum;
	int m_ClientFrameCounter;
	double m_ClientFPS;
	int m_EncodingFrameCounter;
	bool m_bSkipFirstByteCalculation;

	long long m_llShiftedTime;
	long long m_llTimeStampOfFirstPacketRcvd;
	int m_nFirstFrameEncodingTimeDiff;
	int m_ByteRcvInBandSlot;
	long long m_llFirstFrameCapturingTimeStamp;

	unsigned int m_miniPacketBandCounter;

	int m_SlotResetLeftRange;
	int m_SlotResetRightRange;

	Tools m_Tools;
	LongLong m_lfriendID;
	CVideoEncoderListHandler sessionMediaList;

	CPacketHeader m_PacketHeader;

	CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;
	CEncodedFramePacketizer *m_pEncodedFramePacketizer;
	CCommonElementsBucket* m_pCommonElementsBucket;
	CVideoEncoder *m_pVideoEncoder;
	CVideoDecoder *m_pVideoDecoder;

	BitRateController *m_BitRateController;

	CEncodingBuffer *m_EncodingBuffer;
	CVideoPacketQueue *m_pVideoPacketQueue;
	CVideoPacketQueue *m_pRetransVideoPacketQueue;
	CVideoPacketQueue *m_pMiniPacketQueue;
	CRenderingBuffer *m_RenderingBuffer;
	CSendingBuffer *m_SendingBuffer;

	CColorConverter *m_pColorConverter;

	unsigned char m_miniPacket[PACKET_HEADER_LENGTH_NO_VERSION + 1];

protected:

	SmartPointer<CLockHandler> m_pVideoCallSessionMutex;
};


#endif