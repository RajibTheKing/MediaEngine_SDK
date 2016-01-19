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
		m_ll1stDecodedFrameTimeStamp(0),
		m_pEncodedFramePacketizer(NULL),
		m_pVideoEncoder(NULL)
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
	m_pEncodedFramePacketizer = new CEncodedFramePacketizer(sharedObject);
	m_pEncodedFrameDepacketizer = new CEncodedFrameDepacketizer(sharedObject,this);

	g_FriendID = fname;

	ExpectedFramePacketPair.first = 0;
	ExpectedFramePacketPair.second = 0;
	iNumberOfPacketsInCurrentFrame = 0;

	StartRenderingThread();
	StartEncodingThread();
	StartDepacketizationThread();
	StartDecodingThread();


	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::CVideoCallSession created");
}

CVideoCallSession::~CVideoCallSession()
{
	StopDepacketizationThread();
	StopDecodingThread();

	StopEncodingThread();
	StopRenderingThread();

	if(NULL!=m_pVideoEncoder)
	{
		delete m_pVideoEncoder;
		m_pVideoEncoder = NULL;
	}

	if(NULL!=m_pEncodedFramePacketizer)
	{
		delete m_pEncodedFramePacketizer;
		m_pEncodedFramePacketizer = NULL;
	}

	if(NULL!=m_pEncodedFrameDepacketizer)
	{
		delete m_pEncodedFrameDepacketizer;
		m_pEncodedFrameDepacketizer = NULL;
	}

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
	if(  ((in_data[4] >> 7) & 1) /* ||  ((in_data[4] >> 6) & 1) */ ) //If MiniPacket or RetransMitted packet
    {
        CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CVideoCallSession::PushPacketForMerging --> GOT RETRANSMITTED PACKET");
		m_pRetransVideoPacketQueue.Queue(in_data,in_size);
    }
    else if(((in_data[4] >> 6) & 1))
    {
        CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CVideoCallSession::PushPacketForMerging --> GOT MINI PACKET");
        m_pMiniPacketQueue.Queue(in_data, in_size);
    }
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
	long long encodingTime, encodingTimeStamp, nMaxEncodingTime = 0;
	double dbTotalEncodingTime=0;
	int iEncodedFrameCounter=0;

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
			encodingTimeStamp = toolsObject.CurrentTimestamp();
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
			encodingTime  = toolsObject.CurrentTimestamp() - encodingTimeStamp;
			dbTotalEncodingTime += encodingTime;
			++ iEncodedFrameCounter;
			nMaxEncodingTime = max(nMaxEncodingTime,encodingTime);

			if(0 == (7&iEncodedFrameCounter)){
				CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "Force: AVG EncodingTime = "+ m_Tools.DoubleToString(dbTotalEncodingTime/iEncodedFrameCounter)+" ~ "+m_Tools.IntegertoStringConvert(nMaxEncodingTime));
			}

			CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure video data encoded");

#endif

