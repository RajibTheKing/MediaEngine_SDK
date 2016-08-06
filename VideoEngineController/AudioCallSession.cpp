#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"

#define __AUDIO_SLEF_CALL__
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

	SendingHeader = new CAudioPacketHeader();
	ReceivingHeader = new CAudioPacketHeader();
	m_AudioHeadersize = SendingHeader->GetHeaderSize();

	m_iPacketNumber = 0;
	m_iSlotID = 0;
	m_iPrevRecvdSlotID = -1;
	m_iCurrentRecvdSlotID = -1;
	m_iOpponentReceivedPackets = AUDIO_SLOT_SIZE;
	m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;

	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");
}

CAudioCallSession::~CAudioCallSession()
{
    StopDecodingThread();
    StopEncodingThread();

#ifdef OPUS_ENABLE
    delete m_pAudioCodec;
#else
    delete m_pG729CodecNative;
#endif

	/*if (NULL != m_pAudioDecoder)
	{
		delete m_pAudioDecoder;

		m_pAudioDecoder = NULL;
	}

	if (NULL != m_pAudioCodec)
	{
		delete m_pAudioCodec;

		m_pAudioCodec = NULL;
	}*/

	m_FriendID = -1;

	SHARED_PTR_DELETE(m_pAudioCallSessionMutex);
}

void CAudioCallSession::InitializeAudioCallSession(LongLong llFriendID)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession");

	//this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket);

	//m_pAudioCodec->CreateAudioEncoder();

	//m_pAudioDecoder->CreateAudioDecoder();
#ifdef OPUS_ENABLE
    this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket);
    m_pAudioCodec->CreateAudioEncoder();
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
	/*if (unLength > 300)
    {
        CLogPrinter_Write(CLogPrinter::DEBUGS, "CController::DecodeAudioData BIG AUDIO !!!");
        return 0;
    }*/
    
	int returnedValue = m_AudioDecodingBuffer.Queue(&pucaDecodingAudioData[1], unLength - 1);

	return returnedValue;
}

