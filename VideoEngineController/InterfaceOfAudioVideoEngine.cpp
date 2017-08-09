
#include "Controller.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "MediaLogger.h"
#include "CommonMacros.h"

namespace MediaSDK
{

	CInterfaceOfAudioVideoEngine *G_pInterfaceOfAudioVideoEngine = nullptr;

	CController* m_pcController = nullptr;
	Tools m_Tools;

#ifdef LIVE_CHUNK_DUMPLINGS
	FILE *fp_live_data = nullptr;//, *fp_live_missing_vec = nullptr;
	unsigned char temp_buffer[12];
#endif

	CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine()
	{
		MediaLogInit(LOG_CODE_TRACE, false);

		G_pInterfaceOfAudioVideoEngine = this;
		m_pcController = nullptr;
		
		CController* pController = new CController();

		m_llTimeOffset = -1;

		pController->initializeEventHandler();

		//Late assignment to avoid transitional undefined behaviors
		m_pcController = pController;
	}

	CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine(const char* szLoggerPath, int nLoggerPrintLevel)
	{
		MediaLogInit(LOG_CODE_TRACE, false);

		m_pcController = nullptr;
			
		CController* pController = new CController(szLoggerPath, nLoggerPrintLevel);

		m_llTimeOffset = -1;

		pController->initializeEventHandler();

		//Late assignment to avoid transitional undefined behaviors
		m_pcController = pController;
	}

	bool CInterfaceOfAudioVideoEngine::Init(const IPVLongType& llUserID, const char* szLoggerPath, int nLoggerPrintLevel)
	{
		return true;
	}

	bool CInterfaceOfAudioVideoEngine::InitializeLibrary(const IPVLongType& llUserID)
	{
		return true;
	}

	CInterfaceOfAudioVideoEngine::~CInterfaceOfAudioVideoEngine()
	{
		if (nullptr != m_pcController)
		{

			CController* pController = m_pcController;
			
			m_pcController = nullptr;

			//Late destruction to avoid transitional UB
			delete pController;
		}

		MediaLogRelease();
	}

	bool CInterfaceOfAudioVideoEngine::SetUserName(const IPVLongType llUserName)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		m_pcController->initializeEventHandler();

		bool Ret = m_pcController->SetUserName(llUserName);

