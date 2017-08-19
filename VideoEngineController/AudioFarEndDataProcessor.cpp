#include "AudioFarEndDataProcessor.h"
#include "AudioCallSession.h"
#include "AudioDecoderBuffer.h"
#include "AudioDePacketizer.h"
#include "LogPrinter.h"
#include "CommonElementsBucket.h"
#include "LiveAudioParser.h"
#include "LiveAudioParserForCallee.h"
#include "LiveAudioParserForPublisher.h"
#include "LiveAudioParserForChannel.h"
#include "AudioMixer.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "MuxHeader.h"
#include "AudioShortBufferForPublisherFarEnd.h"
#include "LiveAudioDecodingQueue.h"
#include "AudioPacketHeader.h"
#include "AudioEncoderBuffer.h"
#include "AudioDecoderProvider.h"
#include "AudioEncoderInterface.h"
#include "AudioDecoderInterface.h"
#include "GomGomGain.h"
#include "AudioGainInstanceProvider.h"
#include "AudioGainInterface.h"
#include "Trace.h"
#include "AudioMacros.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif



namespace MediaSDK
{

	AudioFarEndDataProcessor::AudioFarEndDataProcessor(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		m_nServiceType(nServiceType),
		m_nEntityType(nEntityType),
		m_pAudioCallSession(pAudioCallSession),
		m_bIsLiveStreamingRunning(bIsLiveStreamingRunning),
		m_bAudioDecodingThreadRunning(false),
		m_bAudioDecodingThreadClosed(true),
		m_llLastTime(-1),
		m_bAudioQualityLowNotified(false),
		m_bAudioQualityHighNotified(false),
		m_bAudioShouldStopNotified(false),
		m_inoLossSlot(0),
		m_ihugeLossSlot(0),
		m_pDataEventListener(nullptr),
		m_pNetworkChangeListener(nullptr),
		m_pAudioAlarmListener(nullptr)
	{
		m_b1stPlaying = true;
		m_llNextPlayingTime = -1;
		m_AudioReceivedBuffer.reset(new CAudioByteBuffer());

		memset(m_saPlayingData, 0, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false) * sizeof(short));

		for (int i = 0; i < MAX_NUMBER_OF_CALLEE; i++){
			m_vAudioFarEndBufferVector.push_back(new LiveAudioDecodingQueue());	//Need to delete.
		}

		m_pLiveAudioParser = nullptr;
#if !defined(TARGET_OS_WINDOWS_PHONE)
		if (SERVICE_TYPE_CALL == m_nServiceType || SERVICE_TYPE_SELF_CALL == m_nServiceType)
		{
			m_AudioReceivedBuffer->SetQueueCapacity(MAX_AUDIO_DECODER_BUFFER_CAPACITY_FOR_CALL);
		}
#endif
		if (SERVICE_TYPE_LIVE_STREAM == m_nServiceType || SERVICE_TYPE_SELF_STREAM == m_nServiceType)
		{
			if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
			{
				m_pLiveAudioParser = new CLiveAudioParserForPublisher(m_vAudioFarEndBufferVector);
			}
			else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
			{
				m_pLiveAudioParser = new CLiveAudioParserForCallee(m_vAudioFarEndBufferVector);
			}
		}
		else if (SERVICE_TYPE_CHANNEL == m_nServiceType)
		{
			m_pLiveAudioParser = new CLiveAudioParserForChannel(m_vAudioFarEndBufferVector);
		}

		m_pAudioFarEndPacketHeader = pAudioCallSession->GetAudioFarEndPacketHeader();
		m_pAudioDePacketizer = new AudioDePacketizer(m_pAudioCallSession);

		m_cAacDecoder = AudioDecoderProvider::GetAudioDecoder(AAC_Decoder);
		if (m_cAacDecoder.get())
		{
			m_cAacDecoder->SetParameters(44100, 2);
		}

		m_pGomGomGain.reset(new GomGomGain());

		m_pAudioEncoder = m_pAudioCallSession->GetAudioEncoder();
	}

