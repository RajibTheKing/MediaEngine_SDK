#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"

//#define __AUDIO_SELF_CALL__
//#define FIRE_ENC_TIME

#ifdef USE_AECM
#include "Echo.h"
#endif
#ifdef USE_ANS
#include "Noise.h"
#endif
#ifdef USE_AGC
#include "Gain.h"
#endif
#ifdef USE_VAD
#include "Voice.h"
#endif


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define OPUS_ENABLE
//#define __DUMP_FILE__

#ifdef __DUMP_FILE__
FILE *FileInput;
FILE *FileOutput;
#endif


//#define USE_ECHO2

#define __TIMESTUMP_MOD__ 100000

#define __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ 2
#define __AUDIO_DELAY_TIMESTAMP_TOLERANCE__ 25



CAudioCallSession::CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, bool bIsCheckCall) :
m_pCommonElementsBucket(pSharedObject),
m_bIsCheckCall(bIsCheckCall),
m_nServiceType(nServiceType)
{

	m_bLiveAudioStreamRunning = false;

	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
	{
		m_bLiveAudioStreamRunning = true;
		m_pLiveAudioDecodingQueue = new LiveAudioDecodingQueue();
		m_pLiveReceiverAudio = new LiveReceiver(m_pCommonElementsBucket);
		m_pLiveReceiverAudio->SetAudioDecodingQueue(m_pLiveAudioDecodingQueue);
	}


	m_pAudioCallSessionMutex.reset(new CLockHandler);
	m_FriendID = llFriendID;

	StartEncodingThread();
	StartDecodingThread();

	SendingHeader = new CAudioPacketHeader();
	ReceivingHeader = new CAudioPacketHeader();

	m_AudioHeadersize = SendingHeader->GetHeaderSize();

	m_llEncodingTimeStampOffset = m_Tools.CurrentTimestamp();
	m_llDecodingTimeStampOffset = -1;
	m_iPacketNumber = 0;
	m_iLastDecodedPacketNumber = -1;
	m_iSlotID = 0;
	m_iPrevRecvdSlotID = -1;
	m_iCurrentRecvdSlotID = -1;
	m_iOpponentReceivedPackets = AUDIO_SLOT_SIZE;
	m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;
	m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;

	m_llMaxAudioPacketNumber = ((1LL << HeaderBitmap[PACKETNUMBER]) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;

    if(m_bLiveAudioStreamRunning)
	{
		m_iAudioDataSendIndex = 0;
		m_vEncodedFrameLenght.clear();
	}

	m_bUsingLoudSpeaker = false;
	m_bEchoCancellerEnabled = false;

	m_bLoudSpeakerEnabled = false;

#ifdef USE_AECM
	m_pEcho = new CEcho(66);
#ifdef USE_ECHO2
	m_pEcho2 = new CEcho(77);
#endif
#endif

#ifdef USE_ANS
	m_pNoise = new CNoise();
#endif

#ifdef USE_AGC
	m_pRecorderGain = new CGain();
	m_pPlayerGain = new CGain();
#endif

#ifdef USE_VAD
	m_pVoice = new CVoice();
#endif


	m_iAudioVersionFriend = -1;
	if(m_bLiveAudioStreamRunning)
	{
		m_iAudioVersionSelf = __AUDIO_LIVE_VERSION__;
	}
	else {
		m_iAudioVersionSelf = __AUDIO_CALL_VERSION__;
	}

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
#ifdef USE_AECM
	delete m_pEcho;
#ifdef USE_ECHO2
	delete m_pEcho2;
#endif
#endif
#ifdef USE_ANS
	delete m_pNoise;
#endif
#ifdef USE_AGC
	delete m_pRecorderGain;
	delete m_pPlayerGain;
#endif
#ifdef USE_VAD
	delete m_pVoice;
#endif



    
    if(m_bLiveAudioStreamRunning)
    {
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
    }
    
    
    
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
	this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket, this, llFriendID);
	m_pAudioCodec->CreateAudioEncoder();
#else
	m_pG729CodecNative = new G729CodecNative();
	int iRet = m_pG729CodecNative->Open();
#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession session initialized, iRet = " + m_Tools.IntegertoStringConvert(iRet));

}


