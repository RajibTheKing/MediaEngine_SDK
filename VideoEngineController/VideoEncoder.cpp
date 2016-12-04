
#include "VideoEncoder.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "ThreadTools.h"

#include <string>

CVideoEncoder::CVideoEncoder(CCommonElementsBucket* pSharedObject):

m_pCommonElementsBucket(pSharedObject),
m_nMaxBitRate(BITRATE_MAX),
m_nBitRate(BITRATE_MAX - 25000),
m_nNetworkType(NETWORK_TYPE_NOT_2G),
m_pSVCVideoEncoder(NULL)

{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CVideoEncoder");

	m_pVideoEncoderMutex.reset(new CLockHandler);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoEncoder::CVideoEncoder Video Encoder Created");
}

CVideoEncoder::~CVideoEncoder()
{
	if(NULL != m_pSVCVideoEncoder)
    {
        m_pSVCVideoEncoder->Uninitialize();
        
        m_pSVCVideoEncoder = NULL;
    }

	SHARED_PTR_DELETE(m_pVideoEncoderMutex);
}

int CVideoEncoder::SetHeightWidth(int nVideoHeight, int nVideoWidth, int nFPS, int nIFrameInterval, bool bCheckDeviceCapability)
{
	Locker lock(*m_pVideoEncoderMutex);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder");

	m_nVideoWidth = nVideoWidth;
	m_nVideoHeight = nVideoHeight;

	SEncParamExt encoderParemeters;

	memset(&encoderParemeters, 0, sizeof(SEncParamExt));

	m_pSVCVideoEncoder->GetDefaultParams(&encoderParemeters);

	encoderParemeters.iUsageType = CAMERA_VIDEO_REAL_TIME;
	encoderParemeters.iTemporalLayerNum = 0;
	encoderParemeters.uiIntraPeriod = nIFrameInterval;
	encoderParemeters.eSpsPpsIdStrategy = INCREASING_ID;
	encoderParemeters.bEnableSSEI = false;
	encoderParemeters.bEnableFrameCroppingFlag = true;
	encoderParemeters.iLoopFilterDisableIdc = 0;
	encoderParemeters.iLoopFilterAlphaC0Offset = 0;
	encoderParemeters.iLoopFilterBetaOffset = 0;
	encoderParemeters.iMultipleThreadIdc = 0;
    encoderParemeters.iEntropyCodingModeFlag = true;
    
    
    
    
    if(!bCheckDeviceCapability)
    {
        CVideoCallSession* pVideoSession;
        bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(200, pVideoSession);
        
        if(bExist && (pVideoSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM))
		{   encoderParemeters.iRCMode = RC_BITRATE_MODE;
			encoderParemeters.iMinQp = 0;
			encoderParemeters.iMaxQp = 52;
		}
		else
        {
            encoderParemeters.iRCMode = RC_BITRATE_MODE;
            encoderParemeters.iMinQp = 0;
            encoderParemeters.iMaxQp = 52;
        }
        
    }
    else
    {
        encoderParemeters.iRCMode = RC_OFF_MODE;
    }
    
    
    
    
    
    
	encoderParemeters.bEnableDenoise = false;
	encoderParemeters.bEnableSceneChangeDetect = false;
	encoderParemeters.bEnableBackgroundDetection = true;
	encoderParemeters.bEnableAdaptiveQuant = false;
	encoderParemeters.bEnableFrameSkip = true;
	encoderParemeters.bEnableLongTermReference = true;
	encoderParemeters.iLtrMarkPeriod = 20;
	encoderParemeters.bPrefixNalAddingCtrl = false;
	encoderParemeters.iSpatialLayerNum = 1;


	SSpatialLayerConfig *spartialLayerConfiguration = &encoderParemeters.sSpatialLayers[0];

	spartialLayerConfiguration->uiProfileIdc = PRO_BASELINE;//;

	encoderParemeters.iPicWidth = spartialLayerConfiguration->iVideoWidth = m_nVideoWidth;
	encoderParemeters.iPicHeight = spartialLayerConfiguration->iVideoHeight = m_nVideoHeight;
	encoderParemeters.fMaxFrameRate = spartialLayerConfiguration->fFrameRate = (float)nFPS;
    if(!bCheckDeviceCapability)
    {
		CVideoCallSession* pVideoSession;
		bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(200, pVideoSession);

		if (bExist && (pVideoSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM))
		{
			LOGEF("fahad -->> VideoEncoder::SetHeightWidth -- SERVICE_TYPE_LIVE_STREAM -- %d", SERVICE_TYPE_LIVE_STREAM);
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_BEGIN_FOR_STREAM;
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_BEGIN_FOR_STREAM;
		}
		else
		{
			LOGEF("fahad -->> VideoEncoder::SetHeightWidth -- SERVICE_TYPE_Call -- %d", BITRATE_BEGIN);
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_BEGIN;
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_BEGIN;
		}
    }
    else
    {
		LOGEF("fahad -->> VideoEncoder::SetHeightWidth -- BITRATE_CHECK_CAPABILITY -- %d", BITRATE_CHECK_CAPABILITY);
        encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_CHECK_CAPABILITY;
        encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_CHECK_CAPABILITY;
    }


	spartialLayerConfiguration->iDLayerQp = 24;
	//spartialLayerConfiguration->sSliceCfg.uiSliceMode = SM_SINGLE_SLICE;
	spartialLayerConfiguration->sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder encoder initializing");

	long nReturnedValueFromEncoder = m_pSVCVideoEncoder->InitializeExt(&encoderParemeters);

	if (nReturnedValueFromEncoder != 0)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder unable to initialize OpenH264 encoder ");

		return 0;
	}

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoEncoder::CreateVideoEncoder Open h264 video encoder initialized");

	return 1;
}