CAudioCodec* CAudioCallSession::GetAudioEncoder()
{
	//	return sessionMediaList.GetFromAudioEncoderList(mediaName);

	return m_pAudioCodec;
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
    double avgCountTimeStamp = 0;
    int countFrame = 0;

    while (m_bAudioEncodingThreadRunning)
    {
        if (m_AudioEncodingBuffer.GetQueueSize() == 0)
            toolsObject.SOSleep(10);
        else
        {
			nEncodingFrameSize = m_AudioEncodingBuffer.DeQueue(m_saAudioEncodingFrame);
            if(AUDIO_CLIENT_SAMPLE_SIZE != nEncodingFrameSize)
            {
                ALOG("#EXP# nEncodingFrameSize: "+Tools::IntegertoStringConvert(nEncodingFrameSize));
                continue;
            }
            int nEncodedFrameSize;

            timeStamp = m_Tools.CurrentTimestamp();

#ifdef OPUS_ENABLE
            nEncodedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);
#else
            nEncodedFrameSize = m_pG729CodecNative->Encode(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);
#endif
            m_saAudioEncodingFrame[0] = 0;
            int encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
			m_pAudioCodec->DecideToChangeComplexity(encodingTime);
			avgCountTimeStamp += encodingTime;
            countFrame++;
            if(countFrame % 20 == 0)
            ALOG( "#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize)
                                                          + " nEncodedFrameSize = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize) +" ratio: " +m_Tools.DoubleToString((nEncodedFrameSize*100)/nEncodingFrameSize)
														  + " EncodeTime: " + m_Tools.IntegertoStringConvert(encodingTime)
                                                          +" AvgTime: " + m_Tools.DoubleToString(avgCountTimeStamp / countFrame));

            //m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(1, size, m_EncodedFrame);

			SendingHeader->SetInformation(m_iPacketNumber, PACKETNUMBER);
			SendingHeader->SetInformation(m_iSlotID, SLOTNUMBER);
			SendingHeader->SetInformation(nEncodedFrameSize, PACKETLENGTH);
			SendingHeader->SetInformation(m_iPrevRecvdSlotID, RECVDSLOTNUMBER);
			SendingHeader->SetInformation(m_iReceivedPacketsInPrevSlot, NUMPACKETRECVD);
			SendingHeader->GetHeaderInByteArray(&m_ucaEncodedFrame[1]);

//            ALOG("#V# E: PacketNumber: "+m_Tools.IntegertoStringConvert(m_iPacketNumber)
//                + " #V# E: SLOTNUMBER: "+m_Tools.IntegertoStringConvert(m_iSlotID)
//                + " #V# E: NUMPACKETRECVD: "+m_Tools.IntegertoStringConvert(m_iReceivedPacketsInPrevSlot)
//                + " #V# E: RECVDSLOTNUMBER: "+m_Tools.IntegertoStringConvert(m_iPrevRecvdSlotID)
//            );
			
			m_iPacketNumber = (m_iPacketNumber + 1) % SendingHeader->GetFieldCapacity(PACKETNUMBER);
			m_iSlotID = m_iPacketNumber / AUDIO_SLOT_SIZE;
			m_iSlotID %= SendingHeader->GetFieldCapacity(SLOTNUMBER);

//            ALOG("#DE#--->> QUEUE = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize + m_AudioHeadersize + 1));
//            CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "#DE#--->> QUEUE = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize + m_AudioHeadersize + 1));

#ifdef  __AUDIO_SLEF_CALL__
            DecodeAudioData(m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
#else
            if (m_bIsCheckCall == LIVE_CALL_MOOD)
				m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
			else
				DecodeAudioData(m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
#endif

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
    int nDecodingFrameSize, nDecodedFrameSize, iFrameCounter = 0;
    long long timeStamp, nDecodingTime = 0;
    double dbTotalTime = 0;
    toolsObject.SOSleep(1000);
    while (m_bAudioDecodingThreadRunning)
    {
        if (m_AudioDecodingBuffer.GetQueueSize() == 0)
            toolsObject.SOSleep(10);
        else
        {
			nDecodingFrameSize = m_AudioDecodingBuffer.DeQueue(m_ucaDecodingFrame);
//            ALOG( "#DE#--->> nDecodingFrameSize = " + m_Tools.IntegertoStringConvert(nDecodingFrameSize));
            timeStamp = m_Tools.CurrentTimestamp();
			ReceivingHeader->CopyHeaderToInformation(m_ucaDecodingFrame);
//            ALOG("#V# PacketNumber: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(PACKETNUMBER))
//                    + " #V# SLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(SLOTNUMBER))
//                    + " #V# NUMPACKETRECVD: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(NUMPACKETRECVD))
//                    + " #V# RECVDSLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(RECVDSLOTNUMBER))
//            );

			m_iOpponentReceivedPackets = ReceivingHeader->GetInformation(NUMPACKETRECVD);
			
			if (ReceivingHeader->GetInformation(SLOTNUMBER) != m_iCurrentRecvdSlotID)
			{
				m_iPrevRecvdSlotID = m_iCurrentRecvdSlotID;
				if (m_iPrevRecvdSlotID != -1)
				{
					m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot;
				}

				m_iCurrentRecvdSlotID = ReceivingHeader->GetInformation(SLOTNUMBER);
				m_iReceivedPacketsInCurrentSlot = 0;
				m_pAudioCodec->DecideToChangeBitrate(m_iOpponentReceivedPackets);
			}
			
			m_iReceivedPacketsInCurrentSlot ++;
            //continue;
			nDecodingFrameSize -= m_AudioHeadersize;

#ifdef OPUS_ENABLE
            nDecodedFrameSize = m_pAudioCodec->decodeAudio(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
#else
            nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame, nDecodingFrameSize + m_AudioHeadersize, m_saDecodedFrame);
#endif
            ++iFrameCounter;
            nDecodingTime = m_Tools.CurrentTimestamp() - timeStamp;
            dbTotalTime += nDecodingTime;
            if(iFrameCounter % 20 == 0)
                ALOG( "#DE#--->> Size " + m_Tools.IntegertoStringConvert(nDecodedFrameSize) + " DecodingTime: "+ m_Tools.IntegertoStringConvert(nDecodingTime) + "A.D.Time : "+m_Tools.DoubleToString(dbTotalTime / iFrameCounter));
#if defined(DUMP_DECODED_AUDIO)
			m_Tools.WriteToFile(m_saDecodedFrame, size);
#endif
            if(nDecodedFrameSize < 1)
            {
                ALOG("#EXP# Decoding Failed.");
                continue;
            }
			if (m_bIsCheckCall == LIVE_CALL_MOOD )
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_FriendID, nDecodedFrameSize, m_saDecodedFrame);

            toolsObject.SOSleep(0);
        }
    }
    
    m_bAudioDecodingThreadClosed = true;
    
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Stopped DecodingThreadProcedure method.");
}