//			CLogPrinter::WriteSpecific(CLogPrinter::INFO, "CVideoCallSession::EncodingThreadProcedure m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));
//			CLogPrinter::WriteSpecific(CLogPrinter::INFO, "$ENCODEING$ To Parser");
			//m_pVideoEncoder->GetEncodedFramePacketizer()->Packetize(friendID,m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
			m_pEncodedFramePacketizer->Packetize(friendID,m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
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


int CVideoCallSession::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize,int nFramNumber, unsigned int nTimeStampDiff)
{
	m_decodedFrameSize = m_pVideoDecoder->Decode(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

	if(0 == m_decodedFrameSize)
		return 0;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	this->m_pColorConverter->ConvertI420ToNV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#else

	this->m_pColorConverter->ConvertI420ToNV21(m_DecodedFrame, m_decodingHeight, m_decodingWidth);

#endif

	m_RenderingBuffer.Queue(nFramNumber, m_DecodedFrame,m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth);

	return m_decodedFrameSize;
}

void CVideoCallSession::DepacketizationThreadProcedure()		//Merging Thread
{
	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Started DepacketizationThreadProcedure method.");
	Tools toolsObject;
	unsigned char temp;
	int frameSize,queSize=0,retQueuSize=0, miniPacketQueueSize = 0,consicutiveRetransmittedPkt=0;
	int frameNumber,packetNumber;
	bool bIsMiniPacket;
	m_iCountRecResPack = 0;

	while (bDepacketizationThreadRunning)
	{
		bIsMiniPacket = false;
		//CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::DepacketizationThreadProcedure");
		queSize = m_pVideoPacketQueue.GetQueueSize();
#ifdef	RETRANSMISSION_ENABLED
		retQueuSize = m_pRetransVideoPacketQueue.GetQueueSize();
        miniPacketQueueSize = m_pMiniPacketQueue.GetQueueSize();
		CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "SIZE "+ m_Tools.IntegertoStringConvert(retQueuSize)+"  "+ m_Tools.IntegertoStringConvert(queSize));
#endif
		if (0 == queSize && 0 == retQueuSize && 0 == miniPacketQueueSize)
			toolsObject.SOSleep(10);
		else
		{
#ifdef	RETRANSMISSION_ENABLED
            if(miniPacketQueueSize !=0)
            {
                frameSize = m_pMiniPacketQueue.DeQueue(m_PacketToBeMerged);
            }
			else if(retQueuSize>0 && consicutiveRetransmittedPkt<2)
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
            bool bMiniPacket = (m_PacketToBeMerged[4] >> 6) & 1;
       
			m_PacketToBeMerged[4] &= ~(1<<7); //Removed the Retransmit flag from the LMB of Number of Packets
            m_PacketToBeMerged[4] &= ~(1<<6); //Removed the MiniPacket flag from the LMB of Number of Packets

			if(!bRetransmitted && !bMiniPacket)
			{
				int iNumberOfPackets = -1;
				temp = m_PacketToBeMerged[SIGNAL_BYTE_INDEX];
				m_PacketToBeMerged[SIGNAL_BYTE_INDEX]=0;
				pair<int, int> currentFramePacketPair = m_Tools.GetFramePacketFromHeader(m_PacketToBeMerged , iNumberOfPackets);
				m_PacketToBeMerged[SIGNAL_BYTE_INDEX]=temp;

				if (currentFramePacketPair != ExpectedFramePacketPair && !m_pVideoPacketQueue.PacketExists(ExpectedFramePacketPair.first, ExpectedFramePacketPair.second)) //Out of order frame found, need to retransmit
				{
                    
                    CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::Current(FN,PN) = ("
                                               + m_Tools.IntegertoStringConvert(currentFramePacketPair.first)
                                               + ","
                                               + m_Tools.IntegertoStringConvert(currentFramePacketPair.second)
                                               + ") and Expected(FN,PN) = ("
                                               + m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first)
                                               + ","
                                               + m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second)
                                               + ")" );
                    
                    if(currentFramePacketPair.first != ExpectedFramePacketPair.first) //different frame received
                    {
                        if(currentFramePacketPair.first - ExpectedFramePacketPair.first == 2) //one complete frame missed, maybe it was a mini frame containing only 1 packet
                        {
                            CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "ExpectedFramePacketPair case 1");
                            CreateAndSendMiniPacket(ExpectedFramePacketPair.first, ExpectedFramePacketPair.second);
                            pair<int, int> requestFramePacketPair;

							requestFramePacketPair.first = currentFramePacketPair.first;

							requestFramePacketPair.second = 0;

							int iSendCounter = 0;

							while(requestFramePacketPair.second < currentFramePacketPair.second) //
							{
								if(iSendCounter /*&& requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);

								if(!m_pVideoPacketQueue.PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
								{
									CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
								}

								iSendCounter ++;
								requestFramePacketPair.second ++;
							}
                        }
                        else if(currentFramePacketPair.first - ExpectedFramePacketPair.first == 1) //last packets from last frame and some packets from current misssed
                        {
                            CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "ExpectedFramePacketPair case 2");
                            pair<int, int> requestFramePacketPair;
                            requestFramePacketPair.first = ExpectedFramePacketPair.first;
                            requestFramePacketPair.second = ExpectedFramePacketPair.second;
                            
                            int iSendCounter = 0;
                            while(requestFramePacketPair.second < iNumberOfPacketsInCurrentFrame)
                            {
                                if(iSendCounter /*&& requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
                                if(!m_pVideoPacketQueue.PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
                                {
                                    CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
                                }
                                iSendCounter ++;
                                requestFramePacketPair.second ++;
                            }
                            
                            requestFramePacketPair.first = currentFramePacketPair.first;
                            requestFramePacketPair.second = 0;
                            
                            iSendCounter = 0;
                            while(requestFramePacketPair.second < currentFramePacketPair.second)
                            {
                                if(iSendCounter /*&& requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
                                if(!m_pVideoPacketQueue.PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
                                {
                                    CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
                                }
                                iSendCounter ++;
                                requestFramePacketPair.second ++;
                            }
                            
                        }
                        else//we dont handle burst frame miss, but 1st packets of the current frame should come, only if it is an iFrame
                        {
                            CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "ExpectedFramePacketPair case 3-- killed previous frames");
                           	if(currentFramePacketPair.first % 8 == 0)
							{
								pair<int, int> requestFramePacketPair;
								requestFramePacketPair.first = currentFramePacketPair.first;
								requestFramePacketPair.second = 0;

								int iSendCounter = 0;
								while(requestFramePacketPair.second < currentFramePacketPair.second)
								{
									if(iSendCounter /*&& requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
									if(!m_pVideoPacketQueue.PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
									{
										CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
									}
									iSendCounter ++;
									requestFramePacketPair.second ++;
								}
							}
                        }
                        
                    }
                    else //packet missed from same frame
                    {
                        CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "ExpectedFramePacketPair case 4");
                        pair<int, int> requestFramePacketPair;
                        requestFramePacketPair.first = ExpectedFramePacketPair.first;
                        requestFramePacketPair.second = ExpectedFramePacketPair.second;
                        
                        int iSendCounter = 0;
                        while(requestFramePacketPair.second < currentFramePacketPair.second)
                        {
                            if(iSendCounter /* && requestFramePacketPair.first %8 ==0*/) m_Tools.SOSleep(1);
                            if(!m_pVideoPacketQueue.PacketExists(requestFramePacketPair.first, requestFramePacketPair.second))
                            {
                                CreateAndSendMiniPacket(requestFramePacketPair.first, requestFramePacketPair.second);
                            }
                            iSendCounter ++;
                            requestFramePacketPair.second ++;
                        }
                    }
                    
                    
					g_timeInt.setTime(ExpectedFramePacketPair.first,ExpectedFramePacketPair.second);
                    
                    
				}
				UpdateExpectedFramePacketPair(currentFramePacketPair, iNumberOfPackets);

			}
			else if (bRetransmitted)
			{
				int iNumberOfPackets = -1;
                
				m_PacketToBeMerged[SIGNAL_BYTE_INDEX]|=(1<<4); //the retransmitted flag is moved to signal byte
                
                pair<int, int> currentFramePacketPair = m_Tools.GetFramePacketFromHeader(m_PacketToBeMerged , iNumberOfPackets);

                CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::ReTransmitted: FrameNumber: "+ m_Tools.IntegertoStringConvert(currentFramePacketPair.first) + " PacketNumber. : "+  m_Tools.IntegertoStringConvert(currentFramePacketPair.second));
			}
            else if (bMiniPacket)
            {
                int iNumberOfPackets = -1;
                pair<int, int> currentFramePacketPair = m_Tools.GetFramePacketFromHeader(m_PacketToBeMerged , iNumberOfPackets);
                CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::Minipacket: FrameNumber: "+ m_Tools.IntegertoStringConvert(currentFramePacketPair.first) + " PacketNumber. : "+  m_Tools.IntegertoStringConvert(currentFramePacketPair.second));
//                m_PacketToBeMerged[SIGNAL_BYTE_INDEX]|=(1<<5); //the mini packet flag is moved to signal byte
				bIsMiniPacket = true;
            }



#endif
			m_pEncodedFrameDepacketizer->Depacketize(m_PacketToBeMerged, frameSize, bIsMiniPacket);

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
	long long firstTime,decodingTime,nMaxDecodingTime=0;
	int nDecodingStatus, fps = -1;
	double dbAverageDecodingTime, dbTotalDecodingTime = 0;
	int nOponnentFPS, nMaxProcessableByMine;
	m_iDecodedFrameCounter = 0;


	while (bDecodingThreadRunning)
	{
		//CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::DepacketizationThreadProcedure");

		if (m_DecodingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(5);
		else
		{
			firstTime = toolsObject.CurrentTimestamp();
			frameSize = m_DecodingBuffer.DeQueue(nFrameNumber, nTimeStampDiff, m_PacketizedFrame);

			nOponnentFPS = g_FPSController.GetOpponentFPS();
			nMaxProcessableByMine = g_FPSController.GetMaxOwnProcessableFPS();

			if( nOponnentFPS > 1 + nMaxProcessableByMine  && (nFrameNumber & 7) > 3 ) {
				CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "Force:: Frame: "+m_Tools.IntegertoStringConvert(nFrameNumber)+"  FPS: "+m_Tools.IntegertoStringConvert(nOponnentFPS)+" ~"+toolsObject.IntegertoStringConvert(nMaxProcessableByMine));
				toolsObject.SOSleep(5);
				continue;
			}

			nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame, frameSize, nFrameNumber, nTimeStampDiff);
//			toolsObject.SOSleep(100);

			if(nDecodingStatus > 0) {
				decodingTime = toolsObject.CurrentTimestamp() - firstTime;
				dbTotalDecodingTime += decodingTime;
				++ m_iDecodedFrameCounter;
				nMaxDecodingTime = max(nMaxDecodingTime, decodingTime);
				if( 0 == (m_iDecodedFrameCounter & 3) )
				{
					dbAverageDecodingTime = dbTotalDecodingTime / m_iDecodedFrameCounter;
					dbAverageDecodingTime *=1.5;
					fps = 1000 / dbAverageDecodingTime;

					if(fps<FPS_MAXIMUM)
						g_FPSController.SetMaxOwnProcessableFPS(fps);
				}
				CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "Force:: AVG Decoding Time:"+m_Tools.DoubleToString(dbAverageDecodingTime)+"  Max Decoding-time: "+m_Tools.IntegertoStringConvert(nMaxDecodingTime)+"  MaxOwnProcessable: "+m_Tools.IntegertoStringConvert(fps));
			}

//			if(intervalTime > decodingTime+5)
//				toolsObject.SOSleep(intervalTime-decodingTime-5);
//			else
				toolsObject.SOSleep(3);
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
		iNumberOfPacketsInCurrentFrame = 1;//next frame has at least 1 packet, it will be updated when a packet is received
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

    if(resendFrameNumber %8 !=0)//faltu frame, dorkar nai
    {
        return;
    }

    
    
    CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::CreateAndSendMiniPacket() resendFrameNumber = " + m_Tools.IntegertoStringConvert(resendFrameNumber) +
                                            ", resendPacketNumber = " + m_Tools.IntegertoStringConvert(resendPacketNumber));
    int startFraction = SIZE_OF_INT_MINUS_8;
    int fractionInterval = BYTE_SIZE;
    int startPoint = 1;
    
    
    int numberOfPackets = 1000; //dummy numberOfPackets
    
    
    for (int f = startFraction; f >= 0; f -= fractionInterval)
    {
        m_miniPacket[startPoint++] = (resendFrameNumber >> f) & 0xFF; //resend Frame Number
    }
    
    for (int f = startFraction; f >= 0; f -= fractionInterval)
    {
        m_miniPacket[startPoint++] = (numberOfPackets >> f) & 0xFF; //Dummy numberOfPackets 1000
    }
    
    m_miniPacket[4 + 1] |= 1<<6; //MiniPacket Flag
    
    for (int f = startFraction; f >= 0; f -= fractionInterval)
    {
        m_miniPacket[startPoint++] = (resendPacketNumber >> f) & 0xFF; //resend packet Number
    }
    m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;
    
    m_pCommonElementsBucket->SendFunctionPointer(friendID,2,m_miniPacket,MINI_PACKET_LENGTH_WITH_MEDIA_TYPE);
    
    //m_SendingBuffer.Queue(frameNumber, miniPacket, PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
    
}






