int CVideoEncoder::CreateVideoEncoder(int nVideoHeight, int nVideoWidth, int nFPS, int nIFrameInterval, bool bCheckDeviceCapability)
{
	Locker lock(*m_pVideoEncoderMutex);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder");

	long nReturnedValueFromEncoder = WelsCreateSVCEncoder(&m_pSVCVideoEncoder);

	m_nVideoWidth = nVideoWidth;
	m_nVideoHeight = nVideoHeight;

	SEncParamExt encoderParemeters;

	memset(&encoderParemeters, 0, sizeof(SEncParamExt));

	m_pSVCVideoEncoder->GetDefaultParams(&encoderParemeters);

	encoderParemeters.iUsageType = CAMERA_VIDEO_REAL_TIME;
	encoderParemeters.iTemporalLayerNum = 0;
	encoderParemeters.uiIntraPeriod = nIFrameInterval;
	encoderParemeters.eSpsPpsIdStrategy = INCREASING_ID;
	encoderParemeters.bEnableSSEI = false;
	encoderParemeters.bEnableFrameCroppingFlag = true;
	encoderParemeters.iLoopFilterDisableIdc = 0;
	encoderParemeters.iLoopFilterAlphaC0Offset = 0;
	encoderParemeters.iLoopFilterBetaOffset = 0;
	encoderParemeters.iMultipleThreadIdc = 0;
    encoderParemeters.iEntropyCodingModeFlag = true;
    
	//encoderParemeters.iRCMode = RC_OFF_MODE;
    
    
    if(!bCheckDeviceCapability)
    {
        CVideoCallSession* pVideoSession;
        bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(200, pVideoSession);
        
        if(bExist && (pVideoSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM))
		{   encoderParemeters.iRCMode = RC_BITRATE_MODE;
			encoderParemeters.iMinQp = 0;
			encoderParemeters.iMaxQp = 52;
		}
        else
        {
            encoderParemeters.iRCMode = RC_BITRATE_MODE;
            encoderParemeters.iMinQp = 0;
            encoderParemeters.iMaxQp = 52;
        }
        
    }
    else
    {
        encoderParemeters.iRCMode = RC_OFF_MODE;
    }
    
    
    
	encoderParemeters.bEnableDenoise = false;
	encoderParemeters.bEnableSceneChangeDetect = false;
	encoderParemeters.bEnableBackgroundDetection = true;
	encoderParemeters.bEnableAdaptiveQuant = false;
	encoderParemeters.bEnableFrameSkip = true;
	encoderParemeters.bEnableLongTermReference = true;
	encoderParemeters.iLtrMarkPeriod = 20;
	encoderParemeters.bPrefixNalAddingCtrl = false;
	encoderParemeters.iSpatialLayerNum = 1;


	SSpatialLayerConfig *spartialLayerConfiguration = &encoderParemeters.sSpatialLayers[0];

	spartialLayerConfiguration->uiProfileIdc = PRO_BASELINE;//;

	encoderParemeters.iPicWidth = spartialLayerConfiguration->iVideoWidth = m_nVideoWidth;
	encoderParemeters.iPicHeight = spartialLayerConfiguration->iVideoHeight = m_nVideoHeight;
	encoderParemeters.fMaxFrameRate = spartialLayerConfiguration->fFrameRate = (float)nFPS;
    if(!bCheckDeviceCapability)
    {
		CVideoCallSession* pVideoSession;
		bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(200, pVideoSession);

		if (bExist && (pVideoSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM))
		{
			LOGEF("fahad -->> VideoEncoder::CreateVideoEncoder -- SERVICE_TYPE_LIVE_STREAM -- %d", SERVICE_TYPE_LIVE_STREAM);
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_BEGIN_FOR_STREAM;
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_BEGIN_FOR_STREAM;
		}
		else
		{
			LOGEF("fahad -->> VideoEncoder::CreateVideoEncoder -- SERVICE_TYPE_Call -- %d, bExist= %d, pVideoSession->GetServiceType() = %d", BITRATE_BEGIN, bExist, pVideoSession->GetServiceType());
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_BEGIN;
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_BEGIN;
		}     
    }
    else
    {
		LOGEF("fahad -->> VideoEncoder::CreateVideoEncoder -- BITRATE_CHECK_CAPABILITY -- %d", BITRATE_CHECK_CAPABILITY);
        encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_CHECK_CAPABILITY;
        encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_CHECK_CAPABILITY;
    }
    
	spartialLayerConfiguration->iDLayerQp = 24;
	//spartialLayerConfiguration->sSliceCfg.uiSliceMode = SM_SINGLE_SLICE;
	spartialLayerConfiguration->sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder encoder initializing");

	nReturnedValueFromEncoder = m_pSVCVideoEncoder->InitializeExt(&encoderParemeters);

	if (nReturnedValueFromEncoder != 0)
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder unable to initialize OpenH264 encoder ");

		return 0;
	}

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoEncoder::CreateVideoEncoder Open h264 video encoder initialized");

	return 1;
}

