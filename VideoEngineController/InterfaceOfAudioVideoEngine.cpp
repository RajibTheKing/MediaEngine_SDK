
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

bool CInterfaceOfAudioVideoEngine::StartAudioCall(const IPVLongType llFriendID)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StartAudioCall(llFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::StartVideoCall(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nNetworkType)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StartVideoCall(llFriendID, nVideoHeight, nVideoWidth,nNetworkType);

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

int CInterfaceOfAudioVideoEngine::PushAudioForDecoding(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength, int numberOfMissingFrames, int *missingFrames)
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

#ifdef ONLY_FOR_LIVESTREAMING

		//int lengthOfVideoData = m_Tools.UnsignedCharToIntConversion(in_data, 0);
		//int lengthOfAudioData = m_Tools.UnsignedCharToIntConversion(in_data, 4);

		int headerPosition;

		for (headerPosition = 0; numberOfMissingFrames > headerPosition && missingFrames[headerPosition] == 0; headerPosition++)
		{
			if (headerPosition == NUMBER_OF_HEADER_FOR_STREAMING)
				return 5;
		}

		int version = m_Tools.GetMediaUnitVersionFromMediaChunck(in_data + headerPosition);

		int lengthOfAudioData = m_Tools.GetAudioBlockSizeFromMediaChunck(in_data + headerPosition);
		int lengthOfVideoData = m_Tools.GetVideoBlockSizeFromMediaChunck(in_data + headerPosition);

		int audioFrameSizes[100];
		int videoFrameSizes[100];

		int numberOfAudioFrames = m_Tools.GetNumberOfAudioFramesFromMediaChunck(LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION, in_data + headerPosition);

		int index = LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION + LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE;

		for (int i = 0; i < numberOfAudioFrames; i++)
		{
			audioFrameSizes[i] = m_Tools.GetNextAudioFramePositionFromMediaChunck(index, in_data + headerPosition);

			index += LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE;
		}

		int numberOfVideoFrames = m_Tools.GetNumberOfVideoFramesFromMediaChunck(index, in_data + headerPosition);

		index += LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE;

		for (int i = 0; i < numberOfVideoFrames; i++)
		{
			videoFrameSizes[i] = m_Tools.GetNextAudioFramePositionFromMediaChunck(index, in_data + headerPosition);

			index += LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE;
		}

		iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data + __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ * NUMBER_OF_HEADER_FOR_STREAMING, lengthOfVideoData, numberOfAudioFrames, audioFrameSizes, numberOfMissingFrames, missingFrames);
		iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, lengthOfVideoData + __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ * NUMBER_OF_HEADER_FOR_STREAMING, in_data, lengthOfAudioData, numberOfAudioFrames, audioFrameSizes, numberOfMissingFrames, missingFrames);

#else

		if (100 > (int)in_data[1])
		{
			iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data, unLength);
		}
		else
		{
			iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, in_data, unLength);
		}

#endif

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

int CInterfaceOfAudioVideoEngine::SetHeightWidth(const IPVLongType llFriendID, int nVideoWidth, int nVideoHeight)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SetHeightWidth(llFriendID, nVideoWidth, nVideoHeight);

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


void CInterfaceOfAudioVideoEngine::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithPacketCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int, int, int, int))
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

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
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

void CInterfaceOfAudioVideoEngine::SetSendFunctionPointer(void(*callBackFunctionPointer)(unsigned char*, int, int))
{
    if (NULL != m_pcController)
    {
        m_pcController->SetSendFunctionPointer(callBackFunctionPointer);
    }
}

