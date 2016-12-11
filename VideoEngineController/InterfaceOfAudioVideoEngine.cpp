
#include "Controller.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "LogPrinter.h"

CInterfaceOfAudioVideoEngine *G_pInterfaceOfAudioVideoEngine = NULL;

CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine()
{
	G_pInterfaceOfAudioVideoEngine = this;
	m_pcController = new CController();

	m_pcController->initializeEventHandler();
}

CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine(const char* szLoggerPath, int nLoggerPrintLevel)
{
	m_pcController = new CController(szLoggerPath, nLoggerPrintLevel);

	m_pcController->initializeEventHandler();
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
	if (NULL != m_pcController)
	{
		delete m_pcController;

		m_pcController = NULL;
	}
}

bool CInterfaceOfAudioVideoEngine::SetUserName(const IPVLongType llUserName)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	m_pcController->initializeEventHandler();

	bool Ret = m_pcController->SetUserName(llUserName);

	return Ret;
}

bool CInterfaceOfAudioVideoEngine::StartAudioCall(const IPVLongType llFriendID , int nServiceType)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StartAudioCall(llFriendID, nServiceType);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetVolume(const LongLong lFriendID, int iVolume)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->SetVolume(lFriendID, iVolume);
    return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetLoudSpeaker(const LongLong lFriendID, bool bOn)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->SetLoudSpeaker(lFriendID, bOn);
    return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::StartVideoCall(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nServiceType, int packetSizeOfNetwork, int nNetworkType)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	m_pcController->m_pCommonElementsBucket->SetPacketSizeOfNetwork(packetSizeOfNetwork);

	bool bReturnedValue = m_pcController->StartVideoCall(llFriendID, nVideoHeight, nVideoWidth, nServiceType, nNetworkType);	

	return bReturnedValue;
}