int CVideoEncoder::SetBitrate(int nBitRate)
{
	CVideoCallSession* pVideoSession;
	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(200, pVideoSession);

	if (bExist && (pVideoSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM))
	{
		return 0;
	}

	int nTargetBitRate = nBitRate - (nBitRate % 25000);
    
	if (m_nNetworkType == NETWORK_TYPE_NOT_2G && nTargetBitRate<BITRATE_MIN) 
		nTargetBitRate = BITRATE_MIN;
    
	if (m_nNetworkType == NETWORK_TYPE_2G && nTargetBitRate<BITRATE_MIN_FOR_2G) 
		nTargetBitRate = BITRATE_MIN_FOR_2G;
    
    if(nTargetBitRate>BITRATE_MAX) 
		nTargetBitRate = BITRATE_MAX;
    
	SBitrateInfo targetEncoderBitrateInfo;

	targetEncoderBitrateInfo.iLayer = SPATIAL_LAYER_0;
	targetEncoderBitrateInfo.iBitrate = nTargetBitRate;

	LOGEF("fahad -->> VideoEncoder::SetBitrate -- nTargetBitRate = %d", nTargetBitRate);
	int nReturnedValueFromEncoder;

	if(m_pSVCVideoEncoder)
	{
		nReturnedValueFromEncoder = m_pSVCVideoEncoder->SetOption(ENCODER_OPTION_BITRATE, &targetEncoderBitrateInfo);

		if (nReturnedValueFromEncoder != 0)
		{
			CLogPrinter_WriteSpecific4(CLogPrinter::INFO, "BR~ CVideoEncoder::CreateVideoEncoder unable to set bitrate "+ Tools::IntegertoStringConvert(nTargetBitRate));
		}
		else
		{
			CLogPrinter_WriteSpecific4(CLogPrinter::INFO, "BR~ CVideoEncoder::CreateVideoEncoder bitrate set to " + Tools::IntegertoStringConvert(nTargetBitRate));
                
			m_nBitRate = nTargetBitRate;
		}
	}
	else
	{
		CLogPrinter_Write("OpenH264 encoder NULL!");
	}
        
    return nReturnedValueFromEncoder;
}

void CVideoEncoder::SetNetworkType(int nNetworkType)
{
	m_nNetworkType = nNetworkType;
}

