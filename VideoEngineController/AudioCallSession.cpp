#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "Globals.h"

//#define __AUDIO_SELF_CALL__
//#define FIRE_ENC_TIME

//int g_iNextPacketType = 1;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    #include <dispatch/dispatch.h>
#endif

#define OPUS_ENABLE
//#define __DUMP_FILE__

#ifdef __DUMP_FILE__
FILE *FileInput;
FILE *FileOutput;
#endif

//extern int g_StopVideoSending;

CAudioCallSession::CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, bool bIsCheckCall) :

m_pCommonElementsBucket(pSharedObject),
m_bIsCheckCall(bIsCheckCall)

{
    Globals::g_bIsLiveStreaming = true;
    
#ifdef ONLY_FOR_LIVESTREAMING
    m_pLiveAudioDecodingQueue = new LiveAudioDecodingQueue();
    m_pLiveReceiverAudio = new LiveReceiver();
    m_pLiveReceiverAudio->SetAudioDecodingQueue(m_pLiveAudioDecodingQueue);
#endif  
    

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
    m_nMaxAudioPacketNumber = ( (1 << HeaderBitmap[PACKETNUMBER]) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;
	m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
#ifdef ONLY_FOR_LIVESTREAMING
    m_iAudioDataSendIndex = 0;
    m_vEncodedFrameLenght.clear();
#endif
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
    
    
#ifdef ONLY_FOR_LIVESTREAMING
    if(NULL != m_pLiveReceiverAudio)
    {
        delete m_pLiveReceiverAudio;
        m_pLiveReceiverAudio = NULL;
    }
    
    if(NULL != m_pLiveAudioDecodingQueue)
    {
        delete m_pLiveAudioDecodingQueue;
        
        m_pLiveAudioDecodingQueue = NULL;
    }
#endif
    
    
    
	m_FriendID = -1;
#ifdef __DUMP_FILE__
	fclose(FileOutput);
	fclose(FileInput);
#endif

	SHARED_PTR_DELETE(m_pAudioCallSessionMutex);
}

void CAudioCallSession::InitializeAudioCallSession(LongLong llFriendID)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession");

	//this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket);

	//m_pAudioCodec->CreateAudioEncoder();

	//m_pAudioDecoder->CreateAudioDecoder();
#ifdef OPUS_ENABLE
    this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket, this);
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

int CAudioCallSession::DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
//    ALOG("#H#Received PacketType: "+m_Tools.IntegertoStringConvert(pucaDecodingAudioData[0]));
    if(Globals::g_bIsLiveStreaming)
    {
        m_pLiveReceiverAudio->ProcessAudioStream(nOffset, pucaDecodingAudioData, unLength, frameSizes, numberOfFrames, missingFrames, numberOfMissingFrames);
        
        return 1;
    }

	int returnedValue = m_AudioDecodingBuffer.Queue(&pucaDecodingAudioData[1], unLength - 1);

	return returnedValue;
}

CAudioCodec* CAudioCallSession::GetAudioCodec()
{
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

#ifdef FIRE_ENC_TIME
int encodingtimetimes = 0, cumulitiveenctime = 0;
#endif

void CAudioCallSession::EncodingThreadProcedure()
{
    CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
#ifdef __DUMP_FILE__
	FileInput = fopen("/storage/emulated/0/InputPCMN.pcm", "w");
	//    FileInput = fopen("/stcard/emulated/0/InputPCM.pcm", "w");
#endif
    Tools toolsObject;
    int nEncodingFrameSize, nEncodedFrameSize, encodingTime;
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
            if( nEncodingFrameSize % AUDIO_FRAME_SIZE >0)
            {
                ALOG("#EXP# Client Sample Size not multiple of AUDIO-FRAME-SIZE = "+Tools::IntegertoStringConvert(nEncodingFrameSize));
            }
#ifdef __DUMP_FILE__
			fwrite(m_saAudioEncodingFrame, 2, nEncodingFrameSize, FileInput);
#endif
            int nEncodedFrameSize;

            timeStamp = m_Tools.CurrentTimestamp();
            countFrame++;

#ifdef OPUS_ENABLE
            nEncodedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);

#ifndef __AUDIO_FIXED_COMPLEXITY__
			encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
			m_pAudioCodec->DecideToChangeComplexity(encodingTime);
#endif
			avgCountTimeStamp += encodingTime;
#ifdef FIRE_ENC_TIME
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_ENCODING_TIME, encodingTime,  0);
			cumulitiveenctime += encodingTime;
			encodingtimetimes ++;
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_AVG_ENCODING_TIME, cumulitiveenctime * 1.0 / encodingtimetimes, 0);
#endif

#else
            nEncodedFrameSize = m_pG729CodecNative->Encode(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);
            encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
            avgCountTimeStamp += encodingTime;
#endif
//            if (countFrame % 100 == 0)
//                ALOG("#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize)
//                     + " nEncodedFrameSize = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize) + " ratio: " + m_Tools.DoubleToString((nEncodedFrameSize * 100) / nEncodingFrameSize)
//                     + " EncodeTime: " + m_Tools.IntegertoStringConvert(encodingTime)
//                     + " AvgTime: " + m_Tools.DoubleToString(avgCountTimeStamp / countFrame)
//                     + " MaxFrameNumber: " + m_Tools.IntegertoStringConvert(m_nMaxAudioPacketNumber));

            //m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(1, size, m_EncodedFrame);

