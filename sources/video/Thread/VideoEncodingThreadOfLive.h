
#ifndef IPV_VIDEO_ENCODING_THREAD_OF_LIVE_H
#define IPV_VIDEO_ENCODING_THREAD_OF_LIVE_H

#include "Tools.h"
#include "SmartPointer.h"
#include "EncodingBuffer.h"
#include "BitRateController.h"
#include "IDRFrameIntervalController.h"
#include "ColorConverter.h"
#include "EncodedFrameDepacketizer.h"
#include "EncodedFramePacketizer.h"
#include "AverageCalculator.h"
#include "VideoBeautificationer.h"
#include "VideoEffects.h"

#include <thread>

namespace MediaSDK
{

	class CVideoCallSession;
	class CCommonElementsBucket;

	class CVideoEncodingThreadOfLive
	{

	public:

		CVideoEncodingThreadOfLive(long long llFriendID, CEncodingBuffer *pEncodingBuffer, CCommonElementsBucket *commonElementsBucket, BitRateController *pBitRateController, IDRFrameIntervalController *pIdrFrameController, CColorConverter *pColorConverter, CVideoEncoder *pVideoEncoder, CEncodedFramePacketizer *pEncodedFramePacketizer, CVideoCallSession *pVideoCallSession, int nFPS, bool bIsCheckCall, bool bSelfViewOnly);
		~CVideoEncodingThreadOfLive();

		void StartEncodingThread();
		void StopEncodingThread();
		void EncodingThreadProcedure();
		static void *CreateVideoEncodingThread(void* param);

		void ResetForViewerCallerCallEnd();
		void ResetForPublisherCallerInAudioOnly();

		void SetOrientationType(int nOrientationType);
		void ResetVideoEncodingThread(BitRateController *pBitRateController);

		void SetCallFPS(int nFPS);

		bool IsThreadStarted();

		void SetNotifierFlag(bool flag);

		void SetFrameNumber(int nFrameNumber);

		int SetVideoEffect(int nEffectStatus);

		void MakeBlackScreen(unsigned char *pData, int iHeight, int iWidth, int colorFormat);
		void TestVideoEffect(int *param, int size);
		

		CEncodingBuffer *m_pEncodingBuffer;

	private:

		CVideoCallSession *m_pVideoCallSession;

		BitRateController *m_pBitRateController;
		IDRFrameIntervalController *m_pIdrFrameIntervalController;
		CColorConverter *m_pColorConverter;
		CVideoEncoder *m_pVideoEncoder;
		CEncodedFramePacketizer *m_pEncodedFramePacketizer;

		bool m_bVideoEffectEnabled;

		bool m_bResetForViewerCallerCallEnd;
		bool m_ResetForPublisherCallerInAudioOnly;

		unsigned char m_ucaEncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
		unsigned char m_ucaConvertedEncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
		unsigned char m_ucaEncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

		unsigned char m_ucaMirroredFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
		unsigned char m_ucaCropedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

		unsigned char m_ucaDummmyFrame[3][MAX_VIDEO_ENCODER_FRAME_SIZE];

		unsigned char m_ucaDummmyStillFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

#if defined(DESKTOP_C_SHARP)
		unsigned char m_RenderingRGBFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
		unsigned char m_pSmallFrame[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 1];
#endif

		int m_iFrameNumber;
		long long m_llFriendID;
		int m_nOrientationType;
		bool bEncodingThreadRunning;
		bool bEncodingThreadClosed;
		bool m_bNotifyToClientVideoQuality;

		CCommonElementsBucket *m_pCommonElementBucket;

		bool m_bSelfViewOnly;

		bool m_bIsThisThreadStarted;

		int m_nCallFPS;

		int mt_nTotalEncodingTimePerFrameRate;
		int mt_nCheckSlot;

		Tools m_Tools;

		CAverageCalculator *m_pCalculatorEncodeTime;
		CAverageCalculator *m_pCalculateEncodingTimeDiff;
		CVideoBeautificationer *m_VideoBeautificationer;
		CVideoEffects *m_VideoEffects;

		long long m_FPS_TimeDiff;
		int m_FpsCounter;
		bool m_bIsCheckCall;

		int m_filterToApply;

		SharedPointer<std::thread> pEncodingThread;
	};

} //namespace MediaSDK

#endif 