void CAudioCallSession::SetEchoCanceller(bool bOn)
{
#ifdef USE_AECM
	m_bEchoCancellerEnabled = bOn;
#endif
}

int CAudioCallSession::EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength)
{
	//	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");
	long long llCurrentTime = Tools::CurrentTimestamp();
	
#ifdef USE_AECM
	if (!m_bLiveAudioStreamRunning && m_bEchoCancellerEnabled)
	{
#ifdef USE_ECHO2
		m_pEcho2->AddFarEnd(psaEncodingAudioData, unLength);
#endif
		m_pEcho->CancelEcho(psaEncodingAudioData, unLength);
	}
#endif

	int returnedValue = m_AudioEncodingBuffer.Queue(psaEncodingAudioData, unLength, llCurrentTime);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");

	return returnedValue;
}

int CAudioCallSession::CancelAudioData(short *psaPlayingAudioData, unsigned int unLength)
{
#ifdef USE_AECM
	if (!m_bLiveAudioStreamRunning && m_bEchoCancellerEnabled)
	{
#ifdef USE_ECHO2
		m_pEcho2->CancelEcho(psaPlayingAudioData, unLength);
#endif
		m_pEcho->AddFarEnd(psaPlayingAudioData, unLength, m_bLoudSpeakerEnabled);
	}
#endif
	return true;
}

void CAudioCallSession::SetVolume(int iVolume, bool bRecorder)
{
#ifdef USE_AGC
	if (!m_bLiveAudioStreamRunning)
	{
		if (bRecorder)
		{
			m_pRecorderGain->SetGain(iVolume);
		}
		else
		{
			m_pPlayerGain->SetGain(iVolume);
		}
	}
#endif
}

void CAudioCallSession::SetLoudSpeaker(bool bOn)
{
#ifdef USE_AGC
	/*if (m_bUsingLoudSpeaker != bOn)
	{
	m_bUsingLoudSpeaker = bOn;
	if (bOn)
	{
	m_iVolume = m_iVolume * 1.0 / LS_RATIO;
	}
	else
	{
	m_iVolume *= LS_RATIO;
	}
	}*/
	//This method may be used in future.
#endif
	m_bUsingLoudSpeaker = bOn;
	/*
	#ifdef USE_AECM
	delete m_pEcho;
	m_pEcho = new CEcho(66);
	#ifdef USE_ECHO2
	delete m_pEcho2;
	m_pEcho2 = new CEcho(77);
	#endif
	#endif*/
}

int CAudioCallSession::DecodeAudioDataVector(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	//    ALOG("#H#Received PacketType: "+m_Tools.IntegertoStringConvert(pucaDecodingAudioData[0]));
	if (m_bLiveAudioStreamRunning)
	{
		m_pLiveReceiverAudio->ProcessAudioStreamVector(nOffset, pucaDecodingAudioData, unLength, frameSizes, numberOfFrames, vMissingFrames);

		return 1;
	}

	int returnedValue = m_AudioDecodingBuffer.Queue(pucaDecodingAudioData, unLength);

	return returnedValue;
}