	AudioFarEndDataProcessor::~AudioFarEndDataProcessor()
	{
		for (auto &liveQ : m_vAudioFarEndBufferVector) {
			if (liveQ) {
				delete liveQ;
				liveQ = nullptr;
			}
		}

		if (m_pAudioDePacketizer)
		{
			delete m_pAudioDePacketizer;
			m_pAudioDePacketizer = nullptr;
		}
		//if (m_ReceivingHeader)
		//{
		//delete m_ReceivingHeader;
		//m_ReceivingHeader = nullptr;
		//}
		if (m_pLiveAudioParser)
		{
			delete m_pLiveAudioParser;
			m_pLiveAudioParser = NULL;
		}
	}


	int AudioFarEndDataProcessor::DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > &vMissingFrames)
	{
		if (m_bIsLiveStreamingRunning)
		{
			/*
			TODO:
			1. Here we assume that right now there is a single caller.
			So we use default FarEnd data sender id zero.
			if there is multiple caller then you need to give an ID for the data sender.
			*/
			m_pLiveAudioParser->ProcessLiveAudio(0, nOffset, pucaDecodingAudioData, unLength, frameSizes, numberOfFrames, vMissingFrames);
			return 1;
		}

		return  m_AudioReceivedBuffer->EnQueue(pucaDecodingAudioData, unLength);
	}

	void AudioFarEndDataProcessor::StartCallInLive(int nEntityType)
	{
		if (nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
		{
			m_vAudioFarEndBufferVector[0]->ResetBuffer(); //Contains Data From Live Stream
		}
		m_AudioReceivedBuffer->ResetBuffer();
		m_nEntityType = nEntityType;
	}

	void AudioFarEndDataProcessor::StopCallInLive(int nEntityType)
	{
		m_vAudioFarEndBufferVector[0]->ResetBuffer();
		m_nEntityType = nEntityType;
	}


	void AudioFarEndDataProcessor::DecodeAndPostProcessIfNeeded(int &iPacketNumber, int &nCurrentPacketHeaderLength, int &nCurrentAudioPacketType)
	{
		m_iLastDecodedPacketNumber = iPacketNumber;
		LOGEF("Role %d, Before decode", m_iRole);
		if (!m_bIsLiveStreamingRunning)
		{
			m_nDecodedFrameSize = m_pAudioCallSession->GetAudioDecoder()->DecodeAudio(m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize, m_saDecodedFrame);
//			ALOG("#A#DE#--->> Self#  PacketNumber = " + Tools::IntegertoStringConvert(iPacketNumber));
			LOGEF("Role %d, done decode", m_iRole);
		}
		else
		{
			if (AUDIO_CHANNEL_PACKET_TYPE == nCurrentAudioPacketType)	//Only for channel
			{
				long long llNow = Tools::CurrentTimestamp();
				if (m_cAacDecoder.get()){
					m_nDecodedFrameSize = m_cAacDecoder->DecodeAudio(m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize, m_saDecodedFrame);
					//				LOG_AAC("#aac# AAC_DecodingFrameSize: %d, DecodedFrameSize: %d", m_nDecodingFrameSize, m_nDecodedFrameSize);
				}
				else{
					LOG_AAC("#aac# AAC decoder not exist.");
				}
			}
			else
			{
				memcpy(m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize);
				m_nDecodedFrameSize = m_nDecodingFrameSize / sizeof(short);
				LOGEF("Role %d, no viewers in call", m_iRole);
			}
		}
	}

	bool AudioFarEndDataProcessor::IsPacketTypeSupported(int &nCurrentAudioPacketType)
	{
		if (!m_bIsLiveStreamingRunning)
		{
			if (m_pAudioFarEndPacketHeader->IsPacketTypeSupported(nCurrentAudioPacketType))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}

	void AudioFarEndDataProcessor::SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llLastTime, int iCurrentPacketNumber)
	{
		COW("###^^^### SENT TO PLAYER DATA .................");
		long long llNow = 0;

		if (m_bIsLiveStreamingRunning == true)
		{

			llNow = Tools::CurrentTimestamp();

			__LOG("!@@@@@@@@@@@  #WQ     FN: %d -------- Receiver Time Diff : %lld    DataLength: %d",
				iPacketNumber, llNow - llLastTime, nSentFrameSize);

			llLastTime = llNow;
			if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER)
			{
				LOG18("#18@# PUb enq , packet type %d", iCurrentPacketNumber);
				int iStartIndex = 0;
				int iEndIndex = 1599;
				int iCalleeId = 1;
				int iTotalBlocks = 16;
				int iFrameSize = 800;
				int iMuxHeaderSize = AUDIO_MUX_HEADER_LENGHT;

				MuxHeader audioMuxHeader(iCalleeId, iCurrentPacketNumber, m_vFrameMissingBlocks);

				m_pAudioCallSession->m_PublisherBufferForMuxing->EnQueue(pshSentFrame, nSentFrameSize, iCurrentPacketNumber, audioMuxHeader);
			}

			HITLER("*STP -> PN: %d, FS: %d, STime: %lld", iCurrentPacketNumber, nSentFrameSize, Tools::CurrentTimestamp());
//#ifdef __ANDROID__
//			if (m_bIsLiveStreamingRunning && (ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType))
//			{
//				m_pGomGomGain->AddGain(pshSentFrame, nSentFrameSize, m_bIsLiveStreamingRunning);
//			}
//#endif
			//m_pEventNotifier->fireAudioEvent(m_llFriendID, SERVICE_TYPE_LIVE_STREAM, nSentFrameSize, pshSentFrame);
#ifdef USE_AECM
			if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER || m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
			{
				LOG18("Pushing to q");
				memcpy(m_saPlayingData, pshSentFrame, nSentFrameSize * sizeof(short));
			}
			else
			{
				if (m_pDataEventListener != nullptr)
				{
					m_nPacketPlayed ++;
					MediaLog(LOG_INFO, "[AFEDP]Viewer# To Player [SendToPlayer]");
					m_pDataEventListener->FireDataEvent(SERVICE_TYPE_LIVE_STREAM, nSentFrameSize, pshSentFrame);
#ifdef PCM_DUMP
					if (m_pAudioCallSession->PlayedFile)
					{
						fwrite(pshSentFrame, 2, nSentFrameSize, m_pAudioCallSession->PlayedFile);
					}
#endif
				}
			}
#else
			if (m_pDataEventListener != nullptr)
			{
				m_pDataEventListener->FireDataEvent(SERVICE_TYPE_LIVE_STREAM, nSentFrameSize, pshSentFrame);
			}
#endif

		}
		else
		{
			LOG18("Pushing to q");
#ifdef __ANDROID__
			if (m_pAudioCallSession->GetPlayerGain().get())
			{
				m_pAudioCallSession->GetPlayerGain()->AddGain(pshSentFrame, nSentFrameSize, false);
			}
#endif // __ANDROID__

			memcpy(m_saPlayingData, pshSentFrame, nSentFrameSize * sizeof(short));
		}


	}
	
	void AudioFarEndDataProcessor::DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize)
	{
#ifdef DUMP_FILE
		fwrite(psDecodedFrame, 2, nDecodedFrameSize, m_pAudioCallSession->FileOutput);
#endif
	}

	void AudioFarEndDataProcessor::ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
		int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength)
	{
		m_pAudioFarEndPacketHeader->CopyHeaderToInformation(header);

		packetType = m_pAudioFarEndPacketHeader->GetInformation(INF_PACKETTYPE);
		nHeaderLength = m_pAudioFarEndPacketHeader->GetInformation(INF_HEADERLENGTH);
		networkType = m_pAudioFarEndPacketHeader->GetInformation(INF_NETWORKTYPE);
		slotNumber = m_pAudioFarEndPacketHeader->GetInformation(INF_SLOTNUMBER);
		packetNumber = m_pAudioFarEndPacketHeader->GetInformation(INF_PACKETNUMBER);
		packetLength = m_pAudioFarEndPacketHeader->GetInformation(INF_BLOCK_LENGTH);
		recvSlotNumber = m_pAudioFarEndPacketHeader->GetInformation(INF_RECVDSLOTNUMBER);
		numPacketRecv = m_pAudioFarEndPacketHeader->GetInformation(INF_NUMPACKETRECVD);
		channel = m_pAudioFarEndPacketHeader->GetInformation(INF_CHANNELS);
		version = m_pAudioFarEndPacketHeader->GetInformation(INF_VERSIONCODE);
		timestamp = m_pAudioFarEndPacketHeader->GetInformation(INF_TIMESTAMP);


		iBlockNumber = m_pAudioFarEndPacketHeader->GetInformation(INF_PACKET_BLOCK_NUMBER);
		nNumberOfBlocks = m_pAudioFarEndPacketHeader->GetInformation(INF_TOTAL_PACKET_BLOCKS);
		iOffsetOfBlock = m_pAudioFarEndPacketHeader->GetInformation(INF_BLOCK_OFFSET);
		nFrameLength = m_pAudioFarEndPacketHeader->GetInformation(INF_FRAME_LENGTH);

		if (iBlockNumber == -1)
		{
			iBlockNumber = 0;
		}

		if (nNumberOfBlocks == -1)
		{
			nNumberOfBlocks = 1;
			iOffsetOfBlock = 0;
			nFrameLength = packetLength;
		}
	}

	bool AudioFarEndDataProcessor::IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType)
	{
		LOGENEW("m_iRole = %d, nCurrentAudioPacketType = %d\n", m_nEntityType, nCurrentAudioPacketType);

		if (SERVICE_TYPE_CHANNEL == m_nServiceType)	//Channel
		{
			if (AUDIO_CHANNEL_PACKET_TYPE == nCurrentAudioPacketType)
			{
				return true;
			}
			return false;
		}
		else if (SERVICE_TYPE_LIVE_STREAM == m_nServiceType || SERVICE_TYPE_SELF_STREAM == m_nServiceType)	//LiveStreaming.
		{
			if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER)	//
			{
				if (nCurrentAudioPacketType == AUDIO_LIVE_CALLEE_PACKET_TYPE)
					return true;
			}
			else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
			{
				if (AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED == nCurrentAudioPacketType || AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED == nCurrentAudioPacketType)
				{
					return true;
				}
			}

		}

		return false;
	}


	bool AudioFarEndDataProcessor::IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType)
	{
#ifndef LOCAL_SERVER_LIVE_CALL
		if (m_bIsLiveStreamingRunning)
		{
			if (m_pLiveAudioParser->GetRoleChanging() == true)
			{
				return false;
			}
			if ((m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER) || (m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_VIEWER_CALLEE))
			{
				return true;
			}
			if (-1 == m_llDecodingTimeStampOffset)
			{
				Tools::SOSleep(__LIVE_FIRST_FRAME_SLEEP_TIME__);
				m_llDecodingTimeStampOffset = Tools::CurrentTimestamp() - llCurrentFrameRelativeTime;
				LOGENEW("iPacketNumber resyncing");
				m_nExpectedNextPacketNumber = iPacketNumber++;
			}
			else
			{
				long long llNow = Tools::CurrentTimestamp();
				long long llExpectedEncodingTimeStamp = llNow - m_llDecodingTimeStampOffset;
				long long llWaitingTime = llCurrentFrameRelativeTime - llExpectedEncodingTimeStamp;

				if (m_nExpectedNextPacketNumber < iPacketNumber)
				{
					m_nPacketsLost += iPacketNumber - m_nExpectedNextPacketNumber;
					LOGDISCARD("Discarding because of loss, m_nPacketsRecvdTimely = %d, m_nPacketsRecvdNotTimely = %d, percentage = %f, m_nPacketsLost = %d, m_nPacketNotPlayed = %d",
						m_nPacketsRecvdTimely, m_nPacketsRecvdNotTimely, m_nPacketsRecvdTimely == 0 ? 0 : (m_nPacketsRecvdNotTimely * 1.0 / m_nPacketsRecvdTimely) * 100,
						m_nPacketsLost, m_nPacketsRecvdTimely - m_nPacketPlayed);
				}
				m_nExpectedNextPacketNumber = iPacketNumber + 1;

				LOGENEW("@@@@@@@@@--> relativeTime: [%lld] DELAY FRAME: %lld  currentTime: %lld, iPacketNumber = %d", llCurrentFrameRelativeTime, llWaitingTime, llNow, iPacketNumber);

				
				
				if (llExpectedEncodingTimeStamp - __AUDIO_DELAY_TIMESTAMP_TOLERANCE__ > llCurrentFrameRelativeTime)
				{
					CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "CAudioCallSession::IsPacketProcessableBasedOnRelativeTime relativeTime = "
						+ Tools::getText(llCurrentFrameRelativeTime) + " DELAY = " + Tools::getText(llWaitingTime) + " currentTime = " + Tools::getText(llNow)
						+ " iPacketNumber = " + Tools::getText(iPacketNumber));
					
					m_nPacketsRecvdNotTimely++;
					LOGDISCARD("Discarding because of delay, m_nPacketsRecvdTimely = %d, m_nPacketsRecvdNotTimely = %d, percentage = %f, m_nPacketsLost = %d, m_nPacketNotPlayed = %d",
						m_nPacketsRecvdTimely, m_nPacketsRecvdNotTimely, m_nPacketsRecvdTimely == 0 ? 0 : (m_nPacketsRecvdNotTimely * 1.0 / m_nPacketsRecvdTimely) * 100,
						m_nPacketsLost, m_nPacketsRecvdTimely - m_nPacketPlayed);

					if (m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_VIEWER)
					{
						return true; //Use delay frame for viewer.
					}
					return false;

				}
				else if (llCurrentFrameRelativeTime - llExpectedEncodingTimeStamp > MAX_WAITING_TIME_FOR_CHANNEL)
				{
					m_nPacketsRecvdNotTimely++;
					LOGDISCARD("Discarding because of delay 2nd case, m_nPacketsRecvdTimely = %d, m_nPacketsRecvdNotTimely = %d, percentage = %f, m_nPacketsLost = %d, m_nPacketNotPlayed = %d",
						m_nPacketsRecvdTimely, m_nPacketsRecvdNotTimely, m_nPacketsRecvdTimely == 0 ? 0 : (m_nPacketsRecvdNotTimely * 1.0 / m_nPacketsRecvdTimely) * 100,
						m_nPacketsLost, m_nPacketsRecvdTimely - m_nPacketPlayed);

					return false;
				}
				


				while (llExpectedEncodingTimeStamp + __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ < llCurrentFrameRelativeTime)
				{
					Tools::SOSleep(5);
					llExpectedEncodingTimeStamp = Tools::CurrentTimestamp() - m_llDecodingTimeStampOffset;
					
				}
				m_nPacketsRecvdTimely++;
				LOGDISCARD("Discarding sttistics, m_nPacketsRecvdTimely = %d, m_nPacketsRecvdNotTimely = %d, percentage = %f, m_nPacketsLost = %d, m_nPacketNotPlayed = %d",
					m_nPacketsRecvdTimely, m_nPacketsRecvdNotTimely, m_nPacketsRecvdTimely == 0 ? 0 : (m_nPacketsRecvdNotTimely * 1.0 / m_nPacketsRecvdTimely) * 100,
					m_nPacketsLost, m_nPacketsRecvdTimely - m_nPacketPlayed);
			}
			return true;
		}
		else
		{
			return true;
		}