		return Ret;
	}

	bool CInterfaceOfAudioVideoEngine::StartAudioCall(const IPVLongType llFriendID, int nServiceType, int nEntityType, int nAudioSpeakerType)
	{
		m_llTimeOffset = -1;

		if (nullptr == m_pcController)
		{
			return false;
		}

		bool bReturnedValue = m_pcController->StartAudioCall(llFriendID, nServiceType, nEntityType, nAudioSpeakerType);

		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::SetVolume(const LongLong lFriendID, int iVolume, bool bRecorder)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		bool bReturnedValue = m_pcController->SetVolume(lFriendID, iVolume, bRecorder);
		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::SetSpeakerType(const LongLong lFriendID, int iSpeakerType)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		bool bReturnedValue = m_pcController->SetSpeakerType(lFriendID, iSpeakerType);
		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::SetEchoCanceller(const LongLong lFriendID, bool bOn)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		bool bReturnedValue = m_pcController->SetEchoCanceller(lFriendID, bOn);

		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::StartLiveStreaming(const IPVLongType llFriendID, int nEntityType, bool bAudioOnlyLive, int nVideoHeight, int nVideoWidth)
	{
#ifdef LIVE_CHUNK_DUMPLINGS
#if defined(__ANDROID__)
		std::string dpath = "/sdcard/";
#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
		std::string dpath = std::string(getenv("HOME")) + "/Documents/";
#elif defined(DESKTOP_C_SHARP)
		std::string dpath = "C:\\";
#endif
		dpath += "inf_" + Tools::LongLongToString(Tools::CurrentTimestamp() / 1000);
		std::string data_file = dpath + ".data";
		fp_live_data = fopen(data_file.c_str(), "wb");

		LOGE_MAIN("###PPP stalivest call filepath:%s", data_file.c_str());
#endif
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartLiveStreaming called 1 ID %lld", llFriendID);

		m_llTimeOffset = -1;

		if (nullptr == m_pcController)
		{
			return false;
		}

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartLiveStreaming called 2 ID %lld", llFriendID);

		bool bReturnedValue = m_pcController->StartAudioCall(llFriendID, SERVICE_TYPE_LIVE_STREAM, nEntityType, AUDIO_PLAYER_LOUDSPEAKER);

		if (bReturnedValue)
			bReturnedValue = m_pcController->StartVideoCall(llFriendID, nVideoHeight, nVideoWidth, SERVICE_TYPE_LIVE_STREAM, nEntityType, NETWORK_TYPE_NOT_2G, bAudioOnlyLive, false);

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartLiveStreaming StartVideoCall completed ID %lld", llFriendID);

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartLiveStreaming done ID %lld", llFriendID);

		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::StartChannelView(const IPVLongType llFriendID)
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartChannelView called 1 ID %lld", llFriendID);

		m_llTimeOffset = -1;

		if (nullptr == m_pcController)
		{
			return false;
		}

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartChannelView called 2 ID %lld", llFriendID);

		bool bReturnedValue = m_pcController->StartAudioCall(llFriendID, SERVICE_TYPE_CHANNEL, ENTITY_TYPE_VIEWER, AUDIO_PLAYER_DEFAULT);

		if (bReturnedValue)
			bReturnedValue = m_pcController->StartVideoCall(llFriendID, 352, 288, SERVICE_TYPE_CHANNEL, ENTITY_TYPE_VIEWER, NETWORK_TYPE_NOT_2G, false, false);

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartChannelView StartVideoCall completed ID %lld", llFriendID);

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartChannelView done ID %lld", llFriendID);
			
		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::StartVideoCall(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nServiceType, int nEntityType, int nNetworkType, bool bAudioOnlyLive)
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartVideoCall called 1 ID %lld", llFriendID);

		m_llTimeOffset = -1;
		bool bSelfViewOnly = false;

		if (nullptr == m_pcController)
		{
			return false;
		}

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartVideoCall called 2 ID %lld", llFriendID);

		if (llFriendID == SESSION_ID_FOR_SELF_VIEW)
			bSelfViewOnly = true;

		bool bReturnedValue = m_pcController->StartVideoCall(llFriendID, nVideoHeight, nVideoWidth, nServiceType, nEntityType, nNetworkType, bAudioOnlyLive, bSelfViewOnly);

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StartVideoCall done ID %lld", llFriendID);
		
		return bReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::EncodeAndTransfer(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->EncodeVideoFrame(llFriendID, in_data, unLength);

		return iReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::PushPacketForDecoding(const IPVLongType llFriendID, int mediaType, int nEntityType, unsigned char *in_data, unsigned int unLength)
	{
		HITLER("#@#@26022017# RECEIVING DATA FOR BOKKOR %u", unLength);
		std::vector< std::pair<int, int> > vMissingFrames;

		return PushAudioForDecodingVector(llFriendID, mediaType, nEntityType, in_data, unLength, vMissingFrames);
	}

	int CInterfaceOfAudioVideoEngine::PushAudioForDecodingVector(const IPVLongType llFriendID, int mediaType, int nEntityType, unsigned char *in_data, unsigned int unLength, std::vector< std::pair<int, int> > vMissingFrames)
	{
		HITLER("#@#@26022017# RECEIVING DATA FOR BOKKOR %u", unLength);
		int iReturnedValue = 0;

#ifdef LIVE_CHUNK_DUMPLINGS
		Tools::ConvertToCharArray(temp_buffer, vMissingFrames.size(), 4);
		fwrite(temp_buffer, 1, 4, fp_live_data);

		for (auto par : vMissingFrames){
			Tools::ConvertToCharArray(temp_buffer, par.first, 4);
			Tools::ConvertToCharArray(temp_buffer + 4, par.second, 4);
			fwrite(temp_buffer, 1, 8, fp_live_data);
		}

		Tools::ConvertToCharArray(temp_buffer, Tools::CurrentTimestamp(), 8);
		Tools::ConvertToCharArray(temp_buffer + 8, unLength, 4);
		fwrite(temp_buffer, 1, 12, fp_live_data);
		fwrite(in_data, 1, unLength, fp_live_data);
#endif


		LOGE_MAIN("##KK len:%d", (int)unLength);
		for (auto a : vMissingFrames){
			LOGE_MAIN("##KKKKK left:%d right:%d", a.first, a.second);
		}

		if (nullptr == m_pcController)
		{
			return 0;
		}
		else if (nullptr == in_data)
		{
			CLogPrinter_Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushAudioForDecoding null data from connectivity");

			return 0;
		}
		else if (NULL == in_data)
		{
			CLogPrinter_Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushAudioForDecoding null data from connectivity");

			return 0;
		}
		else
		{
			//		if (VIDEO_PACKET_MEDIA_TYPE == (int)in_data[0])
			//        {
			//            iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data, unLength);
			//        }
			//		else if (AUDIO_PACKET_MEDIA_TYPE == (int)in_data[0])
			//        {
			//            iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, in_data, unLength);
			//        }
			//		else
			//			return 0;

			if ((mediaType == MEDIA_TYPE_LIVE_STREAM && (nEntityType == ENTITY_TYPE_VIEWER || nEntityType == ENTITY_TYPE_VIEWER_CALLEE)) || ((mediaType == MEDIA_TYPE_LIVE_CALL_AUDIO || mediaType == MEDIA_TYPE_LIVE_CALL_VIDEO) && nEntityType == ENTITY_TYPE_PUBLISHER_CALLER))
			{
				//int lengthOfVideoData = m_Tools.UnsignedCharToIntConversion(in_data, 0);
				//int lengthOfAudioData = m_Tools.UnsignedCharToIntConversion(in_data, 4);

				/*int headerPosition;

				for (headerPosition = 0; numberOfMissingFrames > headerPosition && missingFrames[headerPosition] == headerPosition; headerPosition ++ )
				{
				if (headerPosition == NUMBER_OF_HEADER_FOR_STREAMING)
				return 5;
				}

				if(headerPosition >= NUMBER_OF_HEADER_FOR_STREAMING)
				return 6;*/

				int nValidHeaderOffset = 0;

				long long itIsNow = m_Tools.CurrentTimestamp();
				long long recvTimeOffset = m_Tools.GetMediaUnitTimestampInMediaChunck(in_data + nValidHeaderOffset);

				//LOGE("##DE#Interface## now %lld peertimestamp %lld timediff %lld relativediff %lld", itIsNow, recvTimeOffset, itIsNow - m_llTimeOffset, recvTimeOffset);

				if (m_llTimeOffset == -1)
				{
					m_llTimeOffset = itIsNow - recvTimeOffset;
					m_pcController->m_llLastTimeStamp = recvTimeOffset;
					HITLER("##DE#interface*first# timestamp:%lld recv:%lld", m_llTimeOffset, recvTimeOffset);
				}
				else
				{
					long long expectedTime = itIsNow - m_llTimeOffset;
					int tmp_headerLength = m_Tools.GetMediaUnitHeaderLengthFromMediaChunck(in_data + nValidHeaderOffset);
					int tmp_chunkDuration = m_Tools.GetMediaUnitChunkDurationFromMediaChunck(in_data + nValidHeaderOffset);
					HITLER("@#DE#Interface##now:%lld peertimestamp:%lld expected:%lld  [%lld] CHUNK_DURA = %d HEAD_LEN = %d ", itIsNow, recvTimeOffset, expectedTime, recvTimeOffset - expectedTime, tmp_chunkDuration, tmp_headerLength);

					if (recvTimeOffset < expectedTime - CHUNK_DELAY_TOLERANCE) {
						if (!m_pcController->IsCallInLiveEnabled())
						{
							//HITLER("##Discarding packet! | expected:%lld", expectedTime);
							//return -10;
						}
					}
					if (m_pcController->m_llLastTimeStamp >= recvTimeOffset) {
						// HITLER("#@#@26022017# RECEIVING DATA FOR BOKKOR %u", unLength);
						HITLER("#@#@26022017# ##Interface discarding duplicate packet.");
						return -10;
					}

					m_pcController->m_llLastTimeStamp = max(m_pcController->m_llLastTimeStamp, recvTimeOffset);
				}

				int version = m_Tools.GetMediaUnitVersionFromMediaChunck(in_data + nValidHeaderOffset);

				int headerLength = m_Tools.GetMediaUnitHeaderLengthFromMediaChunck(in_data + nValidHeaderOffset);
				int chunkDuration = m_Tools.GetMediaUnitChunkDurationFromMediaChunck(in_data + nValidHeaderOffset);

				HITLER("##@@@TTTTTTheaderLength %d chunkDuration %d\n", headerLength, chunkDuration);

				int lengthOfAudioData = m_Tools.GetAudioBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);
				int lengthOfVideoData = m_Tools.GetVideoBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);

				CLogPrinter_LOG(CRASH_CHECK_LOG || CHUNK_RECIVE_LOG, "CInterfaceOfAudioVideoEngine::PushAudioForDecodingVector headerLength %d lengthOfAudioData %d lengthOfVideoData %d unLength %d equality %d", headerLength, lengthOfAudioData, lengthOfVideoData, (int)unLength, (headerLength + lengthOfAudioData + lengthOfVideoData) == (int)unLength);

				//LOGEF("THeKing--> interface:receive ############## lengthOfVideoData =  %d  Pos=%d   Offset= %d,  \n", lengthOfVideoData,headerPosition, nValidHeaderOffset);

				int audioFrameSizes[200];
				int videoFrameSizes[150];

				int blockInfoPosition = m_Tools.GetMediaUnitBlockInfoPositionFromMediaChunck(in_data + nValidHeaderOffset);

				int numberOfAudioFrames = m_Tools.GetNumberOfAudioFramesFromMediaChunck(blockInfoPosition, in_data + nValidHeaderOffset);

				int index = blockInfoPosition + LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE;

				for (int i = 0; i < numberOfAudioFrames; i++)
				{
					audioFrameSizes[i] = m_Tools.GetNextAudioFramePositionFromMediaChunck(index, in_data + nValidHeaderOffset);

					index += LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE;
				}

				int numberOfVideoFrames = m_Tools.GetNumberOfVideoFramesFromMediaChunck(index, in_data + nValidHeaderOffset);

				index += LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE;

				for (int i = 0; i < numberOfVideoFrames; i++)
				{
					videoFrameSizes[i] = m_Tools.GetNextAudioFramePositionFromMediaChunck(index, in_data + nValidHeaderOffset);

					index += LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE;
				}

				HITLER("#TTTTTTTTTTTTaac#b4q# GotNumberOfAudioFrames: %d, numberOfVideoFrames: %d, missingVectorSize: %d, audioDataSize: %d", numberOfAudioFrames, numberOfVideoFrames, vMissingFrames.size(), lengthOfAudioData);

				int audioStartingPosition = m_Tools.GetAudioBlockStartingPositionFromMediaChunck(in_data + nValidHeaderOffset);
				int videoStartingPosition = m_Tools.GetVideoBlockStartingPositionFromMediaChunck(in_data + nValidHeaderOffset);

				int streamType = m_Tools.GetMediaUnitStreamTypeFromMediaChunck(in_data + nValidHeaderOffset);

				HITLER("TTTTTTTTTTTaudioStartingPosition %d videoStartingPosition %d streamType %d\n", audioStartingPosition, videoStartingPosition, streamType);

				iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, audioStartingPosition, in_data, lengthOfAudioData, numberOfAudioFrames, audioFrameSizes, vMissingFrames);

				//m_Tools.SOSleep(100); //Temporary Fix to Sync Audio And Video Data for LIVE STREAM SERVICE
#ifndef DISABLE_VIDEO_FOR_LIVE
				iReturnedValue = m_pcController->PushPacketForDecodingVector(llFriendID, videoStartingPosition, in_data + videoStartingPosition, lengthOfVideoData, numberOfVideoFrames, videoFrameSizes, vMissingFrames);
#endif

			}
			else if (mediaType == MEDIA_TYPE_AUDIO || mediaType == MEDIA_TYPE_VIDEO)
			{
				if (AUDIO_PACKET_MEDIA_TYPE == (int)in_data[0])
				{
					iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, 0, in_data + 1, unLength - 1, 0, NULL, vMissingFrames); //Skip First byte for Audio Data
				}
				else if (VIDEO_PACKET_MEDIA_TYPE == (int)in_data[0])
				{
					iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data + 1, unLength - 1); //Skip First byte for Video Data
				}
				else
					return 0;
			}
		}

		return iReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::SendAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->SendAudioData(llFriendID, in_data, unLength);

		return iReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::CancelAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->CancelAudioData(llFriendID, in_data, unLength);

		return iReturnedValue;
	}


	int CInterfaceOfAudioVideoEngine::SendVideoData(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength, unsigned int nOrientationType, int device_orientation)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->SendVideoData(llFriendID, in_data, unLength, nOrientationType, device_orientation);

		return iReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::SetEncoderHeightWidth(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nDataType)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->SetEncoderHeightWidth(llFriendID, nVideoHeight, nVideoWidth, nDataType);

		return iReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::SetDeviceDisplayHeightWidth(int nVideoHeight, int nVideoWidth)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->SetDeviceDisplayHeightWidth(nVideoHeight, nVideoWidth);

		return iReturnedValue;
	}

    int CInterfaceOfAudioVideoEngine::SetBeautification(const IPVLongType llFriendID, bool bIsEnable)
    {
        if (nullptr == m_pcController)
        {
            return false;
        }
        
        int iReturnedValue = m_pcController->SetBeautification(llFriendID, bIsEnable);
        
        return iReturnedValue;
    }
    
	int CInterfaceOfAudioVideoEngine::SetVideoEffect(const IPVLongType llFriendID, int nEffectStatus)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->SetVideoEffect(llFriendID, nEffectStatus);

		return iReturnedValue;
	}

	/*
		param array documentation
		param[0] = sigma
		param[1] = radius
		param[2] = effect value (*not in use now)
		param[3] = to or not to run beautification
		param[4] = pixel from where intensity should be increased for brightness
		param[5] = pixel upto where intensity should be increased for brightness
		param[6] = peak of intesity increase
		param[7] = peak value of intensity change
		*/

	int CInterfaceOfAudioVideoEngine::TestVideoEffect(const IPVLongType llFriendID, int *param, int size)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->TestVideoEffect(llFriendID, param, size);

		return iReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::SetBitRate(const IPVLongType llFriendID, int nBitRate)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->SetBitRate(llFriendID, nBitRate);

		return iReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::CheckDeviceCapability(const LongLong& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::CheckDeviceCapability called 1 ID %lld", lFriendID);

		if (nullptr == m_pcController)
		{
			return false;
		}

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::CheckDeviceCapability called 2 ID %lld", lFriendID);

		int iReturnedValue = m_pcController->CheckDeviceCapability(lFriendID, iHeightHigh, iWidthHigh, iHeightLow, iWidthLow);

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::CheckDeviceCapability done ID %lld", lFriendID);
		
		return iReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		int iReturnedValue = m_pcController->SetDeviceCapabilityResults(iNotification, iHeightHigh, iWidthHigh, iHeightLow, iWidthLow);

		return iReturnedValue;

	}

	bool CInterfaceOfAudioVideoEngine::StopAudioCall(const IPVLongType llFriendID)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		bool bReturnedValue = m_pcController->StopAudioCall(llFriendID);

		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::StopVideoCall(const IPVLongType llFriendID)
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StopVideoCall called 1 ID %lld", llFriendID);

		if (nullptr == m_pcController)
		{
			return false;
		}

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StopVideoCall called 2 ID %lld", llFriendID);

		bool bReturnedValue = m_pcController->StopVideoCall(llFriendID);

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::StopVideoCall done ID %lld", llFriendID);

