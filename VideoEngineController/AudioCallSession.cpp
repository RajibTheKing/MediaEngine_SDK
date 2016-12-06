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

#ifdef USE_ANS
#define ANS_SAMPLE_SIZE 80
#define Mild 0
#define Medium 1
#define Aggressive 2
#endif

#ifdef USE_AECM
#define AECM_SAMPLE_SIZE 80
#endif

#define __TIMESTUMP_MOD__ 100000
#ifdef USE_WEBRTC_AGC
#define AGC_SAMPLE_SIZE 80
#define AGC_ANALYSIS_SAMPLE_SIZE 80
#define AGCMODE_UNCHANGED 0
#define AGCMODE_ADAPTIVE_ANALOG 1
#define AGNMODE_ADAPTIVE_DIGITAL 2
#define AGCMODE_FIXED_DIGITAL 3
#define MINLEVEL 1
#define MAXLEVEL 255
#endif

#ifdef USE_AGC
#define MAX_GAIN 10
#define DEF_GAIN 3
#define LS_RATIO 1
#endif

#ifdef USE_VAD
#define VAD_ANALYSIS_SAMPLE_SIZE 80
#define NEXT_N_FRAMES_MAYE_VOICE 11
#endif

int gSetMode = -5;

#define __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ 0
#define __AUDIO_DELAY_TIMESTAMP_TOLERANCE__ 2

CAudioCallSession::CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, bool bIsCheckCall) :

