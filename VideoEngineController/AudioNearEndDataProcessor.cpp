
#include "AudioNearEndDataProcessor.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioMacros.h"
#include "AudioPacketHeader.h"
#include "CommonElementsBucket.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioPacketizer.h"
#include "AudioCallSession.h"
#include "AudioMixer.h"
#ifdef USE_AGC
#include "Gain.h"
#endif

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CAudioNearEndDataProcessor::CAudioNearEndDataProcessor(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning) :
m_nServiceType(nServiceType),
m_nEntityType(nEntityType),
m_bIsReady(false),
m_llFriendID(llFriendID),
m_pAudioCallSession(pAudioCallSession),
m_pCommonElementsBucket(pCommonElementsBucket),
m_pAudioEncodingBuffer(pAudioEncodingBuffer),
m_bIsLiveStreamingRunning(bIsLiveStreamingRunning),
m_bAudioEncodingThreadRunning(false),
m_bAudioEncodingThreadClosed(true),
m_nEncodedFrameSize(0),
m_iPacketNumber(0),
m_iRawDataSendIndexViewer(0),
m_llLastChunkLastFrameRT(-1),
m_llLastFrameRT(0)
{
	m_pAudioEncodingMutex.reset(new CLockHandler);
	m_pAudioCodec = pAudioCallSession->GetAudioCodec();
	m_llMaxAudioPacketNumber = ((1LL << HeaderBitmap[INF_PACKETNUMBER]) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;

	m_pAudioPacketHeader = new CAudioPacketHeader();
	m_pAudioPacketizer = new AudioPacketizer(pAudioCallSession, pCommonElementsBucket);

	m_MyAudioHeadersize = m_pAudioPacketHeader->GetHeaderSize();
	m_llEncodingTimeStampOffset = Tools::CurrentTimestamp();

	m_pAudioMixer = new AudioMixer(BITS_USED_FOR_AUDIO_MIXING, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);

	m_bIsReady = true;

#ifdef DUMP_FILE
	m_pAudioCallSession->FileInput = fopen("/sdcard/InputPCMN.pcm", "wb");
	m_pAudioCallSession->FileInputWithEcho = fopen("/sdcard/InputPCMN_WITH_ECHO.pcm", "wb");
	m_pAudioCallSession->FileInputPreGain = fopen("/sdcard/InputPCMNPreGain.pcm", "wb");	
	m_pAudioCallSession->File18BitType = fopen("/sdcard/File18BitType.pcm", "wb");
	m_pAudioCallSession->File18BitData = fopen("/sdcard/File18BitData.pcm", "wb");
#endif	

	StartEncodingThread();
}

CAudioNearEndDataProcessor::~CAudioNearEndDataProcessor(){
	StopEncodingThread();
	if (m_pAudioPacketHeader)
	{
		delete m_pAudioPacketHeader;
	}
	if (m_pAudioPacketizer)
	{
		delete m_pAudioPacketizer;
	}

}

void CAudioNearEndDataProcessor::StartEncodingThread()
{
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

void *CAudioNearEndDataProcessor::CreateAudioEncodingThread(void* param)
{
	CAudioNearEndDataProcessor *pThis = (CAudioNearEndDataProcessor*)param;
	pThis->EncodingThreadProcedure();

	return NULL;
}

					/***Encoding*/

void CAudioNearEndDataProcessor::EncodingThreadProcedure()
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
		
	LOGT("##NF## encoder started");
	while (m_bAudioEncodingThreadRunning)
	{
		if (m_bIsLiveStreamingRunning)
		{
			if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
			{
				LiveStreamNearendProcedurePublisher();
			}
			else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
			{
				LiveStreamNearendProcedureViewer();
			}			
		}
		else 
		{
			AudioCallNearendProcedure();
		}
	}

	m_bAudioEncodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}

void CAudioNearEndDataProcessor::LiveStreamNearendProcedureViewer(){
	int version = 0;
	long long llCapturedTime, llRelativeTime = 0, llLasstTime = -1;
	if (m_pAudioEncodingBuffer->GetQueueSize() == 0)
		Tools::SOSleep(10);
	else
	{
		LOG18("#18#NE#Viewer... ");
		m_pAudioEncodingBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
		int nDataLenthInShort = AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING;

		m_pAudioCallSession->m_ViewerInCallSentDataQueue.EnQueue(m_saAudioRecorderFrame, nDataLenthInShort, m_iPacketNumber);

		LOGT("##NF## encoder got job. time:%lld", llCapturedTime);		
		DumpEncodingFrame();
		UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);		

		if (false == PreProcessAudioBeforeEncoding())
		{
			return;
		}


		m_nRawFrameSize = CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short);
		memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saAudioRecorderFrame, m_nRawFrameSize);
		
		int iSlotID = 0;
		int iPrevRecvdSlotID = 0;
		int iReceivedPacketsInPrevSlot = 0;
		int nChannel = 0;

		m_ucaRawFrameNonMuxed[0] = 0;
		BuildAndGetHeaderInArray(AUDIO_LIVE_CALLEE_PACKET_TYPE, m_MyAudioHeadersize, 0, iSlotID, m_iPacketNumber, m_nRawFrameSize,
			iPrevRecvdSlotID, iReceivedPacketsInPrevSlot, nChannel, version, llRelativeTime, &m_ucaRawFrameNonMuxed[1]);
		
		++m_iPacketNumber;
		if (m_iPacketNumber == m_llMaxAudioPacketNumber)
		{
			m_iPacketNumber = 0;
		}		
		LOG18("#18#NE#Viewer  StoreDataForChunk");
		StoreDataForChunk(m_ucaRawFrameNonMuxed, llRelativeTime, 1600);
		
		Tools::SOSleep(0);
	}
}

