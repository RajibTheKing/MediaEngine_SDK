#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "Globals.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
	#include <dispatch/dispatch.h>
#endif
//#include <android/log.h>

//#define LOG_TAG "NewTest"
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//int FPS=0;
//int fpsCnt=0;

deque<pair<int,int>> ExpectedFramePacketDeQueue;
extern long long g_FriendID;
extern CFPSController g_FPSController;

#define ORIENTATION_0_MIRRORED 1
#define ORIENTATION_90_MIRRORED 2
#define ORIENTATION_180_MIRRORED 3
#define ORIENTATION_270_MIRRORED 4
#define ORIENTATION_0_NOT_MIRRORED 5
#define ORIENTATION_90_NOT_MIRRORED 6
#define ORIENTATION_180_NOT_MIRRORED 7
#define ORIENTATION_270_NOT_MIRRORED 8




//extern int g_MY_FPS;

CVideoCallSession::CVideoCallSession(LongLong fname, CCommonElementsBucket* sharedObject) :

		m_pCommonElementsBucket(sharedObject),
		m_iFrameNumber(0),
		m_ClientFPS(FPS_BEGINNING),
		m_ClientFPSDiffSum(0),
		m_ClientFrameCounter(0),
		m_EncodingFrameCounter(0),
		m_ll1stFrameTimeStamp(0),
		m_bFirstFrame(true),
		m_iTimeStampDiff(0),
		m_b1stDecodedFrame(true),
		m_ll1stDecodedFrameTimeStamp(0)
{
	fpsCnt=0;
	g_FPSController.Reset();
//	g_MY_FPS =
	opponentFPS=ownFPS=FPS_BEGINNING;
	m_iCountReQResPack = 0;

//	FPS=10;

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::CVideoCallSession");
	m_pSessionMutex.reset(new CLockHandler);
	friendID = fname;
	sessionMediaList.ClearAllFromVideoEncoderList();

	m_pEncodedFrameDepacketizer = NULL;
	m_pEncodedFrameDepacketizer = new CEncodedFrameDepacketizer(sharedObject,this);
	g_FriendID = fname;

	ExpectedFramePacketPair.first = 0;
	ExpectedFramePacketPair.second = 0;
	iNumberOfPacketsInCurrentFrame = 0;


	StartEncodingThread();
	StartDepacketizationThread();
	StartDecodingThread();


	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::CVideoCallSession created");
}

CVideoCallSession::~CVideoCallSession()
{
	StopDecodingThread();
	StopDepacketizationThread();
	StopEncodingThread();

	if(NULL!=m_pEncodedFrameDepacketizer)
		delete m_pEncodedFrameDepacketizer;

	if (NULL != m_pVideoDecoder)
	{
		delete m_pVideoDecoder;

		m_pVideoDecoder = NULL;
	}

	if (NULL != m_pColorConverter)
	{
		delete m_pColorConverter;

		m_pColorConverter = NULL;
	}

	friendID = -1;

	SHARED_PTR_DELETE(m_pSessionMutex);
}

LongLong CVideoCallSession::GetFriendID()
{
	return friendID;
}

void CVideoCallSession::InitializeVideoSession(LongLong lFriendID, int iVideoHeight, int iVideoWidth)
{
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession");

	if (sessionMediaList.IsVideoEncoderExist(iVideoHeight, iVideoWidth))
	{
		return;
	}

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession 2");

	this->m_pVideoEncoder = new CVideoEncoder(m_pCommonElementsBucket);

	m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth);

	this->m_pVideoDecoder = new CVideoDecoder(m_pCommonElementsBucket, &m_DecodingBuffer);

	m_pVideoDecoder->CreateVideoDecoder();

	this->m_pColorConverter = new CColorConverter(iVideoHeight, iVideoWidth);

	m_pCommonElementsBucket->m_pVideoEncoderList->AddToVideoEncoderList(lFriendID, m_pVideoEncoder);

	m_ClientFrameCounter = 0;
	m_EncodingFrameCounter = 0;

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession session initialized");
}

CVideoEncoder* CVideoCallSession::GetVideoEncoder()
{
	//	return sessionMediaList.GetFromVideoEncoderList(mediaName);

	return m_pVideoEncoder;
}

bool CVideoCallSession::PushPacketForMerging(unsigned char *in_data, unsigned int in_size)
{
#ifdef	RETRANSMISSION_ENABLED
	if(((in_data[4] >> 7) & 1) || in_size == PACKET_HEADER_LENGTH)
		m_pRetransVideoPacketQueue.Queue(in_data,in_size);
	else
#endif
	{
		m_pVideoPacketQueue.Queue(in_data, in_size);
	}

	return true;
}