int CAudioCallSession::DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
	//    ALOG("#H#Received PacketType: "+m_Tools.IntegertoStringConvert(pucaDecodingAudioData[0]));
	if (m_bLiveAudioStreamRunning)
	{
		m_pLiveReceiverAudio->ProcessAudioStream(nOffset, pucaDecodingAudioData, unLength, frameSizes, numberOfFrames, missingFrames, numberOfMissingFrames);

		return 1;
	}

	int returnedValue = m_AudioDecodingBuffer.Queue(pucaDecodingAudioData, unLength);

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

	dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
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
	int nEncodingFrameSize, nEncodedFrameSize;

	long long encodingTime = 0;
	long long timeStamp;
	double avgCountTimeStamp = 0;
	int countFrame = 0;
    int version = 0;
	long long nCurrentTimeStamp;

	long long llLasstTime = -1;
	int cnt = 1;
	while (m_bAudioEncodingThreadRunning)
	{
		if (m_AudioEncodingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else {
			nEncodingFrameSize = m_AudioEncodingBuffer.DeQueue(m_saAudioEncodingFrame, timeStamp);

			if (nEncodingFrameSize % AUDIO_FRAME_SIZE > 0) {
				ALOG("#EXP# Client Sample Size not multiple of AUDIO-FRAME-SIZE = " + Tools::IntegertoStringConvert(nEncodingFrameSize));
			}
#ifdef __DUMP_FILE__
			fwrite(m_saAudioEncodingFrame, 2, nEncodingFrameSize, FileInput);
#endif
			int nEncodedFrameSize;

			__LOG("#WQ Relative Time Counter: %d-------------------------------- NO: %lld\n",cnt, timeStamp - llLasstTime);
			cnt++;
			llLasstTime = timeStamp;
			countFrame++;
			nCurrentTimeStamp = timeStamp - m_llEncodingTimeStampOffset;

			/*
			* ONLY FOR CALL
			* */
			if (!m_bLiveAudioStreamRunning)
			{
#ifdef USE_VAD			
				if (!m_pVoice->HasVoice(m_saAudioEncodingFrame, nEncodingFrameSize))
				{
					continue;
				}
#endif


#ifdef USE_AGC
				m_pPlayerGain->AddFarEnd(m_saAudioEncodingFrame, nEncodingFrameSize);
				m_pRecorderGain->AddGain(m_saAudioEncodingFrame, nEncodingFrameSize);
#endif


#ifdef USE_ANS
				memcpy(m_saAudioEncodingTempFrame, m_saAudioEncodingFrame, nEncodingFrameSize * sizeof(short));
				m_pNoise->Denoise(m_saAudioEncodingTempFrame, nEncodingFrameSize, m_saAudioEncodingDenoisedFrame);
#ifdef USE_AECM
				
				memcpy(m_saAudioEncodingFrame, m_saAudioEncodingDenoisedFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
#else
				memcpy(m_saAudioEncodingFrame, m_saAudioEncodingDenoisedFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
#endif


#endif
			}


#ifdef OPUS_ENABLE

			if (m_bLiveAudioStreamRunning == false)
			{
				nEncodedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);
				ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize) + " PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
				encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
				m_pAudioCodec->DecideToChangeComplexity(encodingTime);
			}
			else
			{
				nEncodedFrameSize = nEncodingFrameSize * sizeof(short);
				memcpy(&m_ucaEncodedFrame[1 + m_AudioHeadersize], m_saAudioEncodingFrame, nEncodedFrameSize);
			}

			avgCountTimeStamp += encodingTime;

			if (!m_bLiveAudioStreamRunning) {
#ifdef FIRE_ENC_TIME
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_ENCODING_TIME, encodingTime, 0);
				cumulitiveenctime += encodingTime;
				encodingtimetimes++;
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_AVG_ENCODING_TIME, cumulitiveenctime * 1.0 / encodingtimetimes, 0);
#endif
			}
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
			PRT("@#BUILD -----------> %d",m_iPacketNumber);
			BuildAndGetHeaderInArray(m_iNextPacketType, m_AudioHeadersize, 0, m_iSlotID, m_iPacketNumber, nEncodedFrameSize,
				m_iPrevRecvdSlotID, m_iReceivedPacketsInPrevSlot, 0, version, nCurrentTimeStamp, &m_ucaEncodedFrame[1]);

			m_ucaEncodedFrame[0] = 0;   //Setting Audio packet type( = 0).

			++m_iPacketNumber;	
			if (m_iPacketNumber == m_llMaxAudioPacketNumber)
			{
				m_iPacketNumber = 0;
			}

			if (m_iNextPacketType == AUDIO_NOVIDEO_PACKET_TYPE)
			{
				m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
			}

#ifdef  __AUDIO_SELF_CALL__
			if (m_bLiveAudioStreamRunning == false)
			{
				ALOG("#A#EN#--->> Self#  PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
				DecodeAudioData(0, m_ucaEncodedFrame + 1, nEncodedFrameSize + m_AudioHeadersize);
				continue;
			}
#endif
            if (m_bIsCheckCall == LIVE_CALL_MOOD)
            {
//                ALOG("#H#Sent PacketType: "+m_Tools.IntegertoStringConvert(m_ucaEncodedFrame[0]));
                if(m_bLiveAudioStreamRunning)
                {
                    Locker lock(*m_pAudioCallSessionMutex);
                    if((m_iAudioDataSendIndex + nEncodedFrameSize + m_AudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE )
                    {
                        
                        memcpy(m_ucaAudioDataToSend + m_iAudioDataSendIndex,  m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
                        m_iAudioDataSendIndex += (nEncodedFrameSize + m_AudioHeadersize + 1);
                        m_vEncodedFrameLenght.push_back( nEncodedFrameSize + m_AudioHeadersize + 1 );
                    }
                }
                else
                {
					m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1, 0);

					if (false == m_bLiveAudioStreamRunning && m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning())
					{
						toolsObject.SOSleep(5);
						m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1, 0);
						//                    ALOG("#2AE# Sent Second Times");
					}
                }
                    
            }

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

	dispatch_queue_t DecodeThreadQ = dispatch_queue_create("DecodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
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
	int nDecodingFrameSize, nDecodedFrameSize, iFrameCounter = 0, nCurrentAudioPacketType, iPacketNumber;
	long long timeStamp, nDecodingTime = 0;
	double dbTotalTime = 0;

	int iDataSentInCurrentSec = 0, iPlaiedFrameCounter = 0;
	long long llTimeStamp = 0;	
	long long llNow = 0;
	long long llExpectedEncodingTimeStamp = 0, llWaitingTime = 0;
	long long llLastDecodedTime = -1;
	long long llLastDecodedFrameEncodedTimeStamp = -1;
	long long llFirstSentTime = -1;
	long long llFirstSentFrame = -1;
	int nCurrentPacketHeaderLength = 0;

#ifdef __DUMP_FILE__
	FileOutput = fopen("/storage/emulated/0/OutputPCMN.pcm", "w");
#endif

	//toolsObject.SOSleep(1000);
	long long llLastTime = -1, llDiffTimeNow = -1;
	long long llTimeStampOffset = 0;
	int TempOffset = 0;

	while (m_bAudioDecodingThreadRunning)
	{
		if (m_bLiveAudioStreamRunning && m_pLiveAudioDecodingQueue->GetQueueSize() == 0)
		{
			toolsObject.SOSleep(5);
		}
		else if (false == m_bLiveAudioStreamRunning && m_AudioDecodingBuffer.GetQueueSize() == 0)
		{
			toolsObject.SOSleep(10);
		}
		else
		{
			if (m_bLiveAudioStreamRunning)
			{
				nDecodingFrameSize = m_pLiveAudioDecodingQueue->DeQueue(m_ucaDecodingFrame);
			}
			else
			{
				nDecodingFrameSize = m_AudioDecodingBuffer.DeQueue(m_ucaDecodingFrame);
			}

			bIsProcessablePacket = false;
			//            ALOG( "#DE#--->> nDecodingFrameSize = " + m_Tools.IntegertoStringConvert(nDecodingFrameSize));
			timeStamp = m_Tools.CurrentTimestamp();

			int dummy;
			int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
				nChannel, nVersion, llTimeStampOffset, m_ucaDecodingFrame);
			
			//ReceivingHeader->CopyHeaderToInformation(m_ucaDecodingFrame);
			//llTimeStampOffset = ReceivingHeader->GetInformation(TIMESTAMP);
			//            ALOG("#V# PacketNumber: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(PACKETNUMBER))
			//                    + " #V# SLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(SLOTNUMBER))
			//                    + " #V# NUMPACKETRECVD: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(NUMPACKETRECVD))
			//                    + " #V# RECVDSLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(RECVDSLOTNUMBER))
			//            );

			//nCurrentAudioPacketType = ReceivingHeader->GetInformation(PACKETTYPE);
			//iPacketNumber = ReceivingHeader->GetInformation(PACKETNUMBER);

			ALOG("#2A#RCV---> PacketNumber = " + m_Tools.IntegertoStringConvert(iPacketNumber)
				+ "  Last: " + m_Tools.IntegertoStringConvert(m_iLastDecodedPacketNumber)
				+ " m_iAudioVersionFriend = " + m_Tools.IntegertoStringConvert(m_iAudioVersionFriend));

			//			__LOG("@@@@@@@@@@@@@@ PN: %d, Len: %d",iPacketNumber, ReceivingHeader->GetInformation(PACKETLENGTH));


			if (false == m_bLiveAudioStreamRunning && m_iLastDecodedPacketNumber >= iPacketNumber) {
				PRT("@@@@########Skipped Packet: %d",iPacketNumber);
				continue;								
			}

			ALOG("#2A#RCV---------> Decoding = " + m_Tools.IntegertoStringConvert(iPacketNumber));

			if (!ReceivingHeader->IsPacketTypeSupported(nCurrentAudioPacketType))
			{
				continue;
			}

			if (AUDIO_SKIP_PACKET_TYPE == nCurrentAudioPacketType)
			{
				//                ALOG("#V#TYPE# ############################################### SKIPPET");
				toolsObject.SOSleep(0);
				continue;
			}
			else if (AUDIO_NOVIDEO_PACKET_TYPE == nCurrentAudioPacketType)
			{
				//g_StopVideoSending = 1;*/
				if(false == m_bLiveAudioStreamRunning)
					m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO, 0, 0);
				bIsProcessablePacket = true;
			}
			else if (AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType)
			{
				bIsProcessablePacket = true;

				if (false == m_bLiveAudioStreamRunning) {
					m_iAudioVersionFriend = nVersion;//ReceivingHeader->GetInformation(VERSIONCODE);
					//                ALOG("#2A   m_iAudioVersionFriend = "+ m_Tools.IntegertoStringConvert(m_iAudioVersionFriend));
				}

			}

			if (!bIsProcessablePacket) continue;

			//m_iOpponentReceivedPackets = ReceivingHeader->GetInformation(NUMPACKETRECVD);

			if (m_bLiveAudioStreamRunning)
			{
				if (-1 == m_llDecodingTimeStampOffset)
				{
					m_Tools.SOSleep(__LIVE_FIRST_FRAME_SLEEP_TIME__);
					m_llDecodingTimeStampOffset = m_Tools.CurrentTimestamp() - llTimeStampOffset;
				}
				else
				{
					llNow = m_Tools.CurrentTimestamp();
					llExpectedEncodingTimeStamp = llNow - m_llDecodingTimeStampOffset;
					llWaitingTime = llTimeStampOffset - llExpectedEncodingTimeStamp;

					if( llExpectedEncodingTimeStamp -  __AUDIO_DELAY_TIMESTAMP_TOLERANCE__> llTimeStampOffset ) {
						LOGE("##DE##@@@@@@@@@@@@@@@@@--> New*********************************************** FrameNumber: %d [%lld]\t\tDELAY FRAME: %lld  Now: %lld", iPacketNumber, llTimeStampOffset, llWaitingTime, llNow % __TIMESTUMP_MOD__);
						continue;
					}

					while (llExpectedEncodingTimeStamp + __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ < llTimeStampOffset)
					{
						m_Tools.SOSleep(5);
						llExpectedEncodingTimeStamp = m_Tools.CurrentTimestamp() - m_llDecodingTimeStampOffset;
					}
				}
			}

			++ iPlaiedFrameCounter;

			llNow = m_Tools.CurrentTimestamp();

			__LOG("#!@@@@@@@@@@@@@@@@@--> #W FrameNumber: %d [%lld]\t\tAudio Waiting Time: %lld  Now: %lld  DIF: %lld[%lld]", iPacketNumber, llTimeStampOffset, llWaitingTime, llNow % __TIMESTUMP_MOD__, llTimeStampOffset - llLastDecodedFrameEncodedTimeStamp, llNow - llLastDecodedTime);

			llLastDecodedTime = llNow;
			llLastDecodedFrameEncodedTimeStamp = llTimeStampOffset + TempOffset;

			if (nSlotNumber != m_iCurrentRecvdSlotID)
			{
				m_iPrevRecvdSlotID = m_iCurrentRecvdSlotID;
				if (m_iPrevRecvdSlotID != -1)
				{
					m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot;
				}

				m_iCurrentRecvdSlotID = nSlotNumber;
				m_iReceivedPacketsInCurrentSlot = 0;

#ifdef OPUS_ENABLE
				if(false == m_bLiveAudioStreamRunning) {
					m_pAudioCodec->DecideToChangeBitrate(m_iOpponentReceivedPackets);
				}
#endif
			}

			m_iLastDecodedPacketNumber = iPacketNumber;
			m_iReceivedPacketsInCurrentSlot++;

			nDecodingFrameSize -= m_AudioHeadersize;
			//            ALOG("#ES Size: "+m_Tools.IntegertoStringConvert(nDecodingFrameSize));

			/*
			* Start call block.
			*
			*/

			if (!m_bLiveAudioStreamRunning)
			{
#ifdef OPUS_ENABLE
				nDecodedFrameSize = m_pAudioCodec->decodeAudio(m_ucaDecodingFrame + nCurrentPacketHeaderLength, nPacketDataLength, m_saDecodedFrame);
				ALOG("#A#DE#--->> Self#  PacketNumber = " + m_Tools.IntegertoStringConvert(iPacketNumber));

#else
				nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
#endif


#ifdef USE_AGC
				m_pRecorderGain->AddFarEnd(m_saDecodedFrame, nDecodedFrameSize);
				m_pPlayerGain->AddGain(m_saDecodedFrame, nDecodedFrameSize);
#endif
			}
			else
			{
				memcpy(m_saDecodedFrame, m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize);
				nDecodedFrameSize = nDecodingFrameSize / sizeof(short);
			}
			/*
			* Start call block end.
			*
			*/
#ifdef __DUMP_FILE__
			fwrite(m_saDecodedFrame, 2, nDecodedFrameSize, FileOutput);
#endif

            if(!m_bLiveAudioStreamRunning)
			{
				llNow = m_Tools.CurrentTimestamp();
				//            ALOG("#DS Size: "+m_Tools.IntegertoStringConvert(nDecodedFrameSize));
				if (llNow - llTimeStamp >= 1000)
				{
					//                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Num AudioDataDecoded = " + m_Tools.IntegertoStringConvert(iDataSentInCurrentSec));
					iDataSentInCurrentSec = 0;
					llTimeStamp = llNow;
				}
				iDataSentInCurrentSec++;

				++iFrameCounter;
				nDecodingTime = m_Tools.CurrentTimestamp() - timeStamp;
				dbTotalTime += nDecodingTime;
				//            if(iFrameCounter % 100 == 0)
				//                ALOG( "#DE#--->> Size " + m_Tools.IntegertoStringConvert(nDecodedFrameSize) + " DecodingTime: "+ m_Tools.IntegertoStringConvert(nDecodingTime) + "A.D.Time : "+m_Tools.DoubleToString(dbTotalTime / iFrameCounter));
			}


#if defined(DUMP_DECODED_AUDIO)
			m_Tools.WriteToFile(m_saDecodedFrame, size);
#endif
			if (nDecodedFrameSize < 1)
			{
				ALOG("#EXP# Decoding Failed.");
				continue;
			}

			if (m_bLiveAudioStreamRunning == true) {

				llNow = Tools::CurrentTimestamp();

				__LOG("!@@@@@@@@@@@  #WQ     FN: %d -------- Receiver Time Diff : %lld    DataLenght: %d",iPacketNumber, llNow - llLastTime, nDecodedFrameSize);

				llLastTime = llNow;
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_FriendID,
																		  SERVICE_TYPE_LIVE_STREAM,
																		  nDecodedFrameSize,
																		  m_saDecodedFrame);
			}
			else
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_FriendID, SERVICE_TYPE_CALL, nDecodedFrameSize, m_saDecodedFrame);

			toolsObject.SOSleep(0);
		}
	}

	m_bAudioDecodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Stopped DecodingThreadProcedure method.");
}