//            SendingHeader->SetInformation( (countFrame%100 == 0)? 0 : 1, PACKETTYPE);

            m_iSlotID = m_iPacketNumber / AUDIO_SLOT_SIZE;
            m_iSlotID %= SendingHeader->GetFieldCapacity(SLOTNUMBER);

			SendingHeader->SetInformation(m_iNextPacketType, PACKETTYPE);
			if (m_iNextPacketType == AUDIO_NOVIDEO_PACKET_TYPE)
			{
				m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
			}
            int tempVal;
			SendingHeader->SetInformation(m_iPacketNumber, PACKETNUMBER);
			SendingHeader->SetInformation(m_iSlotID, SLOTNUMBER);
			SendingHeader->SetInformation(nEncodedFrameSize, PACKETLENGTH);
            printf("m_iPrevRecvdSlotID = %d\n", m_iPrevRecvdSlotID);
            //if(!Globals::g_bIsLiveStreaming)
            {
			SendingHeader->SetInformation(m_iPrevRecvdSlotID, RECVDSLOTNUMBER);
//            tempVal = ((int)(m_ucaEncodedFrame[4] & 0x3F)<<6) + ((int)(m_ucaEncodedFrame[5]&0xFC)>>2);
//            printf("^^ RECVDSLOTNUMBER : %d\n",tempVal);
			SendingHeader->SetInformation(m_iReceivedPacketsInPrevSlot, NUMPACKETRECVD);
                
//            tempVal = ((int)(m_ucaEncodedFrame[4] & 0x3F)<<6) + ((int)(m_ucaEncodedFrame[5]&0xFC)>>2);
//            printf("^^ NUMPACKETRECVD : %d\n",tempVal);
            }
			SendingHeader->GetHeaderInByteArray(&m_ucaEncodedFrame[1]);
            
            CAudioPacketHeader tmp;
            tmp.CopyHeaderToInformation(&m_ucaEncodedFrame[1]);
//            int val = ((int)(m_ucaEncodedFrame[4] & 0x3F)<<6) + ((int)(m_ucaEncodedFrame[5]&0xFC)>>2);
//            printf("^^ LEN: %d    LEN2: %d  Len3: %d\n",nEncodedFrameSize,tmp.GetInformation(PACKETLENGTH),val);
            m_ucaEncodedFrame[0] = 0;   //Setting Audio packet type( = 0).

//            ALOG("#V# E: PacketNumber: "+m_Tools.IntegertoStringConvert(m_iPacketNumber)
//                + " #V# E: SLOTNUMBER: "+m_Tools.IntegertoStringConvert(m_iSlotID)
//                + " #V# E: NUMPACKETRECVD: "+m_Tools.IntegertoStringConvert(m_iReceivedPacketsInPrevSlot)
//                + " #V# E: RECVDSLOTNUMBER: "+m_Tools.IntegertoStringConvert(m_iPrevRecvdSlotID)
//            );

            if(m_iPacketNumber >= m_nMaxAudioPacketNumber)
			    m_iPacketNumber = 0;
            else
                ++m_iPacketNumber;

//            ALOG("#DE#--->> QUEUE = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize + m_AudioHeadersize + 1));
//            CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "#DE#--->> QUEUE = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize + m_AudioHeadersize + 1));

