#include "AudioFarEndDataProcessor.h"
#include "AudioCallSession.h"
#include "AudioDecoderBuffer.h"
#include "AudioDePacketizer.h"
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
#include "AudioHeaderCall.h"
#include "AudioHeaderLive.h"
#include "AudioNearEndDataProcessor.h"

#include <string.h>

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define AUDIO_FIXED_BITRATE

namespace MediaSDK
{

	AudioFarEndDataProcessor::AudioFarEndDataProcessor(int nAudioFlowType, int nEntityType, CAudioCallSession *pAudioCallSession, bool bIsLiveStreamingRunning) :
		m_nAudioFlowType(nAudioFlowType),
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
		m_bPlayingNotStartedYet = true;
		m_llNextPlayingTime = -1;
		m_AudioReceivedBuffer.reset(new CAudioByteBuffer());

		memset(m_saPlayingData, 0, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false) * sizeof(short));

		for (int i = 0; i < MAX_NUMBER_OF_CALL_PARTICIPANTS; i++){
			m_vAudioFarEndBufferVector.push_back(new LiveAudioDecodingQueue());	//Need to delete.
		}

		m_pLiveAudioParser = nullptr;
#if !defined(TARGET_OS_WINDOWS_PHONE)
		if (AUDIO_FLOW_OPUS_CALL == m_nAudioFlowType || AUDIO_FLOW_USELESS_CALL == m_nAudioFlowType)
		{
			m_AudioReceivedBuffer->SetQueueCapacity(MAX_AUDIO_DECODER_BUFFER_CAPACITY_FOR_CALL);
		}