#ifdef LIVE_CHUNK_DUMPLINGS
		if(fp_live_data) 
		{
			fclose(fp_live_data);
			fp_live_data = nullptr;
		}
		LOGE_MAIN("###PPP stopvideocallcalled.");
#endif
		
		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::SetLoggingState(bool bLoggingState, int nLogLevel)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		bool bReturnedValue = m_pcController->SetLoggingState(bLoggingState, nLogLevel);

		return bReturnedValue;
	}

	void CInterfaceOfAudioVideoEngine::UninitializeLibrary()
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::UninitializeLibrary called 1");

		if (nullptr != m_pcController)
		{
			CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::UninitializeLibrary called 2");

			m_pcController->UninitializeLibrary();
		}

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::UninitializeLibrary done");
	}

	void CInterfaceOfAudioVideoEngine::SetLoggerPath(std::string strLoggerPath)
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetLoggerPath(strLoggerPath);
		}
	}

	int CInterfaceOfAudioVideoEngine::StartAudioEncodeDecodeSession()
	{
		int nReturnedValue = 0;

		if (nullptr != m_pcController)
		{
			nReturnedValue = m_pcController->StartAudioEncodeDecodeSession();
		}

		return nReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer)
	{
		int nReturnedValue = 0;

		if (nullptr != m_pcController)
		{
			nReturnedValue = m_pcController->EncodeAudioFrame(psaEncodingDataBuffer, nAudioFrameSize, ucaEncodedDataBuffer);
		}

		return nReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer)
	{
		int nReturnedValue = 0;

		if (nullptr != m_pcController)
		{
			nReturnedValue = m_pcController->DecodeAudioFrame(ucaDecodedDataBuffer, nAudioFrameSize, psaDecodingDataBuffer);
		}

		return nReturnedValue;
	}

	int CInterfaceOfAudioVideoEngine::StopAudioEncodeDecodeSession()
	{
		int nReturnedValue = 0;

		if (nullptr != m_pcController)
		{
			nReturnedValue = m_pcController->StopAudioEncodeDecodeSession();
		}

		return nReturnedValue;
	}


	int CInterfaceOfAudioVideoEngine::StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data, int iLen, int nVideoHeight, int nVideoWidth)
	{
		int nReturnedValue = 0;

		if (nullptr != m_pcController)
		{
			nReturnedValue = m_pcController->StartVideoMuxingAndEncodeSession(pBMP32Data, iLen, nVideoHeight, nVideoWidth);
		}

		return nReturnedValue;

	}

	int CInterfaceOfAudioVideoEngine::FrameMuxAndEncode(unsigned char *pVideoYuv, int iHeight, int iWidth)
	{

		int nReturnedValue = 0;

		if (nullptr != m_pcController)
		{
			nReturnedValue = m_pcController->FrameMuxAndEncode(pVideoYuv, iHeight, iWidth);
		}

		return nReturnedValue;

	}

	int CInterfaceOfAudioVideoEngine::StopVideoMuxingAndEncodeSession(unsigned char *finalData)
	{
		int nReturnedValue = 0;

		if (nullptr != m_pcController)
		{
			nReturnedValue = m_pcController->StopVideoMuxingAndEncodeSession(finalData);
		}

		return nReturnedValue;

	}

	void CInterfaceOfAudioVideoEngine::InterruptOccured(const LongLong lFriendID)
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::InterruptOccured called 1 ID %lld", lFriendID);

		if (nullptr != m_pcController)
		{
			CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::InterruptOccured called 2 ID %lld", lFriendID);

			m_pcController->InterruptOccured(lFriendID);
		}

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::InterruptOccured done ID %lld", lFriendID);
	}

	void CInterfaceOfAudioVideoEngine::InterruptOver(const LongLong lFriendID)
	{
		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::InterruptOver called 1 ID %lld", lFriendID);

		if (nullptr != m_pcController)
		{
			CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::InterruptOver called 2 ID %lld", lFriendID);

			m_pcController->InterruptOver(lFriendID);
		}

		CLogPrinter_LOG(API_FLOW_CHECK_LOG, "CInterfaceOfAudioVideoEngine::InterruptOver done ID %lld", lFriendID);
	}

	void CInterfaceOfAudioVideoEngine::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetNotifyClientWithPacketCallback(callBackFunctionPointer);
		}
	}



	void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int, int, int))
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetNotifyClientWithVideoDataCallback(callBackFunctionPointer);
		}
	}


	void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int))
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetNotifyClientWithVideoNotificationCallback(callBackFunctionPointer);
		}
	}

	void CInterfaceOfAudioVideoEngine::SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int))
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetNotifyClientWithNetworkStrengthNotificationCallback(callBackFunctionPointer);
		}
	}

	void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, int, short*, int))
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetNotifyClientWithAudioDataCallback(callBackFunctionPointer);
		}
	}

	void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetNotifyClientWithAudioPacketDataCallback(callBackFunctionPointer);
		}
	}

	void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetNotifyClientWithAudioAlarmCallback(callBackFunctionPointer);
		}
	}

	void CInterfaceOfAudioVideoEngine::SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int, int, std::vector< std::pair<int, int> > vAudioBlocks))
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetSendFunctionPointer(callBackFunctionPointer);
		}
	}

	bool CInterfaceOfAudioVideoEngine::StartCallInLive(const IPVLongType llFriendID, int iRole, int nCallInLiveType)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		m_llTimeOffset = -1;

		int nCalleeID = 1;

		bool bReturnedValue = m_pcController->StartAudioCallInLive(llFriendID, iRole, nCallInLiveType);

		m_pcController->SetCallInLiveEnabled(true);

		if (bReturnedValue)
		{
			bReturnedValue = m_pcController->StartVideoCallInLive(llFriendID, nCallInLiveType, nCalleeID);
		}

		return bReturnedValue;
	}

	bool CInterfaceOfAudioVideoEngine::EndCallInLive(const IPVLongType llFriendID)
	{
		if (nullptr == m_pcController)
		{
			return false;
		}

		m_llTimeOffset = -1;

		int nCalleeID = 1;

		bool bReturnedValue = m_pcController->EndAudioCallInLive(llFriendID);

		m_pcController->SetCallInLiveEnabled(false);

		if (bReturnedValue)
		{
			bReturnedValue = m_pcController->EndVideoCallInLive(llFriendID, nCalleeID);
		}

		return bReturnedValue;
	}

	void CInterfaceOfAudioVideoEngine::SetCallInLiveType(const IPVLongType llFriendID, int nCallInLiveType)
	{
		if (nullptr != m_pcController)
		{
			m_pcController->SetCallInLiveType(llFriendID, nCallInLiveType);
		}
	}

std::string CInterfaceOfAudioVideoEngine::GetMediaEngineVersion()
{
    
    return "MediaEngine Version "+ std::string(MEDIA_ENGINE_VERSION)+" (compiled "+string(__TIME__)+", "+string(__DATE__)+")";
}


} //namespace MediaSDK