int CVideoCallSession::PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size)
{
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding");

	LongLong currentTimeStamp = m_Tools.CurrentTimestamp();

	if(m_ClientFrameCounter++)
	{
		m_ClientFPSDiffSum += currentTimeStamp - m_LastTimeStampClientFPS;


		{//Block for LOCK
			Locker lock(*m_pSessionMutex);
			g_FPSController.SetClientFPS(1000 / (m_ClientFPSDiffSum / m_ClientFrameCounter));
//			m_ClientFPS = 1000 / (m_ClientFPSDiffSum / m_ClientFrameCounter);
	//		m_ClientFPS = 1000/(currentTimeStamp - m_LastTimeStampClientFPS);
		}

		m_DropSum = 0;
	}

	m_LastTimeStampClientFPS = currentTimeStamp;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding 2");
	//this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding Converted to 420");

#endif

	int returnedValue = m_EncodingBuffer.Queue(in_data, in_size);

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding pushed to encoder queue");

	return returnedValue;
}

CVideoDecoder* CVideoCallSession::GetVideoDecoder()
{
	//	return sessionMediaList.GetFromVideoEncoderList(mediaName);

	return m_pVideoDecoder;
}

CColorConverter* CVideoCallSession::GetColorConverter()
{
	return m_pColorConverter;
}

/*
void CVideoCallSession::ResetAllInMediaList()
{
sessionMediaList.ResetAllInVideoEncoderList();
}
*/