#endif
		if (AUDIO_FLOW_OPUS_LIVE_CHANNEL == m_nAudioFlowType || AUDIO_FLOW_USELESS_STREAM == m_nAudioFlowType)
		{
			if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
			{
				m_pLiveAudioParser = new CLiveAudioParserForPublisher(m_vAudioFarEndBufferVector, m_pAudioCallSession->GetSessionStatListener());
			}
			else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
			{
				m_pLiveAudioParser = new CLiveAudioParserForCallee(m_vAudioFarEndBufferVector, m_pAudioCallSession->GetSessionStatListener());
			}
		}
		else if (AUDIO_FLOW_AAC_LIVE_CHANNEL == m_nAudioFlowType)
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

		if (nullptr != m_pAudioDePacketizer)
		{
			delete m_pAudioDePacketizer;
			m_pAudioDePacketizer = nullptr;
		}

		if (nullptr != m_pLiveAudioParser)
		{
			delete m_pLiveAudioParser;
			m_pLiveAudioParser = nullptr;
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


	void AudioFarEndDataProcessor::DecodeAndPostProcessIfNeeded(const int iPacketNumber, const int nCurrentPacketHeaderLength, const int nCurrentAudioPacketType)
	{
		m_iLastDecodedPacketNumber = iPacketNumber;
		LOGEF("Role %d, Before decode", m_iRole);
		if (!m_bIsLiveStreamingRunning)
		{
			m_nDecodedFrameSize = m_pAudioCallSession->GetAudioDecoder()->DecodeAudio(m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize, m_saDecodedFrame);
			//MediaLog(CODE_TRACE, "#A#DE#--->> Self#  PacketNumber = %d  Role= %d",iPacketNumber, m_iRole);
			
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
					//MediaLog(CODE_TRACE, "#aac# AAC decoder not exist.");
				}
			}
			else
			{
				memcpy(m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize);
				m_nDecodedFrameSize = m_nDecodingFrameSize / sizeof(short);
				//MediaLog(CODE_TRACE, "Role %d, no viewers in call", m_iRole);
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

	void AudioFarEndDataProcessor::SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llLastTime, int iCurrentPacketNumber, int nEchoStateFlags)
	{
		MediaLog(LOG_INFO, "[FE][AFEDP] SENT TO PLAYER DATA .................");
		long long llNow = 0;

		if (m_bIsLiveStreamingRunning == true)
		{

			llNow = Tools::CurrentTimestamp();

			MediaLog(CODE_TRACE, "[FE][AFEDP] Live Streaming Receiver Time Diff : %lld, DataLength: %d",
				llNow - llLastTime, nSentFrameSize);

			llLastTime = llNow;

			MediaLog(LOG_DEBUG, "[FE][AFEDP] STP -> PN: %d, FS: %d, STime: %lld", iCurrentPacketNumber, nSentFrameSize, Tools::CurrentTimestamp());

			//m_pEventNotifier->fireAudioEvent(m_llFriendID, AUDIO_FLOW_OPUS_LIVE_CHANNEL, nSentFrameSize, pshSentFrame);

			m_pAudioCallSession->m_pPlayerSidePreGain->WriteDump(pshSentFrame, 2, nSentFrameSize);

			if (m_nEntityType == ENTITY_TYPE_VIEWER && m_pAudioCallSession->GetPlayerGain().get())
			{
				m_pAudioCallSession->GetPlayerGain()->AddGain(pshSentFrame, nSentFrameSize, true, nEchoStateFlags);
			}

			if (m_pAudioCallSession->m_pNearEndProcessor->IsEchoCancellerEnabled())
			{
				if (m_nEntityType == ENTITY_TYPE_PUBLISHER_CALLER || m_nEntityType == ENTITY_TYPE_VIEWER_CALLEE)
				{
					memcpy(m_saPlayingData, pshSentFrame, nSentFrameSize * sizeof(short));
				}
				else
				{
					if (m_pDataEventListener != nullptr)
					{
						m_nPacketPlayed++;
						MediaLog(LOG_INFO, "[FE][AFEDP] Viewer# To Player [SendToPlayer]\n");
						m_pDataEventListener->FireDataEvent(m_pAudioCallSession->m_nAudioServiceType, nSentFrameSize, pshSentFrame);
						m_pAudioCallSession->m_pPlayedFE->WriteDump(pshSentFrame, 2, nSentFrameSize);
					}
				}
			}
			else
			{
				if (m_pDataEventListener != nullptr)
				{
					m_pAudioCallSession->m_pPlayedFE->WriteDump(pshSentFrame, 2, nSentFrameSize);
					MediaLog(LOG_INFO, "[FE][AFEDP] FireDataEvent [SendToPlayer]\n");
					m_pDataEventListener->FireDataEvent(m_pAudioCallSession->m_nAudioServiceType, nSentFrameSize, pshSentFrame);
				}
			}
		}
		else
		{
			MediaLog(LOG_DEBUG, "[FE][AFEDP] else condition STP -> PN: %d, FS: %d, STime: %lld", iCurrentPacketNumber, nSentFrameSize, Tools::CurrentTimestamp());
			m_pAudioCallSession->m_pPlayerSidePreGain->WriteDump(pshSentFrame, 2, nSentFrameSize);
#ifdef __ANDROID__
			if (m_pAudioCallSession->GetPlayerGain().get())
			{
				m_pAudioCallSession->GetPlayerGain()->AddGain(pshSentFrame, nSentFrameSize, true, nEchoStateFlags);
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

	void AudioFarEndDataProcessor::ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &packetNumber, int &packetLength, 
		int &channel, int &version, long long &timestamp, unsigned char* header, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength, int &nEchoStateFlags)
	{
		m_pAudioFarEndPacketHeader->CopyHeaderToInformation(header);

		m_pAudioFarEndPacketHeader->ShowDetails("[AFEDP] getting");

		packetType = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_PACKETTYPE);
		nHeaderLength = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_HEADERLENGTH);
		networkType = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_NETWORKTYPE);
		packetNumber = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_PACKETNUMBER);
		packetLength = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_BLOCK_LENGTH);
		channel = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_CHANNELS);
		version = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_VERSIONCODE);
		timestamp = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_TIMESTAMP);


		iBlockNumber = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_PACKET_BLOCK_NUMBER);
		nNumberOfBlocks = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_TOTAL_PACKET_BLOCKS);
		iOffsetOfBlock = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_BLOCK_OFFSET);
		nFrameLength = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_FRAME_LENGTH);
		nEchoStateFlags = m_pAudioFarEndPacketHeader->GetInformation(INF_CALL_ECHO_STATE_FLAGS);
		MediaLog(LOG_DEBUG, "[FE][AFEDP] getting from header nEchoStateFlags = %d\n", nEchoStateFlags);

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


	void AudioFarEndDataProcessor::ParseLiveHeader(int &packetType, int &nHeaderLength, int &version, int &packetNumber, int &packetLength,
		 long long &timestamp, int &nEchoStateFlags, unsigned char* header)
	{
		m_pAudioFarEndPacketHeader->CopyHeaderToInformation(header);

		m_pAudioFarEndPacketHeader->ShowDetails("[AFEDP][ECHOFLAG] ParseLiveHeader");

		packetType = m_pAudioFarEndPacketHeader->GetInformation(INF_LIVE_PACKETTYPE);
		nHeaderLength = m_pAudioFarEndPacketHeader->GetInformation(INF_LIVE_HEADERLENGTH);
		version = m_pAudioFarEndPacketHeader->GetInformation(INF_LIVE_VERSIONCODE);
		packetNumber = m_pAudioFarEndPacketHeader->GetInformation(INF_LIVE_PACKETNUMBER);
		packetLength = m_pAudioFarEndPacketHeader->GetInformation(INF_LIVE_FRAME_LENGTH);
		timestamp = m_pAudioFarEndPacketHeader->GetInformation(INF_LIVE_TIMESTAMP);
		nEchoStateFlags = m_pAudioFarEndPacketHeader->GetInformation(INF_LIVE_ECHO_STATE_FLAGS);

		MediaLog(LOG_DEBUG, "[FE][AFEDP][ECHOFLAG] getting from header nEchoStateFlags = %d\n", nEchoStateFlags);		
	}


	bool AudioFarEndDataProcessor::IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType)
	{
		MediaLog(CODE_TRACE, "[FE] m_iRole = %d, nCurrentAudioPacketType = %d\n", m_nEntityType, nCurrentAudioPacketType);

		if (AUDIO_FLOW_AAC_LIVE_CHANNEL == m_nAudioFlowType)	//Channel
		{
			if (AUDIO_CHANNEL_PACKET_TYPE == nCurrentAudioPacketType)
			{
				return true;
			}
			return false;
		}
		else if (AUDIO_FLOW_OPUS_LIVE_CHANNEL == m_nAudioFlowType || AUDIO_FLOW_USELESS_STREAM == m_nAudioFlowType)	//LiveStreaming.
		{			
			if (m_pAudioCallSession->IsOpusEnable())	
			{
				if (ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType && LIVE_CALLEE_PACKET_TYPE_OPUS == nCurrentAudioPacketType)	//
				{					
					return true;
				}
				else if (ENTITY_TYPE_VIEWER == m_nEntityType && 
					(LIVE_CALLEE_PACKET_TYPE_OPUS == nCurrentAudioPacketType || LIVE_PUBLISHER_PACKET_TYPE_OPUS == nCurrentAudioPacketType))
				{
					return true;					
				}
				else if (ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType && LIVE_PUBLISHER_PACKET_TYPE_OPUS == nCurrentAudioPacketType)
				{
					return true;
				}
			}			
		}

		return false;
	}


	bool AudioFarEndDataProcessor::IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType, int nRelativeTimeOffset)
	{
#ifndef LOCAL_SERVER_LIVE_CALL
		if (m_bIsLiveStreamingRunning)
		{
			if (m_pLiveAudioParser->GetRoleChanging() == true)
			{
				return false;
			}

			if ( (AUDIO_FLOW_OPUS_LIVE_CHANNEL == m_nAudioFlowType || AUDIO_FLOW_USELESS_STREAM == m_nAudioFlowType) 
				&& (m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER
				|| m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_VIEWER_CALLEE
				|| m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_VIEWER) )
			{
				return true;
			}

			if (-1 == m_llDecodingTimeStampOffset)
			{
				Tools::SOSleep(LIVE_FIRST_FRAME_SLEEP_TIME_AUDIO);
				m_llDecodingTimeStampOffset = Tools::CurrentTimestamp() - llCurrentFrameRelativeTime + nRelativeTimeOffset;
				long long llNow = Tools::CurrentTimestamp();
				long long llExpectedEncodingTimeStamp = llNow - m_llDecodingTimeStampOffset;
				m_nExpectedNextPacketNumber = iPacketNumber++;
				MediaLog(LOG_DEBUG, "[FE][PD] Delay Packet not Discurding 1st packet ->  PacketNo:%d m_llDecodingTimeStampOffset = %d llExpectedEncodingTimeStamp = %lld",
					iPacketNumber, m_llDecodingTimeStampOffset, llExpectedEncodingTimeStamp);
			}
			else
			{
				long long llNow = Tools::CurrentTimestamp();
				long long llExpectedEncodingTimeStamp = llNow - m_llDecodingTimeStampOffset;
				long long llWaitingTime = llCurrentFrameRelativeTime - llExpectedEncodingTimeStamp;

				if (m_nExpectedNextPacketNumber < iPacketNumber)
				{
					m_nPacketsLost += iPacketNumber - m_nExpectedNextPacketNumber;
				}
				m_nExpectedNextPacketNumber = iPacketNumber + 1;
			
				MediaLog(LOG_DEBUG, "[FE][PD] Delay Packet Discurding ->  PacketNo:%d WaitingTime:%lld RelativeTime:%lld CurrentTime:%lld Delta:%lld, \
									llExpectedEncodingTimeStamp = %lld, llCurrentFrameRelativeTime = %lld", 
									iPacketNumber, llWaitingTime, llCurrentFrameRelativeTime, llNow, m_llDecodingTimeStampOffset, llExpectedEncodingTimeStamp, llCurrentFrameRelativeTime);

				if (llExpectedEncodingTimeStamp - __AUDIO_DELAY_TIMESTAMP_TOLERANCE__ > llCurrentFrameRelativeTime)
				{	
					MediaLog(LOG_DEBUG, "[FE][PD] discurding diff = %lld", llExpectedEncodingTimeStamp - llCurrentFrameRelativeTime);
					m_nPacketsRecvdNotTimely++;

					if (m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_VIEWER)
					{
						MediaLog(LOG_DEBUG, "[FE][PD] not discurding entity viewer = %lld", llExpectedEncodingTimeStamp - llCurrentFrameRelativeTime);
						return true; //Use delay frame for viewer.
					}
					else
					{
						MediaLog(LOG_DEBUG, "[FE][PD]  discurding entity not viewer = %lld", llExpectedEncodingTimeStamp - llCurrentFrameRelativeTime);
					}
					return false;

				}
				else if (llCurrentFrameRelativeTime - llExpectedEncodingTimeStamp > MAX_WAITING_TIME_FOR_CHANNEL)
				{
					MediaLog(LOG_DEBUG, "[FE][PD]  discurding else condition diff = %lld", llCurrentFrameRelativeTime - llExpectedEncodingTimeStamp);
					m_nPacketsRecvdNotTimely++;
					
					return false;
				}

				while (llExpectedEncodingTimeStamp + __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ < llCurrentFrameRelativeTime)
				{
					Tools::SOSleep(5);
					llExpectedEncodingTimeStamp = Tools::CurrentTimestamp() - m_llDecodingTimeStampOffset;
					
				}
				m_nPacketsRecvdTimely++;
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

#if 0
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
				else if (m_pAudioEncoder->GetCurrentBitrate() != OPUS_BITRATE_INIT_CALL){
					m_pAudioEncoder->SetBitrate(OPUS_BITRATE_INIT_CALL);
				}
			}
			m_iReceivedPacketsInCurrentSlot++;
		}
	}