void CAudioNearEndDataProcessor::LiveStreamNearendProcedurePublisher(){
	int version = 0;
	long long llCapturedTime, llRelativeTime = 0, llLasstTime = -1;
	if (m_pAudioEncodingBuffer->GetQueueSize() == 0)
		Tools::SOSleep(10);
	else
	{
		LOG18("#18#NE#Publisher...");
		m_pAudioEncodingBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
		LOGT("##NF## encoder got job. time:%lld", llCapturedTime);
		DumpEncodingFrame();

		int nSendingDataSizeInByte = 1600;	//Or contain 18 bit data with mixed header.
		bool bIsMuxed = false;
		
		bIsMuxed = MuxIfNeeded(m_saAudioRecorderFrame, m_saSendingDataPublisher, nSendingDataSizeInByte, m_iPacketNumber);
				
		UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);

		if (false == PreProcessAudioBeforeEncoding())
		{
			return;
		}
				
		memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saSendingDataPublisher, nSendingDataSizeInByte);

		int nSendingFramePacketType = AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED;

		if (bIsMuxed){
			nSendingFramePacketType = AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED;
		}

		int iSlotID = 0;
		int iPrevRecvdSlotID = 0;
		int iReceivedPacketsInPrevSlot = 0;
		int nChannel = 0;
		m_ucaRawFrameNonMuxed[0] = 0;
		BuildAndGetHeaderInArray(nSendingFramePacketType, m_MyAudioHeadersize, 0, iSlotID, m_iPacketNumber, nSendingDataSizeInByte,
			iPrevRecvdSlotID, iReceivedPacketsInPrevSlot, nChannel, version, llRelativeTime, &m_ucaRawFrameNonMuxed[1]);

		++m_iPacketNumber;
		if (m_iPacketNumber == m_llMaxAudioPacketNumber)
		{
			m_iPacketNumber = 0;
		}
		LOG18("#18#NE#Publish  StoreDataForChunk nSendingDataSizeInByte = %d m_MyAudioHeadersize = %d", nSendingDataSizeInByte, m_MyAudioHeadersize);
		
		int nSendigFrameSize = nSendingDataSizeInByte + m_MyAudioHeadersize + 1;
		//for (int i = 0; i < 5; i++)
		//{
		//	m_ucaRawFrameNonMuxed[m_MyAudioHeadersize + 1 + i] = 110 + i;
		//	m_ucaRawFrameNonMuxed[nSendigFrameSize - 1 - i] = 110 + i;
		//}		

		StoreDataForChunk(m_ucaRawFrameNonMuxed, llRelativeTime, nSendigFrameSize);

		Tools::SOSleep(0);
	}
}