int CInterfaceOfAudioVideoEngine::EncodeAndTransfer(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->EncodeVideoFrame(llFriendID, in_data, unLength);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::PushPacketForDecoding(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength)
{   
	return -1;
}

int CInterfaceOfAudioVideoEngine::PushAudioForDecodingVector(const IPVLongType llFriendID, int mediaType, unsigned char *in_data, unsigned int unLength, std::vector< std::pair<int, int> > vMissingFrames)
{
	int iReturnedValue = 0;

	if (NULL == m_pcController)
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

		if (mediaType == MEDIA_TYPE_LIVE_STREAM)
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

			int version = m_Tools.GetMediaUnitVersionFromMediaChunck(in_data + nValidHeaderOffset);

			int lengthOfAudioData = m_Tools.GetAudioBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);
			int lengthOfVideoData = m_Tools.GetVideoBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);

			//LOGEF("THeKing--> interface:receive ############## lengthOfVideoData =  %d  Pos=%d   Offset= %d,  \n", lengthOfVideoData,headerPosition, nValidHeaderOffset);

			int audioFrameSizes[100];
			int videoFrameSizes[100];

			int numberOfAudioFrames = m_Tools.GetNumberOfAudioFramesFromMediaChunck(LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION, in_data + nValidHeaderOffset);

			int index = LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION + LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE;

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

			iReturnedValue = m_pcController->PushAudioForDecodingVector(llFriendID, lengthOfVideoData + index, in_data, lengthOfAudioData, numberOfAudioFrames, audioFrameSizes, vMissingFrames);

			//m_Tools.SOSleep(100); //Temporary Fix to Sync Audio And Video Data for LIVE STREAM SERVICE

			iReturnedValue = m_pcController->PushPacketForDecodingVector(llFriendID, index, in_data + index, lengthOfVideoData, numberOfVideoFrames, videoFrameSizes, vMissingFrames);
		}
		else
		{
			if (VIDEO_PACKET_MEDIA_TYPE == (int)in_data[0])
			{
				iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data + 1, unLength - 1); //Skip First byte for Video Data
			}
			else if (AUDIO_PACKET_MEDIA_TYPE == (int)in_data[0])
			{
				iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, 0, in_data + 1, unLength - 1); //Skip First byte for Audio Data
			}
			else
				return 0;
		}
	}

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::PushAudioForDecoding(const IPVLongType llFriendID, int mediaType, unsigned char *in_data, unsigned int unLength, int numberOfMissingFrames, int *missingFrames)
{ 
    int iReturnedValue = 0;
	int packetSizeOfNetwork = m_pcController->m_pCommonElementsBucket->GetPacketSizeOfNetwork();
    
	if (NULL == m_pcController)
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

        if(mediaType == MEDIA_TYPE_LIVE_STREAM)
        {           
            //int lengthOfVideoData = m_Tools.UnsignedCharToIntConversion(in_data, 0);
            //int lengthOfAudioData = m_Tools.UnsignedCharToIntConversion(in_data, 4);

			if (packetSizeOfNetwork < 0)
				return 0;
            
            int headerPosition;
            
            for (headerPosition = 0; numberOfMissingFrames > headerPosition && missingFrames[headerPosition] == headerPosition; headerPosition ++ )
            {
                if (headerPosition == NUMBER_OF_HEADER_FOR_STREAMING)
                    return 5;
            }
            
            if(headerPosition >= NUMBER_OF_HEADER_FOR_STREAMING)
                return 6;
            
            int nValidHeaderOffset = headerPosition * packetSizeOfNetwork;
            
            int version = m_Tools.GetMediaUnitVersionFromMediaChunck(in_data + nValidHeaderOffset);

			int timeStamp = m_Tools.GetMediaUnitTimestampInMediaChunck(in_data + nValidHeaderOffset);

			//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "CInterfaceOfAudioVideoEngine::PushAudioForDecoding " + m_Tools.IntegertoStringConvert(timeStamp));
          
            int lengthOfAudioData = m_Tools.GetAudioBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);
            int lengthOfVideoData = m_Tools.GetVideoBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);
            
            //LOGEF("THeKing--> interface:receive ############## lengthOfVideoData =  %d  Pos=%d   Offset= %d,  \n", lengthOfVideoData,headerPosition, nValidHeaderOffset);
            
            int audioFrameSizes[100];
            int videoFrameSizes[100];
            
            int numberOfAudioFrames = m_Tools.GetNumberOfAudioFramesFromMediaChunck(LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION, in_data + nValidHeaderOffset);
            
            int index = LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION + LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE;
            
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
            
            LOGS("#LV# ------------------------> nMissing: " + Tools::IntegertoStringConvert(numberOfMissingFrames)
                 + "  AudioStartLen: " + Tools::IntegertoStringConvert(lengthOfVideoData + packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING));
            for (int i = 0; i < numberOfMissingFrames; i++)
                LOGS("#LV# StartPosOfMissingPacket : " + Tools::IntegertoStringConvert(packetSizeOfNetwork * missingFrames[i]));
            

            iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, lengthOfVideoData + packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING,
                                                                  in_data, lengthOfAudioData, numberOfAudioFrames, audioFrameSizes, numberOfMissingFrames, missingFrames);

			//m_Tools.SOSleep(100); //Temporary Fix to Sync Audio And Video Data for LIVE STREAM SERVICE

			iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data + packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING,
																   lengthOfVideoData, numberOfVideoFrames, videoFrameSizes, numberOfMissingFrames, missingFrames);
        }
        else
        {
            
            if (VIDEO_PACKET_MEDIA_TYPE == (int)in_data[0])
        	{
            	iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data+1, unLength-1); //Skip First byte for Video Data
        	}
			else if (AUDIO_PACKET_MEDIA_TYPE == (int)in_data[0])
        	{
            	iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, 0, in_data+1, unLength-1); //Skip First byte for Audio Data
        	}
        	else
            	return 0;
            
        }

    }

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SendAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SendAudioData(llFriendID, in_data, unLength);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SendVideoData(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength, unsigned int nOrientationType, int device_orientation)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SendVideoData(llFriendID, in_data, unLength, nOrientationType, device_orientation);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetEncoderHeightWidth(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SetEncoderHeightWidth(llFriendID, nVideoHeight, nVideoWidth);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetBitRate(const IPVLongType llFriendID, int nBitRate)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SetBitRate(llFriendID, nBitRate);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::CheckDeviceCapability(const LongLong& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->CheckDeviceCapability(lFriendID, iHeightHigh, iWidthHigh, iHeightLow, iWidthLow);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
{
    if(NULL == m_pcController)
    {
        return false;
    }
    
    int iReturnedValue = m_pcController->SetDeviceCapabilityResults(iNotification, iHeightHigh, iWidthHigh, iHeightLow, iWidthLow);
    
    return iReturnedValue;
    
}

bool CInterfaceOfAudioVideoEngine::StopAudioCall(const IPVLongType llFriendID)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StopAudioCall(llFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::StopVideoCall(const IPVLongType llFriendID)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StopVideoCall(llFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetLoggingState(bool bLoggingState, int nLogLevel)
{
	if (NULL == m_pcController)
    {
        return false;
    }
    
    bool bReturnedValue = m_pcController->SetLoggingState(bLoggingState, nLogLevel);
    
    return bReturnedValue;
}

void CInterfaceOfAudioVideoEngine::UninitializeLibrary()
{
	if (NULL != m_pcController)
	{
		m_pcController->UninitializeLibrary();
	}
}

void CInterfaceOfAudioVideoEngine::SetLoggerPath(std::string strLoggerPath)
{
	if (NULL != m_pcController)
	{
		m_pcController->SetLoggerPath(strLoggerPath);
	}
}

int CInterfaceOfAudioVideoEngine::StartAudioEncodeDecodeSession()
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->StartAudioEncodeDecodeSession();
	}

	return nReturnedValue;
}

int CInterfaceOfAudioVideoEngine::EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer)
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->EncodeAudioFrame(psaEncodingDataBuffer, nAudioFrameSize, ucaEncodedDataBuffer);
	}

	return nReturnedValue;
}

int CInterfaceOfAudioVideoEngine::DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer)
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->DecodeAudioFrame(ucaDecodedDataBuffer, nAudioFrameSize, psaDecodingDataBuffer);
	}

	return nReturnedValue;
}

int CInterfaceOfAudioVideoEngine::StopAudioEncodeDecodeSession()
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->StopAudioEncodeDecodeSession();
	}

	return nReturnedValue;
}


int CInterfaceOfAudioVideoEngine::StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data,int iLen, int nVideoHeight, int nVideoWidth)
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->StartVideoMuxingAndEncodeSession(pBMP32Data, iLen, nVideoHeight, nVideoWidth);
	}

	return nReturnedValue;

}

int CInterfaceOfAudioVideoEngine::FrameMuxAndEncode( unsigned char *pVideoYuv, int iHeight, int iWidth, unsigned char *pMergedData)
{

	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->FrameMuxAndEncode(pVideoYuv, iHeight, iWidth, pMergedData);
	}

	return nReturnedValue;

}

int CInterfaceOfAudioVideoEngine::StopVideoMuxingAndEncodeSession()
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->StopVideoMuxingAndEncodeSession();
	}

	return nReturnedValue;

}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithPacketCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithVideoDataCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithVideoNotificationCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithNetworkStrengthNotificationCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, int, short*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithAudioDataCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithAudioPacketDataCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithAudioAlarmCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int, int))
{
    if (NULL != m_pcController)
    {
        m_pcController->SetSendFunctionPointer(callBackFunctionPointer);
    }
}

