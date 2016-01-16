#ifndef _VIDEO_CALL_SESSION_H_
#define _VIDEO_CALL_SESSION_H_

#include <stdio.h>
#include <string>
#include <map>
#include "Size.h"

#include "VideoEncoder.h"
#include "VideoDecoder.h"
#include "VideoEncoderListHandler.h"
#include "LockHandler.h"
#include "ColorConverter.h"
#include "DecodingBuffer.h"
#include "EncodingBuffer.h"
#include "RenderingBuffer.h"
#include "EncodedFrameDepacketizer.h"
#include "VideoPacketQueue.h"
#include "Tools.h"
#include "PairMap.h"
#include "RetransmitVideoPacketQueue.h"

#include <queue>
#include <utility>

using namespace std;

extern PairMap g_timeInt;


class CCommonElementsBucket;
class CVideoEncoder;

class CVideoCallSession
{

public:

	CVideoCallSession(LongLong fname, CCommonElementsBucket* sharedObject);
	~CVideoCallSession();

	LongLong GetFriendID();
	void InitializeVideoSession(LongLong lFriendID,int iVideoHeight, int iVideoWidth);
	CVideoEncoder* GetVideoEncoder();
	int PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size);
	int DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize,int nFramNumber, unsigned int nTimeStampDiff);
	CVideoDecoder* GetVideoDecoder();
	CColorConverter* GetColorConverter();

	bool PushPacketForMerging(unsigned char *in_data, unsigned int in_size);
	CEncodedFrameDepacketizer * GetEncodedFrameDepacketizer();

	void StartEncodingThread();
	void StopEncodingThread();
	void EncodingThreadProcedure();
	static void *CreateVideoEncodingThread(void* param);

	void StartDepacketizationThread();
	void StopDepacketizationThread();
	void DepacketizationThreadProcedure();
	static void *CreateVideoDepacketizationThread(void* param);

	void StartDecodingThread();
	void StopDecodingThread();
	void DecodingThreadProcedure();
	static void *CreateDecodingThread(void* param);

	void StartRenderingThread();
	void StopRenderingThread();
	void RenderingThreadProcedure();
	static void *CreateVideoRenderingThread(void* param);

	void PushFrameForDecoding(unsigned char *in_data, unsigned int frameSize,int nFramNumber, unsigned int timeStampDiff);
    
    void CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber);

	int orientation_type;
	int ownFPS;
	LongLong m_LastTimeStampClientFPS;
	double m_ClientFPSDiffSum;
	int m_ClientFrameCounter;
	double m_ClientFPS;
	double m_DropSum;
	int opponentFPS;
	int fpsCnt;
	int m_EncodingFrameCounter;

//	void increaseFPS();
//	void decreaseFPS();
//	bool isProcessable();

private:
	int m_iCountRecResPack;
	int m_iCountReQResPack;

	long long m_ll1stFrameTimeStamp;
	bool m_bFirstFrame;
	unsigned  int m_iTimeStampDiff;
	bool m_b1stDecodedFrame;
	long long m_ll1stDecodedFrameTimeStamp;


	Tools m_Tools;
	Tools m_ToolsDepacketizationThreadProcedure;
	LongLong friendID;
	int m_iFrameNumber;
	CVideoEncoderListHandler sessionMediaList;

	CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;
	CEncodedFramePacketizer *m_pEncodedFramePacketizer;
	CCommonElementsBucket* m_pCommonElementsBucket;
	CVideoEncoder *m_pVideoEncoder;
	CVideoDecoder *m_pVideoDecoder;

	CEncodingBuffer m_EncodingBuffer;
	CDecodingBuffer m_DecodingBuffer;
	CVideoPacketQueue m_pVideoPacketQueue;
	CVideoPacketQueue m_pRetransVideoPacketQueue;
    CVideoPacketQueue m_pMiniPacketQueue;
	CRenderingBuffer m_RenderingBuffer;

	unsigned char m_EncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
	unsigned char m_ConvertedEncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
	unsigned char m_EncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

	int m_decodingFrameNumber;
	int m_decodedFrameSize;
	int m_decodingHeight;
	int m_decodingWidth;

	unsigned char m_PacketToBeMerged[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_DecodedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_PacketizedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
	unsigned char m_RenderingFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

	CColorConverter *m_pColorConverter;

	bool bEncodingThreadRunning;
	bool bEncodingThreadClosed;

	bool bDepacketizationThreadRunning;
	bool bDepacketizationThreadClosed;

	pair<int, int> ExpectedFramePacketPair;
	int iNumberOfPacketsInCurrentFrame;
	//pair<int, int> GetFramePacketFromHeader(unsigned char * packet, int &iNumberOfPackets);
	void UpdateExpectedFramePacketPair(pair<int, int> currentFramePacketPair, int iNumberOfPackets);

	bool bDecodingThreadRunning;
	bool bDecodingThreadClosed;

	bool bRenderingThreadRunning;
	bool bRenderingThreadClosed;
    
    unsigned char m_miniPacket[PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE];

protected:

	SmartPointer<std::thread> pEncodingThread;

	SmartPointer<std::thread> pDecodingThread;

	SmartPointer<std::thread> pDepacketizationThread;

	SmartPointer<std::thread> pRenderingThread;

	SmartPointer<CLockHandler> m_pSessionMutex;
};


#endif