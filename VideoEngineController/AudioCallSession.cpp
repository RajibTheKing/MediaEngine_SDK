#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    #include <dispatch/dispatch.h>
#endif

#define OPUS_ENABLE

CAudioCallSession::CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, bool bIsCheckCall) :

m_pCommonElementsBucket(pSharedObject),
m_bIsCheckCall(bIsCheckCall)

{
	m_pAudioCallSessionMutex.reset(new CLockHandler);
	m_FriendID = llFriendID;
    
    StartEncodingThread();
    StartDecodingThread();

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");
}

CAudioCallSession::~CAudioCallSession()
{
    StopDecodingThread();
    StopEncodingThread();

#ifdef OPUS_ENABLE
    delete m_pAudioEncoder;
#else
    delete m_pG729CodecNative;
#endif

	/*if (NULL != m_pAudioDecoder)
	{
		delete m_pAudioDecoder;

		m_pAudioDecoder = NULL;
	}

	if (NULL != m_pAudioEncoder)
	{
		delete m_pAudioEncoder;

		m_pAudioEncoder = NULL;
	}*/

	m_FriendID = -1;

	SHARED_PTR_DELETE(m_pAudioCallSessionMutex);
}

void CAudioCallSession::InitializeAudioCallSession(LongLong llFriendID)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession");

	//this->m_pAudioEncoder = new CAudioEncoder(m_pCommonElementsBucket);

	//m_pAudioEncoder->CreateAudioEncoder();

	//this->m_pAudioDecoder = new CAudioDecoder(m_pCommonElementsBucket);

	//m_pAudioDecoder->CreateAudioDecoder();
#ifdef OPUS_ENABLE
    this->m_pAudioEncoder = new CAudioEncoder(m_pCommonElementsBucket);
    m_pAudioEncoder->CreateAudioEncoder();
#else
    m_pG729CodecNative = new G729CodecNative();
    int iRet = m_pG729CodecNative->Open();
#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession session initialized, iRet = " + m_Tools.IntegertoStringConvert(iRet));

}

long long iMS = -1;
int iAudioDataCounter = 0;

int CAudioCallSession::EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength)
{
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");

	int returnedValue = m_AudioEncodingBuffer.Queue(psaEncodingAudioData, unLength);
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");

    return returnedValue;
}

int CAudioCallSession::DecodeAudioData(unsigned char *pucaDecodingAudioData, unsigned int unLength)
{
	if (unLength > 300)
    {
        CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::DecodeAudioData BIG AUDIO !!!");
        return 0;
    }
    
	int returnedValue = m_AudioDecodingBuffer.Queue(&pucaDecodingAudioData[1], unLength - 1);

	return returnedValue;
}

CAudioEncoder* CAudioCallSession::GetAudioEncoder()
{
	//	return sessionMediaList.GetFromAudioEncoderList(mediaName);

	return m_pAudioEncoder;
}

CAudioDecoder* CAudioCallSession::GetAudioDecoder()
{
	//	return sessionMediaList.GetFromAudioEncoderList(mediaName);

	return m_pAudioDecoder;
}


void CAudioCallSession::StopEncodingThread()
{
    //if (pInternalThread.get())
    {
        m_bAudioEncodingThreadRunning = false;
        
        while (!m_bAudioEncodingThreadClosed)
            m_Tools.SOSleep(5);
    }
    
    //pInternalThread.reset();
}

void CAudioCallSession::StartEncodingThread()
{
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread 1");
    
    if (m_pAudioEncodingThread.get())
    {
        m_pAudioEncodingThread.reset();
        
        return;
    }
    
    m_bAudioEncodingThreadRunning = true;
    m_bAudioEncodingThreadClosed = false;
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    
    dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(EncodeThreadQ, ^{
        this->EncodingThreadProcedure();
    });
    
#else
    
    std::thread myThread(CreateAudioEncodingThread, this);
    myThread.detach();
    
#endif
    
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread Encoding Thread started");
    
    return;
}

void *CAudioCallSession::CreateAudioEncodingThread(void* param)
{
    CAudioCallSession *pThis = (CAudioCallSession*)param;
    pThis->EncodingThreadProcedure();
    
    return NULL;
}

