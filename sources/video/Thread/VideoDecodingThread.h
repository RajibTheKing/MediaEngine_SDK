
#ifndef IPV_VIDEO_DECODING_THREAD_H
#define IPV_VIDEO_DECODING_THREAD_H

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "EncodedFrameDepacketizer.h"
#include "RenderingBuffer.h"
#include "VideoDecoder.h"
#include "ColorConverter.h"
#include "FPSController.h"
#include "AverageCalculator.h"
#include "LiveVideoDecodingQueue.h"
#include "VideoHeader.h"

#include "VideoEffects.h"
#include <stdio.h>

//#include "Helper_IOS.hpp"
#include <thread>

namespace MediaSDK
{

	class CVideoCallSession;
	class CCommonElementsBucket;

	class CVideoDecodingThread
	{

	public:

		CVideoDecodingThread(CEncodedFrameDepacketizer *encodedFrameDepacketizer,
			long long llFriendID,
			CCommonElementsBucket *pCommonElementBucket,
			CRenderingBuffer *renderingBuffer,
			LiveVideoDecodingQueue *pLiveVideoDecodingQueue,
			CVideoDecoder *videoDecoder,
			CColorConverter *colorConverter,
			CVideoCallSession* pVideoCallSession,
			bool bIsCheckCall,
			int nFPS
			);

		~CVideoDecodingThread();
		void Reset();
		void StartDecodingThread();
		void StopDecodingThread();
		void DecodingThreadProcedure();
		static void *CreateDecodingThread(void* param);

		void ResetForPublisherCallerCallEnd();
		void ResetForViewerCallerCallStartEnd();

		void InstructionToStop();

		void SetCallFPS(int nFPS);

		int DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, long long nFramNumber, long long nTimeStampDiff, int nOrientation, int nInsetHeight, int nInsetWidth, int libraryVersion);
		int DecodeAndSendToClient2();

	private:

		//FILE * pFile;


		CVideoCallSession* m_pVideoCallSession;
		bool bDecodingThreadRunning;
		bool bDecodingThreadClosed;

		int m_naInsetHeights[3];
		int m_naInsetWidths[3];

		CCommonElementsBucket *m_pCommonElementBucket;
		long long m_llFriendID;

		int m_decodingHeight;
		int m_decodingWidth;
		int m_nPreviousDecodingHeight;
		int m_nPreviousDecodingWidth;
		int m_decodedFrameSize;

		int m_previousDecodedFrameSize;
		int m_PreviousDecodingHeight;
		int m_nPreviousInsetHeight;
		int m_nPreviousInsetWidth;
		int m_PreviousDecodingWidth;
		long long m_PreviousFrameNumber;
		int m_PreviousOrientation;

		int m_DecodeFailCounter;
		int m_NoFrameCounter;

		bool m_HasPreviousValues;

		int m_Counter;
		int m_nUpperOffsetOfInset;

		int m_nCallFPS;

		CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;
		CRenderingBuffer *m_RenderingBuffer;
		CVideoDecoder *m_pVideoDecoder;
		CColorConverter *m_pColorConverter;

		bool m_bResetForPublisherCallerCallEnd;
		bool m_bResetForViewerCallerCallStartEnd;

		bool m_bIsCheckCall;

		unsigned char m_PreviousDecodedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
		unsigned char m_PreviousDecodedFrameConvertedData[MAX_VIDEO_DECODER_FRAME_SIZE];

		unsigned char m_DecodedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
        unsigned char m_RotatedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

		unsigned char m_ucaOpponentSmallFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

		unsigned char m_ucaTempFrame2[MAX_VIDEO_ENCODER_FRAME_SIZE];

#if defined(TARGET_OS_WINDOWS_PHONE)

		unsigned char m_TempDecodedFrame[ULTRA_MAX_VIDEO_DECODER_FRAME_SIZE];

#endif

		unsigned char m_ucaTempTempFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
		unsigned char m_CropedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
		unsigned char m_PacketizedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];
		unsigned char m_RenderingRGBFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

		Tools m_Tools;
		CAverageCalculator *m_pCalculatorDecodeTime;
		CVideoBeautificationer *m_VideoBeautificationer;
		SharedPointer<std::thread> pDecodingThread;



		double m_dbAverageDecodingTime = 0, m_dbTotalDecodingTime = 0;
		int m_nOponnentFPS, m_nMaxProcessableByMine;
		int m_iDecodedFrameCounter = 0;
		long long m_nMaxDecodingTime = 0;


		int m_FpsCounter;
		long long m_FPS_TimeDiff;
		long long llQueuePrevTime;
		LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
		CVideoEffects *m_pVideoEffect;
		int m_iMaxLen;
		//int m_iEffectSelection;
		//int m_iNumberOfEffect;
		//int m_iNumberOfEffectedFrame;



	};

} //namespace MediaSDK

#endif 