void CVideoCallSession::StopRenderingThread()
{
	//if (pInternalThread.get())
	{
		bRenderingThreadRunning = false;

		while (!bRenderingThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pInternalThread.reset();
}

void CVideoCallSession::StartRenderingThread()
{
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread 1");

	if (pRenderingThread.get())
	{
		CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread 2");
		pRenderingThread.reset();
		CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread 3");
		return;
	}
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread 4");
	bRenderingThreadRunning = true;
	bRenderingThreadClosed = false;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread 5");

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t RenderThreadQ = dispatch_queue_create("RenderThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(RenderThreadQ, ^{
        this->RenderingThreadProcedure();
    });

#else

	std::thread myThread(CreateVideoRenderingThread, this);
	myThread.detach();

#endif

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread Rendering Thread started");

	return;
}

void *CVideoCallSession::CreateVideoRenderingThread(void* param)
{
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->RenderingThreadProcedure();

	return NULL;
}

void CVideoCallSession::RenderingThreadProcedure()
{
	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::RenderingThreadProcedure() Started EncodingThreadProcedure.");
	Tools toolsObject;
	int frameSize,nFrameNumber,intervalTime;
	unsigned int nTimeStampDiff;
	long long firstTime,decodingTime,firstFrameEncodingTime;
	int videoHeight, videoWidth;

	while (bRenderingThreadRunning)
	{
		//CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::RenderingThreadProcedure");

		if (m_RenderingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			firstTime = toolsObject.CurrentTimestamp();
			frameSize = m_RenderingBuffer.DeQueue(nFrameNumber, nTimeStampDiff, m_RenderingFrame, videoHeight, videoWidth);

			if(frameSize>0)
			{
				if(m_b1stDecodedFrame)
				{
					m_ll1stDecodedFrameTimeStamp = firstTime;
					firstFrameEncodingTime = nTimeStampDiff;
					m_b1stDecodedFrame = false;
				}

				int DecodingDelay = nTimeStampDiff - firstFrameEncodingTime + m_ll1stDecodedFrameTimeStamp - firstTime;

				CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() n timeStampDiff: "+m_Tools.IntegertoStringConvert(nTimeStampDiff)+ " ::DecodingDelay: "+ m_Tools.IntegertoStringConvert(DecodingDelay));
				if(DecodingDelay>5)
					toolsObject.SOSleep(DecodingDelay-5);
				else
					toolsObject.SOSleep(1);

				m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(friendID, nFrameNumber, frameSize, m_RenderingFrame, videoHeight, videoWidth);

			}
//			toolsObject.SOSleep(1);
		}
	}

	bRenderingThreadClosed = true;

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::RenderingThreadProcedure() Stopped EncodingThreadProcedure");
}