int CVideoEncoder::SetMaxBitrate(int nBitRate)
{
	CVideoCallSession* pVideoSession;
	bool bExist = m_pCommonElementsBucket->m_pVideoCallSessionList->IsVideoSessionExist(200, pVideoSession);

	if (bExist && (pVideoSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM))
	{
		return 0;
	}

	nBitRate = nBitRate * MAX_BITRATE_MULTIPLICATION_FACTOR;

	int nTargetBitRate = nBitRate - (nBitRate % 25000);
    
    if(nTargetBitRate<BITRATE_MIN) 
		nTargetBitRate = BITRATE_MIN;

    if(nTargetBitRate>BITRATE_MAX + MAX_BITRATE_TOLERANCE) 
		nTargetBitRate = BITRATE_MAX + MAX_BITRATE_TOLERANCE;

	SBitrateInfo maxEncoderBitRateInfo, targetEncoderBitrateInfo;

	maxEncoderBitRateInfo.iLayer = SPATIAL_LAYER_0;
	maxEncoderBitRateInfo.iBitrate = nTargetBitRate;

	LOGEF("fahad -->> VideoEncoder::SetMaxBitrate -- nTargetBitRate = %d", nTargetBitRate);
	int nReturnedValueFromEncoder;

	if(m_pSVCVideoEncoder)
	{
		nReturnedValueFromEncoder = m_pSVCVideoEncoder->SetOption(ENCODER_OPTION_MAX_BITRATE, &maxEncoderBitRateInfo);

		if (nReturnedValueFromEncoder != 0)
		{
			CLogPrinter_WriteSpecific4(CLogPrinter::INFO, "$$*(BR~ CVideoEncoder::CreateVideoEncoder unable to set max bitrate "+ Tools::IntegertoStringConvert(nTargetBitRate));
		}
		else
		{
			CLogPrinter_WriteSpecific4(CLogPrinter::INFO, "$$*(BR~ CVideoEncoder::CreateVideoEncoder max bitrate set to " + Tools::IntegertoStringConvert(nTargetBitRate));
                
			m_nMaxBitRate = nTargetBitRate;
		}

	}
	else
	{
		CLogPrinter_Write("OpenH264 encoder NULL!");
	}

    return nReturnedValueFromEncoder;
}

int CVideoEncoder::EncodeVideoFrame(unsigned char *ucaEncodingVideoFrameData, unsigned int unLenght, unsigned char *ucaEncodedVideoFrameData)
{
	Locker lock(*m_pVideoEncoderMutex);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::Encode");

	if (NULL == m_pSVCVideoEncoder)
	{
		CLogPrinter_Write("OpenH264 encoder NULL!");

		return 0;
	}

	SFrameBSInfo frameBSInfo;
	SSourcePicture sourcePicture;

	sourcePicture.iColorFormat = videoFormatI420;
	sourcePicture.uiTimeStamp = 0;
	sourcePicture.iPicWidth = m_nVideoWidth;
	sourcePicture.iPicHeight = m_nVideoHeight;

	sourcePicture.iStride[0] = m_nVideoWidth;
	sourcePicture.iStride[1] = sourcePicture.iStride[2] = sourcePicture.iStride[0] >> 1;

	sourcePicture.pData[0] = (unsigned char *)ucaEncodingVideoFrameData;
	sourcePicture.pData[1] = sourcePicture.pData[0] + (m_nVideoWidth * m_nVideoHeight);
	sourcePicture.pData[2] = sourcePicture.pData[1] + (m_nVideoWidth * m_nVideoHeight >> 2);

	int nReturnedValueFromEncoder = m_pSVCVideoEncoder->EncodeFrame(&sourcePicture, &frameBSInfo);

	if (nReturnedValueFromEncoder != 0)
	{
        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoEncoder::EncodeAndTransfer Encode FAILED");

		return 0;
	}

	if (videoFrameTypeSkip == frameBSInfo.eFrameType || videoFrameTypeInvalid == frameBSInfo.eFrameType)
	{
		return 0;
	}

	int nEncodedVideoFrameSize = 0;

	for (int iLayer = 0, iCopyIndex = 0; iLayer < frameBSInfo.iLayerNum; iLayer++)
	{
		SLayerBSInfo* pLayerBsInfo = &frameBSInfo.sLayerInfo[iLayer];
		
		if (pLayerBsInfo)
		{
			int nLayerSize = 0;

			for (int iNalIndex = pLayerBsInfo->iNalCount - 1; iNalIndex >= 0; iNalIndex--)
			{
				nLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIndex];
			}

			memcpy(ucaEncodedVideoFrameData + iCopyIndex, pLayerBsInfo->pBsBuf, nLayerSize);

			iCopyIndex += nLayerSize;
			nEncodedVideoFrameSize += nLayerSize;
		}
	}

	return nEncodedVideoFrameSize;
}

int CVideoEncoder::GetBitrate()
{
	return m_nBitRate;
}
int CVideoEncoder::GetMaxBitrate()
{
	return m_nMaxBitRate;
}























