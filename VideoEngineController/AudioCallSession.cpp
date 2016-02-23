#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    #include <dispatch/dispatch.h>
#endif
//#include <android/log.h>

//#define LOG_TAG "NewTest"
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

CAudioCallSession::CAudioCallSession(LongLong fname, CCommonElementsBucket* sharedObject) :

m_pCommonElementsBucket(sharedObject)

{
	m_pSessionMutex.reset(new CLockHandler);
	friendID = fname;
    
    StartEncodingThread();
    StartDecodingThread();

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");
}

CAudioCallSession::~CAudioCallSession()
{
    StopDecodingThread();
    StopEncodingThread();


    delete m_pG729CodecNative;

    
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

	friendID = -1;

	SHARED_PTR_DELETE(m_pSessionMutex);
}

LongLong CAudioCallSession::GetFriendID()
{
	return friendID;
}

void CAudioCallSession::InitializeAudioCallSession(LongLong lFriendID)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession");

	//this->m_pAudioEncoder = new CAudioEncoder(m_pCommonElementsBucket);

	//m_pAudioEncoder->CreateAudioEncoder();

	//this->m_pAudioDecoder = new CAudioDecoder(m_pCommonElementsBucket);

	//m_pAudioDecoder->CreateAudioDecoder();

	m_pG729CodecNative = new G729CodecNative();
	int iRet = m_pG729CodecNative->Open();

	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession session initialized, iRet = " + m_Tools.IntegertoStringConvert(iRet));

}
long long iMS = -1;
int iAudioDataCounter = 0;
int CAudioCallSession::EncodeAudioData(short *in_data, unsigned int in_size)
{
    /*CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData 1");
	int size = m_pG729CodecNative->Encode(in_data, in_size, &m_EncodedFrame[1]);
    m_EncodingFrame[0] = 0;
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData encoded");

	//m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(1, size, m_EncodingFrame);
    
    m_pCommonElementsBucket->SendFunctionPointer(friendID,1,m_EncodedFrame,size);
     
    return 1;*/
    
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");
    
    
    /*iAudioDataCounter++;
    if(iMS == -1) iMS = m_Tools.CurrentTimestamp();
    if(m_Tools.CurrentTimestamp() - iMS >= 1000)
    {
        printf("TheVampire--> Number of AudioData in 1Sec = %d\n", iAudioDataCounter);
        iAudioDataCounter = 0;
        iMS = m_Tools.CurrentTimestamp();
    }*/
    
    int returnedValue = m_EncodingBuffer.Queue(in_data, in_size);
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");

    return returnedValue;
}

int CAudioCallSession::DecodeAudioData(unsigned char *in_data, unsigned int in_size)
{
    if(in_size > 200)
    {
        CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::DecodeAudioData BIG AUDIO !!!");
        return 0;
    }
    
    /*CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CController::DecodeAudioData in_size " + m_Tools.IntegertoStringConvert(in_size));
    
	int size = m_pG729CodecNative->Decode(&in_data[1], in_size-1, m_DecodedFrame);
    
    CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CController::DecodeAudioData size " + m_Tools.IntegertoStringConvert(size));
    
	m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(friendID, size, m_DecodedFrame);
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::DecodeAudioData 3");
     
    return 1;*/
    
    int returnedValue = m_DecodingBuffer.Queue(&in_data[1], in_size-1);

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
        bEncodingThreadRunning = false;
        
        while (!bEncodingThreadClosed)
            m_Tools.SOSleep(5);
    }
    
    //pInternalThread.reset();
}

void CAudioCallSession::StartEncodingThread()
{
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread 1");
    
    if (pEncodingThread.get())
    {
        CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread 2");
        
        pEncodingThread.reset();
        
        CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread 3");
        
        return;
    }
    
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread 4");
    
    bEncodingThreadRunning = true;
    bEncodingThreadClosed = false;
    
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread 5");
    
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
    int frameSize, encodedFrameSize;
    
    while (bEncodingThreadRunning)
    {
        CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodingThreadProcedure");
        
        if (m_EncodingBuffer.GetQueueSize() == 0)
            toolsObject.SOSleep(10);
        else
        {
            frameSize = m_EncodingBuffer.DeQueue(m_EncodingFrame);

            CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData 1, frameSize = " + m_Tools.IntegertoStringConvert(frameSize));
            int size;


            size = m_pG729CodecNative->Encode(m_EncodingFrame, frameSize, &m_EncodedFrame[1]);

            m_EncodedFrame[0] = 0;
            
            CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData encoded");
            
            //m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(1, size, m_EncodedFrame);
            
            m_pCommonElementsBucket->SendFunctionPointer(friendID,1,m_EncodedFrame,size);

            //toolsObject.SOSleep(1);
            
        }
    }
    
    bEncodingThreadClosed = true;
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}

void CAudioCallSession::StopDecodingThread()
{
    //if (pDecodingThread.get())
    {
        bDecodingThreadRunning = false;
        
        while (!bDecodingThreadClosed)
            m_Tools.SOSleep(5);
    }
    
    //pDecodingThread.reset();
}

void CAudioCallSession::StartDecodingThread()
{
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread 1");
    
    if (pDecodingThread.get())
    {
        CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread 2");
        
        pDecodingThread.reset();
        
        CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread 3");
        
        return;
    }
    
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread 4");
    
    bDecodingThreadRunning = true;
    bDecodingThreadClosed = false;
    
    CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread 5");
    
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
    int frameSize;
    
    while (bDecodingThreadRunning)
    {
        //CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::DecodingThreadProcedure");
        
        if (m_DecodingBuffer.GetQueueSize() == 0)
            toolsObject.SOSleep(10);
        else
        {
            frameSize = m_DecodingBuffer.DeQueue(m_DecodingFrame);
            
            CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure frameSize " + m_Tools.IntegertoStringConvert(frameSize));

            int size;

            size = m_pG729CodecNative->Decode(m_DecodingFrame, frameSize, m_DecodedFrame);

            CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure size " + m_Tools.IntegertoStringConvert(size));
#if defined(DUMP_DECODED_AUDIO)
			m_Tools.WriteToFile(m_DecodedFrame, size);
#endif
            m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(friendID, size, m_DecodedFrame);
            
            CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure 3");

            toolsObject.SOSleep(1);
        }
    }
    
    bDecodingThreadClosed = true;
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Stopped DecodingThreadProcedure method.");
}