#endif

	void AudioFarEndDataProcessor::PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
		long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime)
	{
		if (!m_bIsLiveStreamingRunning)
		{
			llNow = Tools::CurrentTimestamp();			
			if (llNow - llTimeStamp >= 1000)
			{				
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
			
			MediaLog(LOG_INFO, "[FE][AFEDP] @@@@------------------------>Bitrate: %d\n", nChangedBitRate);
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
		
					if (m_pAudioAlarmListener != nullptr && m_pAudioCallSession->m_bIsVideoCallRunning)
					{
						m_pAudioAlarmListener->FireAudioAlarm(AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO, 0, 0);
					}

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
#endif
	}

	void AudioFarEndDataProcessor::SyncPlayingTime()
	{
		long long llCurrentTimeStamp = Tools::CurrentTimestamp();
		
		MediaLog(LOG_DEBUG, "[FE][AFEDP][TS][POL] CurrnetTime=%lld, NextPlayTime=%lld, bPlayingStarted=%d", llCurrentTimeStamp, m_llNextPlayingTime, m_bPlayingNotStartedYet);

		if (m_bPlayingNotStartedYet)
		{			
			m_llNextPlayingTime = llCurrentTimeStamp + 100;
			m_bPlayingNotStartedYet = false;
			MediaLog(LOG_DEBUG, "[FE][AFEDP][TS] PlayingStarted !!!!! CurrentTime=%lld, NextPlayTime=%lld", Tools::CurrentTimestamp(), m_llNextPlayingTime);
		}
		else
		{			
			if (llCurrentTimeStamp + 20 < m_llNextPlayingTime)
			{				
				Tools::SOSleep(m_llNextPlayingTime - llCurrentTimeStamp - 20);
			}						
			m_llNextPlayingTime += 100;
		}
	}

	void AudioFarEndDataProcessor::ProcessPlayingData()
	{
		if (m_pAudioCallSession->m_pNearEndProcessor->IsEchoCancellerEnabled())
		{
			if (m_pAudioCallSession->m_bRecordingStarted)
			{
				if (m_pAudioCallSession->m_pNearEndProcessor->IsTraceSendingEnabled() && m_pAudioCallSession->m_pNearEndProcessor->m_bTraceTailRemains)
				{
					m_pAudioCallSession->m_pNearEndProcessor->m_bTraceTailRemains = m_pAudioCallSession->m_pNearEndProcessor->m_pTrace->GenerateTrace(m_saPlayingData, 800);
					MediaLog(LOG_DEBUG, "[FE][AFEDP][TS] Buffer Size = %d, TraceTailRemains = %d", m_pAudioCallSession->m_FarendBuffer->GetQueueSize(), m_pAudioCallSession->m_pNearEndProcessor->m_bTraceTailRemains);
				}

				if (!m_pAudioCallSession->m_pNearEndProcessor->m_bTraceSent)
				{
					m_pAudioCallSession->m_FarendBuffer->ResetBuffer();
					m_pAudioCallSession->m_pNearEndProcessor->m_llTraceSendingTime = Tools::CurrentTimestamp();
					m_pAudioCallSession->m_pNearEndProcessor->m_bTraceSent = true;
					MediaLog(LOG_DEBUG, "[FE][AFEDP][TS] TraceSent!!!!# Buffer Size=%d, TraceSendingTime=%d", m_pAudioCallSession->m_FarendBuffer->GetQueueSize(), m_pAudioCallSession->m_pNearEndProcessor->m_llTraceSendingTime);
				}

				if (m_pAudioCallSession->m_bEnablePlayerTimeSyncDuringEchoCancellation)
				{
					SyncPlayingTime();
				}

				long long llCurrentTimeStamp = Tools::CurrentTimestamp();
				if (m_pDataEventListener != nullptr)
				{
					MediaLog(LOG_CODE_TRACE, "[FE][AFEDP] To Player# Playing Time: %lld Next: %lld [%lld]\n", llCurrentTimeStamp, m_llNextPlayingTime, m_llNextPlayingTime - llCurrentTimeStamp);
					m_pDataEventListener->FireDataEvent(m_pAudioCallSession->m_nAudioServiceType, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false), m_saPlayingData);
					m_pAudioCallSession->m_pPlayedFE->WriteDump(m_saPlayingData, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false));
				}
				m_pAudioCallSession->m_FarendBuffer->EnQueue(m_saPlayingData, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false), 0);
				memset(m_saPlayingData, 0, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false) * sizeof(short));

			}
			else
			{
				Tools::SOSleep(20);
			}
		}
		else
		{
			if (m_pDataEventListener != nullptr)
			{
				m_pDataEventListener->FireDataEvent(m_pAudioCallSession->m_nAudioServiceType, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false), m_saPlayingData);
				m_pAudioCallSession->m_pPlayedFE->WriteDump(m_saPlayingData, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(false));
			}
		}
	}
	
	void AudioFarEndDataProcessor::SetEventCallback(DataEventListener* pDataListener, NetworkChangeListener* networkListener, AudioAlarmListener* alarmListener)
	{
		m_pDataEventListener = pDataListener;
		m_pNetworkChangeListener = networkListener;
		m_pAudioAlarmListener = alarmListener;
	}

} //namespace MediaSDK