void CAudioCallSession::EncodingThreadProcedure()
{
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
    Tools toolsObject;
    int nEncodingFrameSize, nEncodedFrameSize;
    long long timeStamp;
    long avgCountTimeStamp = 0;
    int countFrame = 0;

    while (m_bAudioEncodingThreadRunning)
    {
        if (m_AudioEncodingBuffer.GetQueueSize() == 0)
            toolsObject.SOSleep(10);
        else
        {
			nEncodingFrameSize = m_AudioEncodingBuffer.DeQueue(m_saAudioEncodingFrame);

            int nEncodedFrameSize;

            timeStamp = m_Tools.CurrentTimestamp();
#ifdef OPUS_ENABLE
            nEncodedFrameSize = m_pAudioEncoder->encodeAudio(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1]);
#else
            nEncodedFrameSize = m_pG729CodecNative->Encode(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1]);
#endif
            m_saAudioEncodingFrame[0] = 0;
            int time = m_Tools.CurrentTimestamp() - timeStamp;
            avgCountTimeStamp += time;
            countFrame++;

            CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"#ENcode");
            CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize)
                                                          + " nEncodedFrameSize =" + m_Tools.IntegertoStringConvert(nEncodedFrameSize) +" ratio: " +m_Tools.DoubleToString((nEncodedFrameSize*100)/nEncodingFrameSize)
                                                          +" encodeTime: " + m_Tools.IntegertoStringConvert(time)
                                                          +" AvgTime: " + m_Tools.DoubleToString(avgCountTimeStamp / countFrame));

            //m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(1, size, m_EncodedFrame);
            
			if (m_bIsCheckCall == LIVE_CALL_MOOD)
				m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaEncodedFrame, nEncodedFrameSize);
			else
				DecodeAudioData(m_ucaEncodedFrame, nEncodedFrameSize);
//            m_AudioDecodingBuffer.Queue(m_ucaEncodedFrame, nEncodedFrameSize);

            toolsObject.SOSleep(0);
            
        }
    }
    
    m_bAudioEncodingThreadClosed = true;
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}

void CAudioCallSession::StopDecodingThread()
{
    //if (m_pAudioDecodingThread.get())
    {
        m_bAudioDecodingThreadRunning = false;
        
        while (!m_bAudioDecodingThreadClosed)
            m_Tools.SOSleep(5);
    }
    
    //m_pAudioDecodingThread.reset();
}

void CAudioCallSession::StartDecodingThread()
{
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread 1");
    
    if (m_pAudioDecodingThread.get())
    {
        m_pAudioDecodingThread.reset();
        
        return;
    }
    
    m_bAudioDecodingThreadRunning = true;
    m_bAudioDecodingThreadClosed = false;
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    
    dispatch_queue_t DecodeThreadQ = dispatch_queue_create("DecodeThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(DecodeThreadQ, ^{
        this->DecodingThreadProcedure();
    });
    
#else
    
    std::thread myThread(CreateAudioDecodingThread, this);
    myThread.detach();
    
#endif
    
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread Decoding Thread started");
    
    return;
}

void *CAudioCallSession::CreateAudioDecodingThread(void* param)
{
    CAudioCallSession *pThis = (CAudioCallSession*)param;
    pThis->DecodingThreadProcedure();
    
    return NULL;
}

void CAudioCallSession::DecodingThreadProcedure()
{
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Started DecodingThreadProcedure method.");

    Tools toolsObject;
    int nDecodingFrameSize, nDecodedFrameSize;
    long long timeStamp;

    while (m_bAudioDecodingThreadRunning)
    {
        if (m_AudioDecodingBuffer.GetQueueSize() == 0)
            toolsObject.SOSleep(10);
        else
        {
			nDecodingFrameSize = m_AudioDecodingBuffer.DeQueue(m_ucaDecodingFrame);
            //int size;

            timeStamp = m_Tools.CurrentTimestamp();
#ifdef OPUS_ENABLE
            nDecodedFrameSize = m_pAudioEncoder->decodeAudio(m_ucaDecodingFrame, nDecodingFrameSize, m_saDecodedFrame);
#else
            nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame, nDecodingFrameSize, m_saDecodedFrame);
#endif
            CLogPrinter_WriteSpecific6(CLogPrinter::DEBUGS, "#DE#--->> size " + m_Tools.IntegertoStringConvert(nDecodedFrameSize) + " timeStamp: "+ m_Tools.IntegertoStringConvert(m_Tools.CurrentTimestamp() - timeStamp));
#if defined(DUMP_DECODED_AUDIO)

			m_Tools.WriteToFile(m_saDecodedFrame, size);

#endif

			if (m_bIsCheckCall == LIVE_CALL_MOOD)
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_FriendID, nDecodedFrameSize, m_saDecodedFrame);

            toolsObject.SOSleep(0);
        }
    }
    
    m_bAudioDecodingThreadClosed = true;
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Stopped DecodingThreadProcedure method.");
}