void CAudioNearEndDataProcessor::AudioCallNearendProcedure(){
	int version = 0;
	long long llCapturedTime, llRelativeTime = 0, llLasstTime = -1;;
	if (m_pAudioEncodingBuffer->GetQueueSize() == 0)
		Tools::SOSleep(10);
	else
	{
		LOG18("#18#NE#AudioCall...");
		m_pAudioEncodingBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
		LOGT("##NF## encoder got job. time:%lld", llCapturedTime);
		
		DumpEncodingFrame();
		UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);		

		if (false == PreProcessAudioBeforeEncoding())
		{
			return;
		}

		long long llEncodingTime, llTimeBeforeEncoding = Tools::CurrentTimestamp();
		m_nEncodedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), &m_ucaEncodedFrame[1 + m_MyAudioHeadersize]);

		//ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize) + " PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
		llEncodingTime = Tools::CurrentTimestamp() - llTimeBeforeEncoding;
		m_pAudioCodec->DecideToChangeComplexity(llEncodingTime);


		int iSlotID = 0;
		int iPrevRecvdSlotID = 0;
		int iReceivedPacketsInPrevSlot = 0;
		int nChannel = 0;

		BuildAndGetHeaderInArray(AUDIO_NORMAL_PACKET_TYPE, m_MyAudioHeadersize, 0, iSlotID, m_iPacketNumber, m_nEncodedFrameSize, iPrevRecvdSlotID, iReceivedPacketsInPrevSlot, nChannel, version, llRelativeTime, &m_ucaEncodedFrame[1]);

		++m_iPacketNumber;
		if (m_iPacketNumber == m_llMaxAudioPacketNumber)
		{
			m_iPacketNumber = 0;
		}

		SentToNetwork(llRelativeTime);		
		LOG18("#18#NE#AudioCall Sent");
		Tools::SOSleep(0);
	}
}

void CAudioNearEndDataProcessor::StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nDataLengthInByte)
{
	Locker lock(*m_pAudioEncodingMutex);

	
	if (0 == m_iRawDataSendIndexViewer && -1 == m_llLastChunkLastFrameRT)
	{
		HITLER("#RT# update lastChunkLastFrame time %lld", llRelativeTime);
		m_llLastChunkLastFrameRT = llRelativeTime;
	}

	m_llLastFrameRT = llRelativeTime;

	if ((m_iRawDataSendIndexViewer + nDataLengthInByte + m_MyAudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE)
	{
		memcpy(m_ucaRawDataToSendViewer + m_iRawDataSendIndexViewer, uchDataToChunk, nDataLengthInByte + m_MyAudioHeadersize + 1);
		m_iRawDataSendIndexViewer += (nDataLengthInByte + m_MyAudioHeadersize + 1);
		m_vRawFrameLengthViewer.push_back(nDataLengthInByte + m_MyAudioHeadersize + 1);
	}
}


void CAudioNearEndDataProcessor::SentToNetwork(long long llRelativeTime)
{
#ifdef  AUDIO_SELF_CALL //Todo: build while this is enable
	//Todo: m_AudioReceivedBuffer fix. not member of this class
	if (m_bIsLiveStreamingRunning == false)
	{
		ALOG("#A#EN#--->> Self#  PacketNumber = " + Tools::IntegertoStringConvert(m_iPacketNumber));
		m_pAudioCallSession->m_pFarEndProcessor->m_AudioReceivedBuffer.EnQueue(m_ucaEncodedFrame + 1, m_nEncodedFrameSize + m_MyAudioHeadersize);
		return;
	}
#endif

#ifndef NO_CONNECTIVITY
		m_pCommonElementsBucket->SendFunctionPointer(m_llFriendID, MEDIA_TYPE_AUDIO, m_ucaEncodedFrame, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, 0, std::vector< std::pair<int, int> >());
#else
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
#endif

#ifdef  DUPLICATE_AUDIO
		if (false == m_bIsLiveStreamingRunning && m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning())
		{
			Tools::SOSleep(5);
#ifndef NO_CONNECTIVITY
			m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, MEDIA_TYPE_AUDIO, m_ucaEncodedFrame, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, 0, std::vector< std::pair<int, int> >());
#else
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
#endif
		}
#endif
}