void CAudioCallSession::getAudioSendToData(unsigned char * pAudioDataToSend, int &length, std::vector<int> &vDataLengthVector)
{
	Locker lock(*m_pAudioCallSessionMutex);

	vDataLengthVector = m_vEncodedFrameLenght;
	m_vEncodedFrameLenght.clear();

	memcpy(pAudioDataToSend, m_ucaAudioDataToSend, m_iAudioDataSendIndex);

	length = m_iAudioDataSendIndex;
	m_iAudioDataSendIndex = 0;
}

int CAudioCallSession::GetServiceType()
{
	return m_nServiceType;
}

void CAudioCallSession::BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
	int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header)
{
	//LOGEF("##EN### BuildAndGetHeader ptype %d ntype %d slotnumber %d packetnumber %d plength %d reslnumber %d npacrecv %d channel %d version %d time %lld",
	//	packetType, networkType, slotNumber, packetNumber, packetLength, recvSlotNumber, numPacketRecv, channel, version, timestamp);

	SendingHeader->SetInformation(AUDIO_NORMAL_PACKET_TYPE, PACKETTYPE);
	SendingHeader->SetInformation(nHeaderLength, HEADERLENGTH);
	SendingHeader->SetInformation(packetNumber, PACKETNUMBER);
	SendingHeader->SetInformation(slotNumber, SLOTNUMBER);
	SendingHeader->SetInformation(packetLength, PACKETLENGTH);
	SendingHeader->SetInformation(recvSlotNumber, RECVDSLOTNUMBER);
	SendingHeader->SetInformation(numPacketRecv, NUMPACKETRECVD);
	SendingHeader->SetInformation(version, VERSIONCODE);
	SendingHeader->SetInformation(timestamp, TIMESTAMP);
	SendingHeader->SetInformation(networkType, NETWORKTYPE);
	SendingHeader->SetInformation(channel, CHANNELS);

	SendingHeader->showDetails("@#BUILD");

	SendingHeader->GetHeaderInByteArray(header);

//	CAudioPacketHeader hi;
//	hi.CopyHeaderToInformation(header);
//	hi.showDetails("@#BUILD Two: ");
}

void CAudioCallSession::ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
	int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header)
{
	ReceivingHeader->CopyHeaderToInformation(header);

	packetType = ReceivingHeader->GetInformation(PACKETTYPE);
	nHeaderLength = ReceivingHeader->GetInformation(HEADERLENGTH);
	networkType = ReceivingHeader->GetInformation(NETWORKTYPE);
	slotNumber = ReceivingHeader->GetInformation(SLOTNUMBER);
	packetNumber = ReceivingHeader->GetInformation(PACKETNUMBER);
	packetLength = ReceivingHeader->GetInformation(PACKETLENGTH);
	recvSlotNumber = ReceivingHeader->GetInformation(RECVDSLOTNUMBER);
	numPacketRecv = ReceivingHeader->GetInformation(NUMPACKETRECVD);
	channel = ReceivingHeader->GetInformation(CHANNELS);
	version = ReceivingHeader->GetInformation(VERSIONCODE);
	timestamp = ReceivingHeader->GetInformation(TIMESTAMP);

	ReceivingHeader->showDetails("@#PARSE");
}
