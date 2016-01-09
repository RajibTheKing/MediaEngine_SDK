#include "Controller.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "Size.h"
void abc(){}

CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine()
{
	m_pController = new CController();
}

CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine(const char* sLoggerPath, int iLoggerPrintLevel)
{
	m_pController = new CController(sLoggerPath, iLoggerPrintLevel);
}

bool CInterfaceOfAudioVideoEngine::Init(const IPVLongType& lUserID, const char* sLoggerPath, int iLoggerPrintLevel)
{
    return true;
}

bool CInterfaceOfAudioVideoEngine::InitializeLibrary(const IPVLongType& lUserID)
{
    return true;
}

CInterfaceOfAudioVideoEngine::~CInterfaceOfAudioVideoEngine()
{
	if (NULL != m_pController)
	{
		delete m_pController;

		m_pController = NULL;
	}
}

bool CInterfaceOfAudioVideoEngine::SetUserName(const IPVLongType lUserName)
{
	if (m_pController == NULL)
	{
		return false;
	}

	m_pController->initializeEventHandler();

	bool Ret = m_pController->SetUserName(lUserName);

	return Ret;
}

bool CInterfaceOfAudioVideoEngine::StartAudioCall(const IPVLongType lFriendID)
{
	if (m_pController == NULL)
	{
		return false;
	}

	bool bReturnedValue = m_pController->StartAudioCall(lFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::StartVideoCall(const IPVLongType lFriendID, int iVideoHeight, int iVideoWidth)
{
	if (m_pController == NULL)
	{
		return false;
	}

	bool bReturnedValue = m_pController->StartVideoCall(lFriendID, iVideoHeight, iVideoWidth);

	return bReturnedValue;
}

int CInterfaceOfAudioVideoEngine::EncodeAndTransfer(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size)
{
	if (m_pController == NULL)
	{
		return false;
	}

	int iReturnedValue = m_pController->EncodeAndTransfer(lFriendID, in_data, in_size);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::PushPacketForDecoding(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size)
{
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushPacketForDecoding called");
    
    if((int)in_data[0]==AUDIO_PACKET_MEDIA_TYPE)
    {
        CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushPacketForDecoding AUDIO data in Video");
        
        return -1;
    }
    else if((int)in_data[0]==VIDEO_PACKET_MEDIA_TYPE)
    {
        CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushPacketForDecoding CORRECT video data");
    }
    else
    {
        CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushPacketForDecoding unknown data");
        
        return -1;
    }
    
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushPacketForDecoding 2called");
    
/*    for(int i=0;i<10;i++)
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushPacketForDecoding in_data[" + Tools::IntegertoStringConvert(i) + "] " + Tools::IntegertoStringConvert(in_data[i]));*/
    
	if (m_pController == NULL)
	{
		return false;
	}

	int iReturnedValue = m_pController->PushPacketForDecoding(lFriendID, in_data, in_size);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::PushAudioForDecoding(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size)
{
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushAudioForDecoding called");
    
    int iReturnedValue = 0;
    
    if (m_pController == NULL)
    {
        return 0;
    }
    else if(in_data == nullptr)
    {
        CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushAudioForDecoding null data from connectivity");
        
        return 0;
    }
    else if(in_data == NULL)
    {
        CLogPrinter::Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushAudioForDecoding null data from connectivity");
        
        return 0;
    }
    else
    {
        if((int)in_data[0]==VIDEO_PACKET_MEDIA_TYPE)
        {
            iReturnedValue = m_pController->PushPacketForDecoding(lFriendID, in_data, in_size);
        }
        else if((int)in_data[0]==AUDIO_PACKET_MEDIA_TYPE)
        {
            iReturnedValue = m_pController->PushAudioForDecoding(lFriendID, in_data, in_size);
        }
        else
            return 0;
    }

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SendAudioData(const IPVLongType lFriendID, short *in_data, unsigned int in_size)
{
	if (m_pController == NULL)
	{
		return false;
	}

	int iReturnedValue = m_pController->SendAudioData(lFriendID, in_data, in_size);

	return iReturnedValue;
}

//int CInterfaceOfAudioVideoEngine::SendVideoData(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size)
int CInterfaceOfAudioVideoEngine::SendVideoData(const IPVLongType lFriendID, unsigned char *in_data, unsigned int in_size, unsigned int orientation_type)
{
	if (m_pController == NULL)
	{
		return false;
	}

//	int iReturnedValue = m_pController->SendVideoData(lFriendID, in_data, in_size);
	int iReturnedValue = m_pController->SendVideoData(lFriendID, in_data, in_size, orientation_type);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetHeightWidth(const IPVLongType lFriendID, int width, int height)
{
	if (m_pController == NULL)
	{
		return false;
	}

	int iReturnedValue = m_pController->SetHeightWidth(lFriendID, width, height);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetBitRate(const IPVLongType lFriendID, int bitRate)
{
	if (m_pController == NULL)
	{
		return false;
	}

	int iReturnedValue = m_pController->SetBitRate(lFriendID, bitRate);

	return iReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::StopAudioCall(const IPVLongType lFriendID)
{
	if (m_pController == NULL)
	{
		return false;
	}

	bool bReturnedValue = m_pController->StopAudioCall(lFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::StopVideoCall(const IPVLongType lFriendID)
{
	if (m_pController == NULL)
	{
		return false;
	}

	bool bReturnedValue = m_pController->StopVideoCall(lFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetLoggingState(bool loggingState, int logLevel)
{
    if (m_pController == NULL)
    {
        return false;
    }
    
    bool bReturnedValue = m_pController->SetLoggingState(loggingState, logLevel);
    
    return bReturnedValue;
}

void CInterfaceOfAudioVideoEngine::UninitializeLibrary()
{
	if (NULL != m_pController)
	{
		m_pController->UninitializeLibrary();
	}
}

void CInterfaceOfAudioVideoEngine::SetLoggerPath(std::string sLoggerPath)
{
	if (NULL != m_pController)
	{
		m_pController->SetLoggerPath(sLoggerPath);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
    //std::cout << "Test set callback 3 A" << std::endl;
    
	if (NULL != m_pController)
	{
        //std::cout << "Test set callback 4 A" << std::endl;
        
		m_pController->SetNotifyClientWithPacketCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int, int, int))
{
	if (NULL != m_pController)
	{
		m_pController->SetNotifyClientWithVideoDataCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
	if (NULL != m_pController)
	{
		m_pController->SetNotifyClientWithAudioDataCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
{
    if (NULL != m_pController)
    {
        m_pController->SetNotifyClientWithAudioPacketDataCallback(callBackFunctionPointer);
    }
}

void CInterfaceOfAudioVideoEngine::SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int))
{
    if (NULL != m_pController)
    {
        m_pController->SetSendFunctionPointer(callBackFunctionPointer);
    }
}