void CVideoCallSession::StopEncodingThread()
{
	//if (pInternalThread.get())
	{
		bEncodingThreadRunning = false;

		while (!bEncodingThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pInternalThread.reset();
}

void CVideoCallSession::StartEncodingThread()
{
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 1");

	if (pEncodingThread.get())
	{
		CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 2");
		pEncodingThread.reset();
		CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 3");
		return;
	}
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 4");
	bEncodingThreadRunning = true;
	bEncodingThreadClosed = false;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 5");
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    
    dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(EncodeThreadQ, ^{
        this->EncodingThreadProcedure();
    });

#else
    
	std::thread myThread(CreateVideoEncodingThread, this);
	myThread.detach();
    
#endif
    
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread Encoding Thread started");

	return;
}

void *CVideoCallSession::CreateVideoEncodingThread(void* param)
{
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->EncodingThreadProcedure();

	return NULL;
}

void CVideoCallSession::EncodingThreadProcedure()
{
	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
	Tools toolsObject;
	int frameSize, encodedFrameSize;

	while (bEncodingThreadRunning)
	{
		//CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::InternalThreadImpl");

		if (m_EncodingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			frameSize = m_EncodingBuffer.DeQueue(m_EncodingFrame);
//			CLogPrinter::WriteSpecific(CLogPrinter::INFO, "Before Processable");
			if(!g_FPSController.IsProcessableFrame())
			{
				toolsObject.SOSleep(10);
				continue;
			}
			if(m_bFirstFrame)
			{
				m_ll1stFrameTimeStamp = toolsObject.CurrentTimestamp();
				m_bFirstFrame = false;
			}

			m_iTimeStampDiff = toolsObject.CurrentTimestamp() - m_ll1stFrameTimeStamp;


//			CLogPrinter::WriteSpecific(CLogPrinter::INFO, "$ENCODEING$");
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
            
            this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);
			
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_EncodingFrame, frameSize, m_EncodedFrame);

			CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure video data encoded");

#else

			CLogPrinter::Write(CLogPrinter::DEBUGS, "orientation_type : "+  m_Tools.IntegertoStringConvert(orientation_type));

			if(orientation_type == ORIENTATION_90_MIRRORED)
			{
				CLogPrinter::Write(CLogPrinter::DEBUGS, "orientation_type : "+  m_Tools.IntegertoStringConvert(orientation_type)+ "  ORIENTATION_90_MIRRORED");
				this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			else if(orientation_type == ORIENTATION_0_MIRRORED)
			{
				CLogPrinter::Write(CLogPrinter::DEBUGS, "orientation_type : "+  m_Tools.IntegertoStringConvert(orientation_type) + " ORIENTATION_0_MIRRORED ");
				this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420ForBackCam(m_EncodingFrame, m_ConvertedEncodingFrame);
			}

			CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure Converted to 420");

			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);

			CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure video data encoded");

#endif

//			CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CVideoCallSession::EncodingThreadProcedure m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));
//			CLogPrinter::WriteSpecific(CLogPrinter::INFO, "$ENCODEING$ To Parser");
			m_pVideoEncoder->GetEncodedFramePacketizer()->Packetize(friendID,m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
			++m_iFrameNumber;
			//CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CVideoCallSession::EncodingThreadProcedure2 m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(CVideoCallSession::m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));

			toolsObject.SOSleep(1);

		}
	}

	bEncodingThreadClosed = true;

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}

void CVideoCallSession::StopDepacketizationThread()
{
	//if (pDepacketizationThread.get())
	{
		bDepacketizationThreadRunning = false;

		while (!bDepacketizationThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pDepacketizationThread.reset();
}

void CVideoCallSession::StartDepacketizationThread()
{
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 1");
	if (pDepacketizationThread.get())
	{
//		CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 2");
		pDepacketizationThread.reset();
//		CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 3");
		return;
	}
//	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 4");
	bDepacketizationThreadRunning = true;
	bDepacketizationThreadClosed = false;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 5");
 
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    
    dispatch_queue_t DecodeThreadQ = dispatch_queue_create("DecodeThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(DecodeThreadQ, ^{
        this->DepacketizationThreadProcedure();
    });
    
#else
    
	std::thread myThread(CreateVideoDepacketizationThread, this);
	myThread.detach();

#endif

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread Decoding Thread started");

	return;
}

void *CVideoCallSession::CreateVideoDepacketizationThread(void* param)
{
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->DepacketizationThreadProcedure();

	return NULL;
}


void CVideoCallSession::PushFrameForDecoding(unsigned char *in_data, unsigned int nFrameSize,int nFramNumber, unsigned int timeStampDiff)
{
	m_DecodingBuffer.Queue(nFramNumber, in_data, nFrameSize, timeStampDiff);
}


int CVideoCallSession::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize,int nFramNumber,
											 unsigned int nTimeStampDiff)
{
	m_decodedFrameSize = m_pVideoDecoder->Decode(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	this->m_pColorConverter->ConvertI420ToNV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#else

	this->m_pColorConverter->ConvertI420ToNV21(m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#endif
	if(m_decodedFrameSize>0)
	{
		if(m_b1stDecodedFrame)
		{
			m_ll1stDecodedFrameTimeStamp = m_Tools.CurrentTimestamp();
			m_b1stDecodedFrame = false;
		}
		int DecodingDelay = m_Tools.CurrentTimestamp() + nTimeStampDiff - m_ll1stDecodedFrameTimeStamp;
		CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() n timeStampDiff: "+m_Tools.IntegertoStringConvert(nTimeStampDiff)+ " ::DecodingDelay: "+ m_Tools.IntegertoStringConvert(DecodingDelay));
		m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(friendID, nFramNumber, m_decodedFrameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);
	}
	return 1;
}

void CVideoCallSession::DepacketizationThreadProcedure()		//Merging Thread
{
	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Started DepacketizationThreadProcedure method.");
	Tools toolsObject;
	unsigned char temp;
	int frameSize,queSize=0,retQueuSize=0,consicutiveRetransmittedPkt=0;
	int frameNumber,packetNumber;
	m_iCountRecResPack = 0;

	while (bDepacketizationThreadRunning)
	{
		//CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::DepacketizationThreadProcedure");
		queSize = m_pVideoPacketQueue.GetQueueSize();
#ifdef	RETRANSMISSION_ENABLED
		retQueuSize = m_pRetransVideoPacketQueue.GetQueueSize();
		CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "SIZE "+ m_Tools.IntegertoStringConvert(retQueuSize)+"  "+ m_Tools.IntegertoStringConvert(queSize));
#endif
		if (0 == queSize && 0 == retQueuSize)
			toolsObject.SOSleep(10);
		else
		{
#ifdef	RETRANSMISSION_ENABLED
			if(retQueuSize>0 && consicutiveRetransmittedPkt<2)
			{
			//	CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "RT QueueSize"+ m_Tools.IntegertoStringConvert(retQueuSize));
				frameSize = m_pRetransVideoPacketQueue.DeQueue(m_PacketToBeMerged);
				++consicutiveRetransmittedPkt;
			}
			else if(queSize>0){
#endif
				frameSize = m_pVideoPacketQueue.DeQueue(m_PacketToBeMerged);

#ifdef	RETRANSMISSION_ENABLED
				consicutiveRetransmittedPkt = 0;
			}
			else
			{
				frameSize = m_pRetransVideoPacketQueue.DeQueue(m_PacketToBeMerged);
				++consicutiveRetransmittedPkt;
			}


			bool bRetransmitted = (m_PacketToBeMerged[4] >> 7) & 1;
			m_PacketToBeMerged[4] &= ~(1<<7); //Removed the Retransmit flag from the LMB of packet size

			if(!bRetransmitted && frameSize>PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE)
			{
				int iNumberOfPackets = -1;
				temp = m_PacketToBeMerged[SIGNAL_BYTE_INDEX];
				m_PacketToBeMerged[SIGNAL_BYTE_INDEX]=0;
				pair<int, int> currentFramePacketPair = m_Tools.GetFramePacketFromHeader(m_PacketToBeMerged , iNumberOfPackets);
				m_PacketToBeMerged[SIGNAL_BYTE_INDEX]=temp;

//				CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::currentFramePacketPair: FrameNumber: "+
//						m_Tools.IntegertoStringConvert(currentFramePacketPair.first) + " PacketNo : "+  m_Tools.IntegertoStringConvert(currentFramePacketPair.second)+
//						" NumberOfPacket : "+  m_Tools.IntegertoStringConvert(iNumberOfPackets));
				if (currentFramePacketPair != ExpectedFramePacketPair && !m_pVideoPacketQueue.PacketExists(ExpectedFramePacketPair.first, ExpectedFramePacketPair.second) && ExpectedFramePacketPair.first % ENCODER_KEY_FRAME_RATE < MAX_BLOCK_RETRANSMISSION)
				{
                    /*
					while(ExpectedFramePacketDeQueue.size() > EXPECTED_FRAME_PACKET_QUEUE_SIZE)
					{
						ExpectedFramePacketDeQueue.pop_back();
					}
                     
					CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "RETRANSMISSION # "+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first) + "~"+  m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second) + "  MOD: "+  m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first%ENCODER_KEY_FRAME_RATE));
					ExpectedFramePacketDeQueue.push_back(ExpectedFramePacketPair);//send minipacket
                    */
                    
                    CreateAndSendMiniPacket(ExpectedFramePacketPair.first, ExpectedFramePacketPair.second);
                    
					g_timeInt.setTime(ExpectedFramePacketPair.first,ExpectedFramePacketPair.second);
                    
                    
				}
				UpdateExpectedFramePacketPair(currentFramePacketPair, iNumberOfPackets);
//				CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::ExpFramePacketPair: ExpFrameNumber: "+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first) + " ExpPacketNo. : "+  m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second)+ " iNumberOfPacketsInCurrentFrame : "+  m_Tools.IntegertoStringConvert(iNumberOfPacketsInCurrentFrame));
			}
			else
			{
				int iNumberOfPackets = -1;
				pair<int, int> currentFramePacketPair = m_Tools.GetFramePacketFromHeader(m_PacketToBeMerged , iNumberOfPackets);
//				CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::currentFramePacketPair: ###########Retransmitted FrameNumber: "+
//																m_Tools.IntegertoStringConvert(currentFramePacketPair.first) + " PacketNo : "+  m_Tools.IntegertoStringConvert(currentFramePacketPair.second)+
//																" NumberOfPacket : "+  m_Tools.IntegertoStringConvert(iNumberOfPackets)  + " m_iCountReQResPack : "+  m_Tools.IntegertoStringConvert(++m_iCountReQResPack));


				m_PacketToBeMerged[SIGNAL_BYTE_INDEX]|=(1<<4);
			}


//			CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::reqrec: m_iCountRecResPack  "+
//															m_Tools.IntegertoStringConvert(m_iCountRecResPack) + " m_iCountReQResPack : "+  m_Tools.IntegertoStringConvert(m_iCountReQResPack));
#endif
			m_pEncodedFrameDepacketizer->Depacketize(m_PacketToBeMerged,frameSize);

			toolsObject.SOSleep(1);
		}
	}

	bDepacketizationThreadClosed = true;

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Stopped DepacketizationThreadProcedure method.");
}


void CVideoCallSession::StopDecodingThread()
{
	//if (pDepacketizationThread.get())
	{
		bDecodingThreadRunning = false;

		while (!bDecodingThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pDepacketizationThread.reset();
}

void CVideoCallSession::StartDecodingThread()
{
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 1");

	if (pDecodingThread.get())
	{
//		CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 2");
		pDecodingThread.reset();
//		CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 3");
		return;
	}

//	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 4");

	bDecodingThreadRunning = true;
	bDecodingThreadClosed = false;

//	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 5");

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t PacketizationThreadQ = dispatch_queue_create("PacketizationThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(PacketizationThreadQ, ^{
        this->DecodingThreadProcedure();
    });

#else

	std::thread myThread(CreateDecodingThread, this);
	myThread.detach();

#endif

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread Decoding Thread started");

	return;
}

void *CVideoCallSession::CreateDecodingThread(void* param)
{
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->DecodingThreadProcedure();

	return NULL;
}

void CVideoCallSession::DecodingThreadProcedure()
{
	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Started DepacketizationThreadProcedure method.");
	Tools toolsObject;
	int frameSize,nFrameNumber,intervalTime;
	unsigned int nTimeStampDiff;
	long long firstTime,decodingTime;

	while (bDecodingThreadRunning)
	{
		//CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::DepacketizationThreadProcedure");

		if (m_DecodingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(5);
		else
		{
			firstTime = toolsObject.CurrentTimestamp();
			frameSize = m_DecodingBuffer.DeQueue(nFrameNumber, nTimeStampDiff, m_PacketizedFrame);

			DecodeAndSendToClient(m_PacketizedFrame,frameSize,nFrameNumber, nTimeStampDiff);
			intervalTime = 1000/opponentFPS;
			decodingTime = toolsObject.CurrentTimestamp() - firstTime;

			if(intervalTime > decodingTime+5)
				toolsObject.SOSleep(intervalTime-decodingTime-5);
			else
				toolsObject.SOSleep(1);
		}
	}

	bDecodingThreadClosed = true;

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Stopped DepacketizationThreadProcedure method.");
}

CEncodedFrameDepacketizer * CVideoCallSession::GetEncodedFrameDepacketizer()
{
	return m_pEncodedFrameDepacketizer;
}

void CVideoCallSession::UpdateExpectedFramePacketPair(pair<int,int> currentFramePacketPair, int iNumberOfPackets)
{
	int iFrameNumber = currentFramePacketPair.first;
	int iPackeNumber = currentFramePacketPair.second;
	if(iPackeNumber == iNumberOfPackets - 1)//Last Packet In a Frame
	{
		iNumberOfPacketsInCurrentFrame = 0;
		ExpectedFramePacketPair.first = iFrameNumber + 1;
		ExpectedFramePacketPair.second = 0;
	}
	else
	{
		iNumberOfPacketsInCurrentFrame = iNumberOfPackets;
		ExpectedFramePacketPair.first = iFrameNumber;
		ExpectedFramePacketPair.second = iPackeNumber + 1;
	}

	//CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CController::UpdateExpectedFramePacketPair: ExFrameNumber: "+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first) + " ExPacketNo. : "+  m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second)+ " ExNumberOfPacket : "+  m_Tools.IntegertoStringConvert(iNumberOfPacketsInCurrentFrame));
}

void CVideoCallSession::CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber)
{
    CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::CreateAndSendMiniPacket() resendFrameNumber = " + m_Tools.IntegertoStringConvert(resendFrameNumber) +
                                            ", resendPacketNumber = " + m_Tools.IntegertoStringConvert(resendPacketNumber));
    int startFraction = SIZE_OF_INT_MINUS_8;
    int fractionInterval = BYTE_SIZE;
    int startPoint = 1;
    
    
    int frameNumber = 555; //dummy FrameNumber
    int numberOfPackets = 17; //dummy numberOfPackets
    int packetNumber = 5; //dummy PacketNumber
    int PacketSize = 24; //dummyPacketSize
    //int resendFrameNumber = 7777; //Dummy ResendFramenumber
    //int resendPacketNumber = 7; //Dummy ResendPacketNumber
    
    
    for (int f = startFraction; f >= 0; f -= fractionInterval)
    {
        m_miniPacket[startPoint++] = (frameNumber >> f) & 0xFF;
    }
    
    for (int f = startFraction; f >= 0; f -= fractionInterval)
    {
        m_miniPacket[startPoint++] = (numberOfPackets >> f) & 0xFF;
    }
    
    for (int f = startFraction; f >= 0; f -= fractionInterval)
    {
        m_miniPacket[startPoint++] = (packetNumber >> f) & 0xFF;
    }
    
    for (int f = startFraction; f >= 0; f -= fractionInterval)
    {
        m_miniPacket[startPoint++] = (PacketSize >> f) & 0xFF;
    }
    
    for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendFrameNumber
    {
        m_miniPacket[startPoint++] = (resendFrameNumber >> f) & 0xFF;
    }
    for (int f = startFraction; f >= 0; f -= fractionInterval) //ResendPacketNumber
    {
        m_miniPacket[startPoint++] = (resendPacketNumber >> f) & 0xFF;
    }

    
    startPoint = PACKET_HEADER_LENGTH+1;
    
    m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;
    m_pCommonElementsBucket->SendFunctionPointer(friendID,2,m_miniPacket,PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
    
    //m_SendingBuffer.Queue(frameNumber, miniPacket, PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
    
}