m_pCommonElementsBucket(pSharedObject),
m_bIsCheckCall(bIsCheckCall),
m_nServiceType(nServiceType)
{
    
    m_bLiveAudioStreamRunning = false;
    
    if(m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
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

	SendingHeader = new CAudioPacketHeader(m_bLiveAudioStreamRunning);
	ReceivingHeader = new CAudioPacketHeader(m_bLiveAudioStreamRunning);
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
	m_nMaxAudioPacketNumber = ((1 << HeaderBitmap[PACKETNUMBER]) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;
	m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
    
    if(m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
    {
        m_iAudioDataSendIndex = 0;
        m_vEncodedFrameLenght.clear();
    }

	m_bUsingLoudSpeaker = false;

#ifdef USE_AGC
	m_iVolume = DEF_GAIN;
#else
	m_iVolume = 1;
#endif


#ifdef USE_AECM
	bAecmCreated = false;
	bAecmInited = false;
	m_bNoDataFromFarendYet = true;
	int iAECERR = WebRtcAecm_Create(&AECM_instance);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Create failed");
	}
	else
	{
		ALOG("WebRtcAecm_Create successful");
		bAecmCreated = true;
	}

	iAECERR = WebRtcAecm_Init(AECM_instance, AUDIO_SAMPLE_RATE);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Init failed");
	}
	else
	{
		ALOG("WebRtcAecm_Init successful");
		bAecmInited = true;
	}
#endif

#ifdef USE_ANS
	int ansret = -1;
	if ((ansret = WebRtcNs_Create(&NS_instance)))
	{
		ALOG("WebRtcNs_Create failed with error code = " + m_Tools.IntegertoStringConvert(ansret));
	}
	else
	{
		ALOG("WebRtcNs_Create successful");
	}
	if ((ansret = WebRtcNs_Init(NS_instance, AUDIO_SAMPLE_RATE)))
	{
		ALOG("WebRtcNs_Init failed with error code= " + m_Tools.IntegertoStringConvert(ansret));
	}
	else
	{
		ALOG("WebRtcNs_Init successful");
	}

	if ((ansret = WebRtcNs_set_policy(NS_instance, Medium)))
	{
		ALOG("WebRtcNs_set_policy failed with error code = " + m_Tools.IntegertoStringConvert(ansret));
	}
	else
	{
		ALOG("WebRtcNs_set_policy successful");
	}
#endif

#ifdef USE_WEBRTC_AGC

	int agcret = -1;
	if ((agcret = WebRtcAgc_Create(&AGC_instance)))
	{
		ALOG("WebRtcAgc_Create failed with error code = " + m_Tools.IntegertoStringConvert(agcret));
	}
	else
	{
		ALOG("WebRtcAgc_Create successful");
	}
	if ((agcret = WebRtcAgc_Init(AGC_instance, MINLEVEL, SHRT_MAX, AGNMODE_ADAPTIVE_DIGITAL, AUDIO_SAMPLE_RATE)))
	{
		ALOG("WebRtcAgc_Init failed with error code= " + m_Tools.IntegertoStringConvert(agcret));
	}
	else
	{
		ALOG("WebRtcAgc_Init successful");
	}
	WebRtcAgc_config_t gain_config;

	gain_config.targetLevelDbfs = 3;
	gain_config.compressionGaindB = m_iVolume * 10;
	gain_config.limiterEnable = 0;
	if ((agcret = WebRtcAgc_set_config(AGC_instance, gain_config)))
	{
		ALOG("WebRtcAgc_set_config failed with error code= " + m_Tools.IntegertoStringConvert(agcret));
	}
	else
	{
		ALOG("WebRtcAgc_Create successful");
	}
#endif

#ifdef USE_VAD
	int vadret = -1;
	if ((vadret = WebRtcVad_Create(&VAD_instance)))
	{
		ALOG("WebRtcVad_Create failed with error code = " + m_Tools.IntegertoStringConvert(vadret));
	}
	else
	{
		ALOG("WebRtcVad_Create successful");
	}

	if ((vadret = WebRtcVad_Init(VAD_instance)))
	{
		ALOG("WebRtcVad_Init failed with error code= " + m_Tools.IntegertoStringConvert(vadret));
	}
	else
	{
		ALOG("WebRtcVad_Init successful");
	}

	if ((gSetMode = vadret = WebRtcVad_set_mode(VAD_instance, 1)))
	{
		ALOG("WebRtcVad_set_mode failed with error code= " + m_Tools.IntegertoStringConvert(vadret));
	}
	else
	{
		ALOG("WebRtcVad_set_mode successful");
	}
	nNextFrameMayHaveVoice = 0;
	memset(m_saAudioBlankFrame, 0, MAX_AUDIO_FRAME_LENGHT*sizeof(short));
#endif


	m_iAudioVersionFriend = -1;
	m_iAudioVersionSelf = __AUDIO_CALL_VERSION__;


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
	WebRtcAecm_Free(AECM_instance);
#endif
#ifdef USE_ANS
	WebRtcNs_Free(NS_instance);
#endif
#ifdef USE_WEBRTC_AGC
	WebRtcAgc_Free(AGC_instance);
#endif
#ifdef USE_VAD
	WebRtcVad_Free(VAD_instance);
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
    
    
    if(m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
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

void CAudioCallSession::InitializeAudioCallSession(LongLong llFriendID, int nServiceType)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession");
    m_nServiceType  = nServiceType;

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

long long iMS = -1;
int iAudioDataCounter = 0;
long long LastFrameTime = -1;
int counter = 0;
int CAudioCallSession::EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength)
{
//	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");
	long long llCurrentTime = Tools::CurrentTimestamp();
	if(LastFrameTime !=-1)
//		__LOG("@@@@@@@@@@@@@@  #WQ NO: %d  ----*--> DIFF : %lld  Length: %d", counter, Tools::CurrentTimestamp() - LastFrameTime, (int)unLength);
	LastFrameTime = llCurrentTime;
	int returnedValue = m_AudioEncodingBuffer.Queue(psaEncodingAudioData, unLength, llCurrentTime);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");
	counter ++;
	return returnedValue;
}

void CAudioCallSession::SetVolume(int iVolume)
{
#ifdef USE_AGC
#ifdef USE_NAIVE_AGC
	if (iVolume >= 0 && iVolume <= MAX_GAIN)
	{
		m_iVolume = iVolume;
		ALOG("SetVolume called with: " + Tools::IntegertoStringConvert(iVolume));
	}
	else
	{
		m_iVolume = DEF_GAIN;
	}
	if (m_bUsingLoudSpeaker)
	{
		m_iVolume = m_iVolume * 1.0 / LS_RATIO;
	}
#else
	WebRtcAgc_config_t gain_config;

	m_iVolume = iVolume;
	gain_config.targetLevelDbfs = 3;
	gain_config.compressionGaindB = m_iVolume * 10;
	gain_config.limiterEnable = 0;
	if (WebRtcAgc_set_config(AGC_instance, gain_config))
	{
		ALOG("WebRtcAgc_set_config failed  ");
	}
	else
	{
		ALOG("WebRtcAgc_set_config successful");
	}
#endif
#endif
}

void CAudioCallSession::SetLoudSpeaker(bool bOn)
{
#ifdef USE_AGC
	if (m_bUsingLoudSpeaker != bOn)
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
	}
#endif
}

int CAudioCallSession::DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
//    ALOG("#H#Received PacketType: "+m_Tools.IntegertoStringConvert(pucaDecodingAudioData[0]));
    if(m_bLiveAudioStreamRunning)
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
	int nEncodingFrameSize, nEncodedFrameSize, encodingTime;
	long long timeStamp;
	double avgCountTimeStamp = 0;
	int countFrame = 0;
    int version = __AUDIO_CALL_VERSION__;
	int nCurrentTimeStamp;
	if(m_bLiveAudioStreamRunning)
		version = 0;
	long long llLasstTime = -1;
	int cnt = 1;
	while (m_bAudioEncodingThreadRunning)
	{
		if (m_AudioEncodingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else {
			nEncodingFrameSize = m_AudioEncodingBuffer.DeQueue(m_saAudioEncodingFrame, timeStamp);

//			m_saAudioEncodingFrame[0] = 1000;
//			m_saAudioEncodingFrame[nEncodingFrameSize - 1] = 2000;

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
			if(!m_bLiveAudioStreamRunning)
			{
#ifdef USE_VAD
				if (WebRtcVad_ValidRateAndFrameLength(AUDIO_SAMPLE_RATE, VAD_ANALYSIS_SAMPLE_SIZE) == 0)
				{
					long long vadtimeStamp = m_Tools.CurrentTimestamp();
					int nhasVoice = 0;
					for (int i = 0; i < nEncodingFrameSize; i += VAD_ANALYSIS_SAMPLE_SIZE)
					{
						int iVadRet = WebRtcVad_Process(VAD_instance, AUDIO_SAMPLE_RATE, m_saAudioEncodingFrame + i, VAD_ANALYSIS_SAMPLE_SIZE);
						if (iVadRet != 1)
						{
							ALOG("No voice found " + Tools::IntegertoStringConvert(iVadRet) + " setmode = " + Tools::IntegertoStringConvert(gSetMode));
							//memset(m_saAudioEncodingFrame + i, 0, VAD_ANALYSIS_SAMPLE_SIZE * sizeof(short));
						}
						else
						{
							ALOG("voice found " + Tools::IntegertoStringConvert(iVadRet));
							nhasVoice = 1;
							nNextFrameMayHaveVoice = NEXT_N_FRAMES_MAYE_VOICE;
						}
					}
					if (!nhasVoice)
					{
						if (nNextFrameMayHaveVoice > 0)
						{
							nNextFrameMayHaveVoice--;
						}
					}
					ALOG(" vad time = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - vadtimeStamp));
					if (!nhasVoice && !nNextFrameMayHaveVoice)
					{
						ALOG("not sending audio");
						m_Tools.SOSleep(70);
						continue;
					}
					else
					{
						ALOG("sending audio");
					}
				}
				else
				{
					ALOG("Invalid combo");
				}
#endif
#if defined(USE_AECM) || defined(USE_ANS)
				memcpy(m_saAudioEncodingTempFrame, m_saAudioEncodingFrame, nEncodingFrameSize * sizeof(short));
#endif
#ifdef USE_WEBRTC_AGC
				for (int i = 0; i < nEncodingFrameSize; i += AGC_SAMPLE_SIZE) {
					if (0 != WebRtcAgc_AddFarend(AGC_instance, m_saAudioEncodingFrame + i,
												 AGC_SAMPLE_SIZE)) { ALOG("WebRtcAgc_AddFarend failed");
					}
				}
#endif
#ifdef USE_ANS
				long long llNow = m_Tools.CurrentTimestamp();
				for (int i = 0; i < AUDIO_CLIENT_SAMPLE_SIZE; i += ANS_SAMPLE_SIZE)
				{
					if (0 != WebRtcNs_Process(NS_instance, m_saAudioEncodingTempFrame + i, NULL, m_saAudioEncodingDenoisedFrame + i, NULL))
					{
						ALOG("WebRtcNs_Process failed");
					}
				}
				if (memcmp(m_saAudioEncodingTempFrame, m_saAudioEncodingDenoisedFrame, nEncodingFrameSize * sizeof(short)) == 0)
				{
					ALOG("WebRtcNs_Process did nothing but took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
				}
				else
				{
					ALOG("WebRtcNs_Process tried to do something, believe me :-(. It took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
				}
#ifdef USE_AECM
				if (m_bNoDataFromFarendYet)
				{
					memcpy(m_saAudioEncodingFrame, m_saAudioEncodingDenoisedFrame, AUDIO_CLIENT_SAMPLE_SIZE);
				}
#else
				memcpy(m_saAudioEncodingFrame, m_saAudioEncodingDenoisedFrame, AUDIO_CLIENT_SAMPLE_SIZE);
#endif


#endif

#ifdef USE_AECM
				if (!m_bNoDataFromFarendYet)
				{
					long long llNow = m_Tools.CurrentTimestamp();
					for (int i = 0; i < AUDIO_CLIENT_SAMPLE_SIZE; i += AECM_SAMPLE_SIZE)
					{
#ifdef USE_ANS
						if (0 != WebRtcAecm_Process(AECM_instance, m_saAudioEncodingTempFrame + i, m_saAudioEncodingDenoisedFrame + i, m_saAudioEncodingFrame + i, AECM_SAMPLE_SIZE, 0))
#else
						if (0 != WebRtcAecm_Process(AECM_instance, m_saAudioEncodingTempFrame + i, NULL, m_saAudioEncodingFrame + i, AECM_SAMPLE_SIZE, 0))
#endif
						{
							ALOG("WebRtcAec_Process failed bAecmCreated = " + m_Tools.IntegertoStringConvert((int)bAecmCreated) + " bAecmInited = " + m_Tools.IntegertoStringConvert((int)bAecmInited));
						}
					}

					if (memcmp(m_saAudioEncodingTempFrame, m_saAudioEncodingFrame, nEncodingFrameSize * sizeof(short)) == 0)
					{
						ALOG("WebRtcAec_Process did nothing but took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
					}
					else
					{
						ALOG("WebRtcAec_Process tried to do something, believe me :-( . It took " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llNow));
					}
				}
#endif
			}

/*
 * ONLY FOR CALL END
 * */

#ifdef OPUS_ENABLE

			if(m_bLiveAudioStreamRunning == false)
			{
            	nEncodedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);
				ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize)+  " PacketNumber = "+m_Tools.IntegertoStringConvert(m_iPacketNumber));
                encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
                m_pAudioCodec->DecideToChangeComplexity(encodingTime);
            }
			else
			{
				nEncodedFrameSize = nEncodingFrameSize * sizeof(short);
				memcpy(&m_ucaEncodedFrame[1 + m_AudioHeadersize], m_saAudioEncodingFrame, nEncodedFrameSize);
			}

			avgCountTimeStamp += encodingTime;

			if(!m_bLiveAudioStreamRunning) {
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

			if(m_bLiveAudioStreamRunning)
				SendingHeader->SetInformation(AUDIO_NORMAL_PACKET_TYPE, PACKETTYPE);
			else
				SendingHeader->SetInformation(m_iNextPacketType, PACKETTYPE);

			if (m_iNextPacketType == AUDIO_NOVIDEO_PACKET_TYPE)
			{
				m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
			}

			//			SendingHeader->SetInformation(m_iPacketNumber + (m_iPacketNumber<30?400:0), PACKETNUMBER);
			int nCurrentPacketNumber = m_iPacketNumber;
			int tempVal;
			SendingHeader->SetInformation(m_iPacketNumber, PACKETNUMBER);
			SendingHeader->SetInformation(m_iSlotID, SLOTNUMBER);
			SendingHeader->SetInformation(nEncodedFrameSize, PACKETLENGTH);
			SendingHeader->SetInformation(m_iPrevRecvdSlotID, RECVDSLOTNUMBER);
			SendingHeader->SetInformation(m_iReceivedPacketsInPrevSlot, NUMPACKETRECVD);
			SendingHeader->SetInformation(version, VERSIONCODE);
			if (m_bLiveAudioStreamRunning)
			{
				SendingHeader->SetInformation(nCurrentTimeStamp, TIMESTAMP);
			}
			SendingHeader->GetHeaderInByteArray(&m_ucaEncodedFrame[1]);

			//            SendingHeader->CopyHeaderToInformation(&m_ucaEncodedFrame[1]);
			//
			//            int version = SendingHeader->GetInformation(VERSIONCODE);
			//            ALOG( "#2AE#--->> PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber)
			//                  +" m_iAudioVersionSelf = "+ m_Tools.IntegertoStringConvert(version));

            m_ucaEncodedFrame[0] = 0;   //Setting Audio packet type( = 0).

//            ALOG("#V# E: PacketNumber: "+m_Tools.IntegertoStringConvert(m_iPacketNumber)
//                + " #V# E: SLOTNUMBER: "+m_Tools.IntegertoStringConvert(m_iSlotID)

			//            if(m_iPacketNumber%100 ==0)
			//                ALOG("#2AE#  ------------------------ Version: "+Tools::IntegertoStringConvert(m_iAudioVersionFriend) +" isVideo: "+Tools::IntegertoStringConvert(m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning()));

			if (m_iPacketNumber >= m_nMaxAudioPacketNumber)
				m_iPacketNumber = 0;
			else
				++m_iPacketNumber;

			//            ALOG("#DE#--->> QUEUE = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize + m_AudioHeadersize + 1));
			//            CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "#DE#--->> QUEUE = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize + m_AudioHeadersize + 1));

#ifdef  __AUDIO_SELF_CALL__
            if(m_bLiveAudioStreamRunning == false)
            {
				ALOG("#A#EN#--->> Self#  PacketNumber = "+m_Tools.IntegertoStringConvert(m_iPacketNumber));
                DecodeAudioData(0, m_ucaEncodedFrame + 1, nEncodedFrameSize + m_AudioHeadersize);
                continue;
            }
#endif
            if (m_bIsCheckCall == LIVE_CALL_MOOD)
            {
//                ALOG("#H#Sent PacketType: "+m_Tools.IntegertoStringConvert(m_ucaEncodedFrame[0]));
                if(m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
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

#ifdef  __DUPLICATE_AUDIO__
					if (false == m_bLiveAudioStreamRunning && 0 < m_iAudioVersionFriend && m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning())
					{
						toolsObject.SOSleep(5);
						m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1, 0);
					//                    ALOG("#2AE# Sent Second Times");
					}
#endif
                }
                    
            }
//			else
//				DecodeAudioData(m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);

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
	int nTolarance = m_nMaxAudioPacketNumber / 2;
	long long llNow = 0;
	long long llExpectedEncodingTimeStamp = 0, llWaitingTime = 0;
	long long llLastDecodedTime = -1;
	long long llLastDecodedFrameEncodedTimeStamp = -1;
	long long llFirstSentTime = -1;
	long long llFirstSentFrame = -1;

#ifdef __DUMP_FILE__
	FileOutput = fopen("/storage/emulated/0/OutputPCMN.pcm", "w");
#endif

	//toolsObject.SOSleep(1000);
	long long llLastTime  = -1, llDiffTimeNow = -1;
	long long iTimeStampOffset = 0;
	int TempOffset = 0;

	while (m_bAudioDecodingThreadRunning)
	{
		if ( m_bLiveAudioStreamRunning && m_pLiveAudioDecodingQueue->GetQueueSize() == 0 )
        {
            toolsObject.SOSleep(1);
        }
        else if ( false == m_bLiveAudioStreamRunning && m_AudioDecodingBuffer.GetQueueSize() == 0)
        {
            toolsObject.SOSleep(10);
        }
		else
		{
            if(m_bLiveAudioStreamRunning)
                nDecodingFrameSize = m_pLiveAudioDecodingQueue->DeQueue(m_ucaDecodingFrame);
            else
                nDecodingFrameSize = m_AudioDecodingBuffer.DeQueue(m_ucaDecodingFrame);

			bIsProcessablePacket = false;
			//            ALOG( "#DE#--->> nDecodingFrameSize = " + m_Tools.IntegertoStringConvert(nDecodingFrameSize));
			timeStamp = m_Tools.CurrentTimestamp();
			ReceivingHeader->CopyHeaderToInformation(m_ucaDecodingFrame);
			if (m_bLiveAudioStreamRunning)
			{
				iTimeStampOffset = ReceivingHeader->GetInformation(TIMESTAMP);
			}
			//            ALOG("#V# PacketNumber: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(PACKETNUMBER))
			//                    + " #V# SLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(SLOTNUMBER))
			//                    + " #V# NUMPACKETRECVD: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(NUMPACKETRECVD))
			//                    + " #V# RECVDSLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(RECVDSLOTNUMBER))
			//            );

			nCurrentAudioPacketType = ReceivingHeader->GetInformation(PACKETTYPE);
			iPacketNumber = ReceivingHeader->GetInformation(PACKETNUMBER);

			ALOG("#2A#RCV---> PacketNumber = " + m_Tools.IntegertoStringConvert(iPacketNumber)
				 + "  Last: " + m_Tools.IntegertoStringConvert(m_iLastDecodedPacketNumber)
				 + " m_iAudioVersionFriend = " + m_Tools.IntegertoStringConvert(m_iAudioVersionFriend));

//			__LOG("@@@@@@@@@@@@@@ PN: %d, Len: %d",iPacketNumber, ReceivingHeader->GetInformation(PACKETLENGTH));

#ifdef  __DUPLICATE_AUDIO__
			//iPacketNumber rotates
			if (m_iLastDecodedPacketNumber > -1) {
				if (iPacketNumber <= m_iLastDecodedPacketNumber && iPacketNumber + nTolarance > m_iLastDecodedPacketNumber) {
					continue;
				}
				else if (iPacketNumber > m_iLastDecodedPacketNumber && iPacketNumber > nTolarance + m_iLastDecodedPacketNumber) {
					continue;
				}
			}
			ALOG("#2A#RCV---------> Decoding = " + m_Tools.IntegertoStringConvert(iPacketNumber));
#endif


			if (!ReceivingHeader->IsPacketTypeSupported())
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
				if(m_nServiceType == SERVICE_TYPE_CALL || m_nServiceType == SERVICE_TYPE_SELF_CALL)
					m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO, 0, 0);
				bIsProcessablePacket = true;
			}
			else if (AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType)
			{
				bIsProcessablePacket = true;
#ifdef  __DUPLICATE_AUDIO__
				if (-1 == m_iAudioVersionFriend )
					m_iAudioVersionFriend = ReceivingHeader->GetInformation(VERSIONCODE);
				//                ALOG("#2A   m_iAudioVersionFriend = "+ m_Tools.IntegertoStringConvert(m_iAudioVersionFriend));
#endif
			}

			if (!bIsProcessablePacket) continue;

			m_iOpponentReceivedPackets = ReceivingHeader->GetInformation(NUMPACKETRECVD);

			if (m_bLiveAudioStreamRunning)
			{
				if (-1 == m_llDecodingTimeStampOffset)
				{
					m_Tools.SOSleep(__LIVE_FIRST_FRAME_SLEEP_TIME__);
					m_llDecodingTimeStampOffset = m_Tools.CurrentTimestamp() - iTimeStampOffset;
				}
				else
				{
					llNow = m_Tools.CurrentTimestamp();
					llExpectedEncodingTimeStamp = llNow - m_llDecodingTimeStampOffset;
					llWaitingTime = iTimeStampOffset - llExpectedEncodingTimeStamp;

//					if( iTimeStampOffset > 100 + llLastDecodedFrameEncodedTimeStamp)
//					{
//						TempOffset = iTimeStampOffset - 98 - llLastDecodedFrameEncodedTimeStamp;
//					}
//					else
//						TempOffset = 0;

//					iTimeStampOffset -= TempOffset;

					if( llExpectedEncodingTimeStamp -  __AUDIO_DELAY_TIMESTAMP_TOLERANCE__> iTimeStampOffset ) {
						__LOG("@@@@@@@@@@@@@@@@@--> New*********************************************** FrameNumber: %d [%lld]\t\tDELAY FRAME: %lld  Now: %lld", iPacketNumber, iTimeStampOffset, llWaitingTime, llNow % __TIMESTUMP_MOD__);
						continue;
					}


					while (llExpectedEncodingTimeStamp + __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ < iTimeStampOffset)
					{
						m_Tools.SOSleep(1);
						llExpectedEncodingTimeStamp = m_Tools.CurrentTimestamp() - m_llDecodingTimeStampOffset;
					}
				}
			}

			++ iPlaiedFrameCounter;

			llNow = m_Tools.CurrentTimestamp();

			__LOG("#!@@@@@@@@@@@@@@@@@--> #W FrameNumber: %d [%lld]\t\tAudio Waiting Time: %lld  Now: %lld  DIF: %lld[%lld]", iPacketNumber, iTimeStampOffset, llWaitingTime, llNow % __TIMESTUMP_MOD__, iTimeStampOffset - llLastDecodedFrameEncodedTimeStamp, llNow - llLastDecodedTime);

			llLastDecodedTime = llNow;
			llLastDecodedFrameEncodedTimeStamp = iTimeStampOffset + TempOffset;

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
				if(m_nServiceType == SERVICE_TYPE_CALL || m_nServiceType == SERVICE_TYPE_SELF_CALL) {
					m_pAudioCodec->DecideToChangeBitrate(m_iOpponentReceivedPackets);
				}
#endif
			}
                
			m_iLastDecodedPacketNumber = iPacketNumber;
			m_iReceivedPacketsInCurrentSlot ++;

			//continue;
			nDecodingFrameSize -= m_AudioHeadersize;
			//            ALOG("#ES Size: "+m_Tools.IntegertoStringConvert(nDecodingFrameSize));

/*
 * Start call block.
 *
 */

			if(!m_bLiveAudioStreamRunning)
			{
#ifdef OPUS_ENABLE
			nDecodedFrameSize = m_pAudioCodec->decodeAudio(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
			ALOG("#A#DE#--->> Self#  PacketNumber = "+m_Tools.IntegertoStringConvert(iPacketNumber));

#else
			nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
#endif

#ifdef USE_AECM
                for (int i = 0; i < nDecodedFrameSize; i += AECM_SAMPLE_SIZE)
                {
                    if (0 != WebRtcAecm_BufferFarend(AECM_instance, m_saDecodedFrame + i, AECM_SAMPLE_SIZE))
                    {
                        ALOG("WebRtcAec_BufferFarend failed");
                    }
                }
#endif
#ifdef USE_WEBRTC_AGC

				uint8_t saturationWarning;
				int32_t inMicLevel = 1;
				int32_t outMicLevel;
				for (int i = 0; i < AUDIO_CLIENT_SAMPLE_SIZE; i += AGC_ANALYSIS_SAMPLE_SIZE) {
					if (0 != WebRtcAgc_AddMic(AGC_instance, m_saDecodedFrame + i, 0,
											  AGC_SAMPLE_SIZE)) { ALOG("WebRtcAgc_AddMic failed");
					}
					if (0 !=
						WebRtcAgc_VirtualMic(AGC_instance, m_saDecodedFrame + i, 0, AGC_SAMPLE_SIZE,
											 inMicLevel, &outMicLevel)) { ALOG(
								"WebRtcAgc_AddMic failed");
					}
					if (0 !=
						WebRtcAgc_Process(AGC_instance, m_saDecodedFrame + i, 0, AGC_SAMPLE_SIZE,
										  m_saAudioDecodedFrameTemp + i, 0,
										  inMicLevel, &outMicLevel, 0, &saturationWarning)) { ALOG(
								"WebRtcAgc_Process failed");
					}
				}
				if (memcmp(m_saDecodedFrame, m_saAudioDecodedFrameTemp,
						   nDecodedFrameSize * sizeof(short)) == 0) { ALOG(
							"WebRtcAgc_Process did nothing");
				}
				else { ALOG("WebRtcAgc_Process tried to do something, believe me :-( . Outputmic =  "
							+ m_Tools.IntegertoStringConvert(outMicLevel) +
							" saturationWarning = " +
							m_Tools.IntegertoStringConvert(saturationWarning));
				}

				int k = 1;
				double iRatio = 0;
				for (int i = 0; i < AUDIO_CLIENT_SAMPLE_SIZE; i++) {
					if (m_saDecodedFrame[i]) {
						//ALOG("ratio = " + m_Tools.IntegertoStringConvert(m_saAudioDecodedFrameTemp[i] / m_saAudioEncodingFrame[i]));
						iRatio += m_saAudioDecodedFrameTemp[i] * 1.0 / m_saDecodedFrame[i];
						k++;
					}
				}ALOG("ratio = " + m_Tools.DoubleToString(iRatio / k));

				memcpy(m_saDecodedFrame, m_saAudioDecodedFrameTemp,
					   AUDIO_CLIENT_SAMPLE_SIZE * sizeof(short));


#elif defined(USE_NAIVE_AGC)

                for (int i = 0; i < AUDIO_CLIENT_SAMPLE_SIZE; i++)
                {
                    int temp = (int)m_saDecodedFrame[i] * m_iVolume;
                    if (temp > SHRT_MAX)
                    {
                        temp = SHRT_MAX;
                    }
                    if (temp < SHRT_MIN)
                    {
                        temp = SHRT_MIN;
                    }
                    m_saDecodedFrame[i] = temp;

                }

#endif
			}
			else {
				memcpy(m_saDecodedFrame, m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize);
				nDecodedFrameSize = nDecodingFrameSize / sizeof(short);
			}
/*
 * Start call block end.
 *
 */
			m_bNoDataFromFarendYet = false;

#ifdef __DUMP_FILE__
			fwrite(m_saDecodedFrame, 2, nDecodedFrameSize, FileOutput);
#endif

            if(m_nServiceType == SERVICE_TYPE_CALL || m_nServiceType == SERVICE_TYPE_SELF_CALL)
            {
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
            }


#if defined(DUMP_DECODED_AUDIO)
			m_Tools.WriteToFile(m_saDecodedFrame, size);
#endif
            if(nDecodedFrameSize < 1)
			{
				ALOG("#EXP# Decoding Failed.");
				continue;
			}

			if (m_bLiveAudioStreamRunning == true) {

				llNow = Tools::CurrentTimestamp();

//				if(llFirstSentTime ==-1)
//				{
//					llFirstSentTime = llNow;
//					llFirstSentFrame = iPacketNumber;
//				}
//				else
//				{
//					int FrameDiff = iPacketNumber - llFirstSentFrame;
//					long long TimeDiff = llNow - llFirstSentTime;
//					long long ExpectedSentTime = FrameDiff * 100 + llFirstSentTime;
//					long long SleepTime = ExpectedSentTime - llNow - 1;
//					toolsObject.SOSleep(SleepTime);
//				}
//				llNow = Tools::CurrentTimestamp();

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