#ifdef  __AUDIO_SELF_CALL__
            DecodeAudioData(m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
#else
            if (m_bIsCheckCall == LIVE_CALL_MOOD) {
//                ALOG("#H#Sent PacketType: "+m_Tools.IntegertoStringConvert(m_ucaEncodedFrame[0]));
#ifdef ONLY_FOR_LIVESTREAMING

                {
                    Locker lock(*m_pAudioCallSessionMutex);
                    if((m_iAudioDataSendIndex + nEncodedFrameSize + m_AudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE )
                    {
                        
                        memcpy(m_ucaAudioDataToSend + m_iAudioDataSendIndex,  m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
                        m_iAudioDataSendIndex += (nEncodedFrameSize + m_AudioHeadersize + 1);
                        m_vEncodedFrameLenght.push_back( nEncodedFrameSize + m_AudioHeadersize + 1 );
                    }
                }

#endif
                /*m_pCommonElementsBucket->SendFunctionPointer(m_ucaEncodedFrame,
                                                             nEncodedFrameSize + m_AudioHeadersize +
                                                             1);*/

            }
//			else
//				DecodeAudioData(m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
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
    bool bIsProcessablePacket;
    int nDecodingFrameSize, nDecodedFrameSize, iFrameCounter = 0, nCurrentAudioPacketType;
    long long timeStamp, nDecodingTime = 0;
    double dbTotalTime = 0;
    toolsObject.SOSleep(1000);
    int iDataSentInCurrentSec = 0;
    long long llTimeStamp = 0;
#ifdef __DUMP_FILE__
	FileOutput = fopen("/storage/emulated/0/OutputPCMN.pcm", "w");
#endif
	//toolsObject.SOSleep(1000);
    while (m_bAudioDecodingThreadRunning)
    {
#ifdef ONLY_FOR_LIVESTREAMING
        if (m_pLiveAudioDecodingQueue->GetQueueSize() == 0)
#else
        if (m_AudioDecodingBuffer.GetQueueSize() == 0)
#endif
        {
            toolsObject.SOSleep(10);
        }
        else
        {

#ifdef ONLY_FOR_LIVESTREAMING
            nDecodingFrameSize = m_pLiveAudioDecodingQueue->DeQueue(m_ucaDecodingFrame);
            LOGEF("THeKing--> *** CAudioCallSession::DecodingThreadProcedure : decodingFrameSize: %d", nDecodingFrameSize);
#else
			nDecodingFrameSize = m_AudioDecodingBuffer.DeQueue(m_ucaDecodingFrame);
#endif
            
            bIsProcessablePacket = false;
//            ALOG( "#DE#--->> nDecodingFrameSize = " + m_Tools.IntegertoStringConvert(nDecodingFrameSize));
            timeStamp = m_Tools.CurrentTimestamp();
			ReceivingHeader->CopyHeaderToInformation(m_ucaDecodingFrame);
//            ALOG("#V# PacketNumber: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(PACKETNUMBER))
//                    + " #V# SLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(SLOTNUMBER))
//                    + " #V# NUMPACKETRECVD: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(NUMPACKETRECVD))
//                    + " #V# RECVDSLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(RECVDSLOTNUMBER))
//            );
#ifndef __LIVE_STREAMING__
            nCurrentAudioPacketType = ReceivingHeader->GetInformation(PACKETTYPE);

//            ALOG("#V#TYPE# Type: "+ m_Tools.IntegertoStringConvert(nCurrentAudioPacketType));

			if (!ReceivingHeader->IsPacketTypeSupported())
			{
				continue;
			}

            if( AUDIO_SKIP_PACKET_TYPE == nCurrentAudioPacketType)
            {
                ALOG("#V#TYPE# ############################################### SKIPPET");
                toolsObject.SOSleep(0);
                continue;
            }
			else if (AUDIO_NOVIDEO_PACKET_TYPE == nCurrentAudioPacketType)
			{
				//g_StopVideoSending = 1;*/
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO, 0, 0);
                bIsProcessablePacket = true;
			}
            else if(AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType)
            {
                bIsProcessablePacket = true;
            }

            if(!bIsProcessablePacket) continue;

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
#ifdef OPUS_ENABLE
#ifndef __AUDIO_FIXED_BITRATE__
				m_pAudioCodec->DecideToChangeBitrate(m_iOpponentReceivedPackets);
#endif
#endif
			}
			
			m_iReceivedPacketsInCurrentSlot ++;
#endif
            //continue;
			nDecodingFrameSize -= m_AudioHeadersize;
//            ALOG("#ES Size: "+m_Tools.IntegertoStringConvert(nDecodingFrameSize));
#ifdef OPUS_ENABLE
            nDecodedFrameSize = m_pAudioCodec->decodeAudio(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
            ALOG("#LAD# Audio Decoded Frame Size: "+m_Tools.IntegertoStringConvert(nDecodingFrameSize));
#else
            nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame  + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
#endif
#ifdef __DUMP_FILE__
			fwrite(m_saDecodedFrame, 2, nDecodedFrameSize, FileOutput);
#endif

#ifndef __LIVE_STREAMING__
            llNow = m_Tools.CurrentTimestamp();
//            ALOG("#DS Size: "+m_Tools.IntegertoStringConvert(nDecodedFrameSize));
            if(llNow - llTimeStamp >= 1000)
            {
//                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Num AudioDataDecoded = " + m_Tools.IntegertoStringConvert(iDataSentInCurrentSec));
                iDataSentInCurrentSec = 0;
                llTimeStamp = llNow;
            }
            iDataSentInCurrentSec ++;

            ++iFrameCounter;
            nDecodingTime = m_Tools.CurrentTimestamp() - timeStamp;
            dbTotalTime += nDecodingTime;
//            if(iFrameCounter % 100 == 0)
//                ALOG( "#DE#--->> Size " + m_Tools.IntegertoStringConvert(nDecodedFrameSize) + " DecodingTime: "+ m_Tools.IntegertoStringConvert(nDecodingTime) + "A.D.Time : "+m_Tools.DoubleToString(dbTotalTime / iFrameCounter));
#endif

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
#ifdef ONLY_FOR_LIVESTREAMING
void CAudioCallSession::getAudioSendToData(unsigned char * pAudioDataToSend, int &length, std::vector<int> &vDataLengthVector)
{
    Locker lock(*m_pAudioCallSessionMutex);

    vDataLengthVector = m_vEncodedFrameLenght;
    m_vEncodedFrameLenght.clear();

    memcpy(pAudioDataToSend, m_ucaAudioDataToSend, m_iAudioDataSendIndex);

    length = m_iAudioDataSendIndex;
    m_iAudioDataSendIndex = 0;
}
#endif