void CAudioNearEndDataProcessor::BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
	int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header)
{
	//LOGEF("##EN### BuildAndGetHeader ptype %d ntype %d slotnumber %d packetnumber %d plength %d reslnumber %d npacrecv %d channel %d version %d time %lld",
	//	packetType, networkType, slotNumber, packetNumber, packetLength, recvSlotNumber, numPacketRecv, channel, version, timestamp);

	m_pAudioPacketHeader->SetInformation(packetType, INF_PACKETTYPE);
	m_pAudioPacketHeader->SetInformation(nHeaderLength, INF_HEADERLENGTH);
	m_pAudioPacketHeader->SetInformation(packetNumber, INF_PACKETNUMBER);
	m_pAudioPacketHeader->SetInformation(slotNumber, INF_SLOTNUMBER);
	m_pAudioPacketHeader->SetInformation(packetLength, INF_BLOCK_LENGTH);
	m_pAudioPacketHeader->SetInformation(recvSlotNumber, INF_RECVDSLOTNUMBER);
	m_pAudioPacketHeader->SetInformation(numPacketRecv, INF_NUMPACKETRECVD);
	m_pAudioPacketHeader->SetInformation(version, INF_VERSIONCODE);
	m_pAudioPacketHeader->SetInformation(timestamp, INF_TIMESTAMP);
	m_pAudioPacketHeader->SetInformation(networkType, INF_NETWORKTYPE);
	m_pAudioPacketHeader->SetInformation(channel, INF_CHANNELS);

	m_pAudioPacketHeader->SetInformation(0, INF_PACKET_BLOCK_NUMBER);
	m_pAudioPacketHeader->SetInformation(1, INF_TOTAL_PACKET_BLOCKS);
	m_pAudioPacketHeader->SetInformation(0, INF_BLOCK_OFFSET);
	m_pAudioPacketHeader->SetInformation(packetLength, INF_FRAME_LENGTH);

	m_pAudioPacketHeader->showDetails("@#BUILD");

	m_pAudioPacketHeader->GetHeaderInByteArray(header);
}

bool CAudioNearEndDataProcessor::PreProcessAudioBeforeEncoding()
{
	//if (!m_bIsLiveStreamingRunning)
	{
#ifdef USE_VAD			
		if (!m_pVoice->HasVoice(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning)))
		{
			return false;
		}
#endif


#ifdef USE_AGC
		m_pAudioCallSession->m_pPlayerGain->AddFarEnd(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning));
		m_pAudioCallSession->m_pRecorderGain->AddGain(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), m_bIsLiveStreamingRunning);
#endif
		 

#ifdef USE_ANS
		memcpy(m_saAudioEncodingTempFrame, m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short));
		m_pNoise->Denoise(m_saAudioEncodingTempFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), m_saAudioEncodingDenoisedFrame, m_bIsLiveStreamingRunning);
#ifdef USE_AECM

		memcpy(m_saAudioRecorderFrame, m_saAudioEncodingDenoisedFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning));
#else
		memcpy(m_saAudioRecorderFrame, m_saAudioEncodingDenoisedFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning));
#endif
#endif
	}
	return true;
}

void CAudioNearEndDataProcessor::GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
	int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime)
{
	Locker lock(*m_pAudioEncodingMutex);

	vCombinedDataLengthVector.clear();
	CombinedLength = 0;
	sendingLengthViewer = 0;
	sendingLengthPeer = 0;
	llAudioChunkDuration = 0;
	llAudioChunkRelativeTime = -1;

	if (-1 == m_llLastChunkLastFrameRT)
	{
		return;
	}

//	HITLER("#RT# lastFrameRT: %lld, lastChunkLastFrameRT: %lld", m_llLastFrameRT, m_llLastChunkLastFrameRT);

	llAudioChunkDuration = m_llLastFrameRT - m_llLastChunkLastFrameRT;
	
	if (0 == llAudioChunkDuration)
	{
		return;
	}

	llAudioChunkRelativeTime = m_llLastChunkLastFrameRT;
	m_llLastChunkLastFrameRT = m_llLastFrameRT;
	
	
	vCombinedDataLengthVector = m_vRawFrameLengthViewer;		
	memcpy(pAudioCombinedDataToSend, m_ucaRawDataToSendViewer, m_iRawDataSendIndexViewer);
	CombinedLength += m_iRawDataSendIndexViewer;
	sendingLengthViewer = m_iRawDataSendIndexViewer;			

	m_iRawDataSendIndexViewer = 0;	
	m_vRawFrameLengthViewer.clear();
}

void CAudioNearEndDataProcessor::UpdateRelativeTimeAndFrame(long long &llLasstTime, long long & llRelativeTime, long long & llCapturedTime)
{	
	llLasstTime = llCapturedTime;	
	llRelativeTime = llCapturedTime - m_llEncodingTimeStampOffset;
}


void CAudioNearEndDataProcessor::DumpEncodingFrame()
{
#ifdef DUMP_FILE
	fwrite(m_saAudioRecorderFrame, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), m_pAudioCallSession->FileInput);