#else
		return true;
#endif
	}

	void AudioFarEndDataProcessor::SetSlotStatesAndDecideToChangeBitRate(int &nSlotNumber)
	{
		if (!m_bIsLiveStreamingRunning)
		{
			if (nSlotNumber != m_iCurrentRecvdSlotID)
			{
				//Todo: m_iPrevRecvdSlotID may be accessed from multiple thread.
				m_pAudioCallSession->m_iPrevRecvdSlotID = m_iCurrentRecvdSlotID;
				if (m_pAudioCallSession->m_iPrevRecvdSlotID != -1)
				{
					m_pAudioCallSession->m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot;
				}

				m_iCurrentRecvdSlotID = nSlotNumber;
				m_iReceivedPacketsInCurrentSlot = 0;

				if (m_pAudioCallSession->GetIsVideoCallRunning()) {
					this->DecideToChangeBitrate(m_iOpponentReceivedPackets);
				}
				else if (m_pAudioEncoder->GetCurrentBitrate() != AUDIO_BITRATE_INIT){
					m_pAudioEncoder->SetBitrate(AUDIO_BITRATE_INIT);
				}
			}
			m_iReceivedPacketsInCurrentSlot++;
		}
	}

	void AudioFarEndDataProcessor::PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
		long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime)
	{
		if (!m_bIsLiveStreamingRunning)
		{
			llNow = Tools::CurrentTimestamp();
			//            ALOG("#DS Size: "+m_Tools.IntegertoStringConvert(m_nDecodedFrameSize));
			if (llNow - llTimeStamp >= 1000)
			{
				//                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Num AudioDataDecoded = " + m_Tools.IntegertoStringConvert(iDataSentInCurrentSec));
				iDataSentInCurrentSec = 0;
				llTimeStamp = llNow;
			}
			iDataSentInCurrentSec++;


			nDecodingTime = Tools::CurrentTimestamp() - llCapturedTime;
			dbTotalTime += nDecodingTime;
		}
	}


	void AudioFarEndDataProcessor::DecideToChangeBitrate(int iNumPacketRecvd)
	{
#ifndef AUDIO_FIXED_BITRATE

		//	ALOG("#BR# DecideToChangeBitrate: "+m_Tools.IntegertoStringConvert(iNumPacketRecvd));

		int nCurrentBitRate = m_pAudioEncoder->GetCurrentBitrate();

		if (iNumPacketRecvd == AUDIO_SLOT_SIZE)
		{
			m_inoLossSlot++;
			m_ihugeLossSlot = 0;
		}
		else
		{
			m_inoLossSlot = 0;

			int nChangedBitRate = (iNumPacketRecvd * nCurrentBitRate) / AUDIO_SLOT_SIZE;
			//		ALOG("now br trying to set : "+Tools::IntegertoStringConvert(nChangedBitRate));
			HITLER("@@@@------------------------>Bitrate: %d\n", nChangedBitRate);
			if (nChangedBitRate < AUDIO_LOW_BITRATE && nChangedBitRate >= AUDIO_MIN_BITRATE)
			{
				m_ihugeLossSlot = 0;

				m_pAudioEncoder->SetBitrate(nChangedBitRate);

				if (false == m_bAudioQualityLowNotified)
				{
					//m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_llFriendID, CEventNotifier::NETWORK_STRENTH_GOOD);
					if (m_pNetworkChangeListener != nullptr)
					{
						m_pNetworkChangeListener->FireNetworkChange(CEventNotifier::NETWORK_STRENTH_GOOD);
					}

					m_bAudioQualityLowNotified = true;
					m_bAudioQualityHighNotified = false;
					m_bAudioShouldStopNotified = false;
				}
			}
			else if (nChangedBitRate < AUDIO_MIN_BITRATE)
			{
				m_ihugeLossSlot++;

				m_pAudioEncoder->SetBitrate(AUDIO_MIN_BITRATE);

				if (false == m_bAudioShouldStopNotified && m_ihugeLossSlot >= AUDIO_MAX_HUGE_LOSS_SLOT)
				{
					//m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_llFriendID, CEventNotifier::NETWORK_STRENTH_BAD);
					if (m_pNetworkChangeListener != nullptr)
					{
						m_pNetworkChangeListener->FireNetworkChange(CEventNotifier::NETWORK_STRENTH_BAD);
					}

					//m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO, 0, 0);		
					if (m_pAudioAlarmListener != nullptr)
					{
						m_pAudioAlarmListener->FireAudioAlarm(AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO);
					}

					m_pAudioCallSession->m_iNextPacketType = AUDIO_NOVIDEO_PACKET_TYPE;

					m_bAudioShouldStopNotified = true;
					m_bAudioQualityHighNotified = false;
					m_bAudioQualityLowNotified = false;
				}
			}
			else if (nChangedBitRate >= AUDIO_LOW_BITRATE)
			{
				m_ihugeLossSlot = 0;

				m_pAudioEncoder->SetBitrate(nChangedBitRate);

				if (false == m_bAudioQualityHighNotified)
				{
					//m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_llFriendID, CEventNotifier::NETWORK_STRENTH_EXCELLENT);
					if (m_pNetworkChangeListener != nullptr)
					{
						m_pNetworkChangeListener->FireNetworkChange(CEventNotifier::NETWORK_STRENTH_EXCELLENT);
					}

					m_bAudioQualityHighNotified = true;
					m_bAudioQualityLowNotified = false;
					m_bAudioShouldStopNotified = false;
				}
			}
		}

		if (m_inoLossSlot == AUDIO_MAX_NO_LOSS_SLOT)
		{
			if (nCurrentBitRate + AUDIO_BITRATE_UP_STEP <= AUDIO_MAX_BITRATE)
			{
				m_pAudioEncoder->SetBitrate(nCurrentBitRate + AUDIO_BITRATE_UP_STEP);
			}
			else
			{
				m_pAudioEncoder->SetBitrate(AUDIO_MAX_BITRATE);
			}
			m_inoLossSlot = 0;
		}
		//	ALOG("#V# E: DecideToChangeBitrate: Done");
#endif
	}

	void AudioFarEndDataProcessor::SyncPlayingTime()
	{
		long long llCurrentTimeStamp = Tools::CurrentTimestamp();
		if (m_b1stPlaying)
		{
			m_llNextPlayingTime = llCurrentTimeStamp + 100;
			m_b1stPlaying = false;
		}
		else
		{
			if (llCurrentTimeStamp + 20 < m_llNextPlayingTime)
			{
				LOG18("processplayingdata sleeping time = %d", m_llNextPlayingTime - llCurrentTimeStamp - 20);
				Tools::SOSleep(m_llNextPlayingTime - llCurrentTimeStamp - 20);
			}
			else
			{
				LOG18("processplayingdata sleeping time = %d", 0);
			}
			LOG18("processplayingdata timestamp = %lld", Tools::CurrentTimestamp());
			m_llNextPlayingTime += 100;
		}
	}

	void AudioFarEndDataProcessor::ProcessPlayingData()
	{
#ifdef USE_AECM
		if (m_pAudioCallSession->m_bRecordingStarted)
		{
			if (m_pAudioCallSession->IsTraceSendingEnabled() && m_pAudioCallSession->m_bTraceTailRemains)
			{				
				LOGFARQUAD("qpushpop while sending trace m_FarendBufferSize = %d",
					m_pAudioCallSession->m_FarendBuffer->GetQueueSize());
				m_pAudioCallSession->m_bTraceTailRemains = m_pAudioCallSession -> m_pTrace -> GenerateTrace(m_saPlayingData, 800);
			}
			
			if (!m_pAudioCallSession->m_bTraceSent)
			{
				m_pAudioCallSession->m_FarendBuffer->ResetBuffer();
				m_pAudioCallSession->m_llTraceSendingTime = Tools::CurrentTimestamp();
				m_pAudioCallSession->m_bTraceSent = true;
			}
		}

		if (m_pAudioCallSession->m_bTraceSent)
		{
			long long llCurrentTimeStamp = Tools::CurrentTimestamp();
			if (m_pDataEventListener != nullptr)
			{				
				MediaLog(LOG_INFO, "[AFEDP] To Player# Playing Time: %lld [%lld]\n", llCurrentTimeStamp, m_llNextPlayingTime);
				m_pDataEventListener->FireDataEvent(m_pAudioCallSession->GetServiceType(), CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false), m_saPlayingData);
#ifdef PCM_DUMP
				if (m_pAudioCallSession->PlayedFile)
				{
					fwrite(m_saPlayingData, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false), m_pAudioCallSession->PlayedFile);
				}
#endif
			}
			llCurrentTimeStamp = Tools::CurrentTimestamp();
			LOG18("qpushpop pushing silent llCurrentTimeStamp = %lld", llCurrentTimeStamp);
			if (m_pAudioCallSession->m_bEnablePlayerTimeSyncDuringEchoCancellation)
			{
				SyncPlayingTime();
			}
			m_pAudioCallSession->m_FarendBuffer->EnQueue(m_saPlayingData, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false), 0);
			memset(m_saPlayingData, 0, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false) * sizeof(short));
			LOG18("ppplaying 0 data");
		}
		else
		{
			LOG18("ppplaying 0 data, trace not sent");
			Tools::SOSleep(20);
		}
#else
		if (m_pDataEventListener != nullptr)
		{
			m_pDataEventListener->FireDataEvent(m_pAudioCallSession -> GetServiceType(), CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false), m_saPlayingData);
#ifdef PCM_DUMP
			if (m_pAudioCallSession->PlayedFile)
			{
				fwrite(m_saPlayingData, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false), m_pAudioCallSession->PlayedFile);
			}
#endif
		}
#endif
	}

	
	void AudioFarEndDataProcessor::SetEventCallback(DataEventListener* pDataListener, NetworkChangeListener* networkListener, AudioAlarmListener* alarmListener)
	{
		m_pDataEventListener = pDataListener;
		m_pNetworkChangeListener = networkListener;
		m_pAudioAlarmListener = alarmListener;
	}

} //namespace MediaSDK