#endif
}


bool CAudioNearEndDataProcessor::MuxIfNeeded(short* shPublisherData, short *shMuxedData ,int &nDataSizeInByte, int nPacketNumber)
{
	long long nFrameNumber;
	nDataSizeInByte = 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING ;
	int nLastDecodedFrameSizeInByte = 0;
	bool bIsMuxed = false;
	int iDataStartIndex, iDataEndIndex;
	int iCallId = 0, nNumberOfBlocks;
	std::vector<pair<int, int> > vMissingBlocks;

	if (m_pAudioCallSession->m_AudioDecodedBuffer.GetQueueSize() != 0)
	{
		bIsMuxed = true;
		nLastDecodedFrameSizeInByte = m_pAudioCallSession->m_AudioDecodedBuffer.DeQueue(m_saAudioPrevDecodedFrame, nFrameNumber) * 2;	//#Magic
		LOG18("#18@# DEQUE data of size %d", nLastDecodedFrameSizeInByte);
		m_pAudioMixer->reset(18, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);
		
		iDataStartIndex = 0;
		iDataEndIndex = 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING - 1;
		iCallId = 0;	//Publisher
		nNumberOfBlocks = 16;
		int nMuxHeaderSize = 14;

		memcpy(shMuxedData + (nMuxHeaderSize / 2), shPublisherData, 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);	//3 instead of 6. since it is short.

		m_pAudioMixer->genCalleeChunkHeader((unsigned char*)shMuxedData, iDataStartIndex, iDataEndIndex,iCallId, nPacketNumber, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING, nNumberOfBlocks, vMissingBlocks);

		m_pAudioMixer->addAudioData((unsigned char*)shMuxedData); // this data should contains only the mux header

		if ((nLastDecodedFrameSizeInByte - nMuxHeaderSize) == 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING) //Both must be 800
		{			
			//iDataStartIndex = 0;
			//iDataEndIndex = 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING - 1;
			//iCallId = 1;	//Callee
			//nNumberOfBlocks = 16;
			//AudioMixer::genCalleeChunkHeader((unsigned char*)m_saAudioPrevDecodedFrame, iDataStartIndex, iDataEndIndex, iCallId, nFrameNumber, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING, nNumberOfBlocks, vMissingBlocks);
			m_pAudioMixer->addAudioData((unsigned char*)m_saAudioPrevDecodedFrame); // this data should contains only the mux header
			
			nDataSizeInByte = m_pAudioMixer->getAudioData((unsigned char*)shMuxedData);
		}
		else
		{
			LOG18("#18@# RETURN WITH FALSE and zeor");
			nDataSizeInByte = 0;
			return false;
		}		
	}
	else 
	{
		memcpy(shMuxedData, m_saAudioRecorderFrame, nDataSizeInByte);
	}
#ifdef DUMP_FILE
	AudioMixer* DumpAudioMixer = new AudioMixer(18,800);
	unsigned char temp[2000];

	if (nDataSizeInByte == 1800)
	{
		DumpAudioMixer->Convert18BitTo16Bit((unsigned char*)shMuxedData, temp, 800);
		fwrite((short *)temp, sizeof(short), 800, m_pAudioCallSession->File18BitData);
	}
	else {
		fwrite(shMuxedData, sizeof(short), 800, m_pAudioCallSession->File18BitData);
	}

	short val = nDataSizeInByte;
	LOG18("#18#NE#DMP nDataSizeInByte = %d", nDataSizeInByte);
	fwrite(&val, 2, 1, m_pAudioCallSession->File18BitType);

#endif
	return bIsMuxed;
}

void CAudioNearEndDataProcessor::StopEncodingThread()
{
	m_bAudioEncodingThreadRunning = false;

	while (!m_bAudioEncodingThreadClosed)
		Tools::SOSleep(5);
}

void CAudioNearEndDataProcessor::StartCallInLive()
{
	if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
	{
		m_llLastChunkLastFrameRT = -1;
		m_iRawDataSendIndexViewer = 0;
	}
}

void CAudioNearEndDataProcessor::StopCallInLive()
{
	if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
	{
		m_llLastChunkLastFrameRT = -1;
		m_iRawDataSendIndexViewer = 0;
	}
}

long long CAudioNearEndDataProcessor::GetBaseOfRelativeTime()
{ 
	return m_llEncodingTimeStampOffset;
}