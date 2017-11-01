
#include "VideoEncoder.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "ThreadTools.h"

#include <string>
#include <cmath>

namespace MediaSDK
{

	CVideoEncoder::CVideoEncoder(CCommonElementsBucket* pSharedObject, long long llfriendID) :

		m_pcCommonElementsBucket(pSharedObject),
		m_nMaxBitRate(BITRATE_MAX),
		m_nBitRate(BITRATE_MAX - 25000),
		m_nNetworkType(NETWORK_TYPE_NOT_2G),
		m_pcSVCVideoEncoder(NULL),
		m_llfriendID(llfriendID)

	{
		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CVideoEncoder");

		m_pVideoEncoderMutex.reset(new CLockHandler);

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoEncoder::CVideoEncoder Video Encoder Created");
	}

	CVideoEncoder::~CVideoEncoder()
	{
		if (NULL != m_pcSVCVideoEncoder)
		{
			m_pcSVCVideoEncoder->Uninitialize();

			m_pcSVCVideoEncoder = NULL;
		}

		SHARED_PTR_DELETE(m_pVideoEncoderMutex);
	}

	int CVideoEncoder::SetHeightWidth(int nVideoHeight, int nVideoWidth, int nFPS, int nIFrameInterval, bool bCheckDeviceCapability, int nServiceType, int nDataType)
	{
		EncoderLocker lock(*m_pVideoEncoderMutex);

		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder");

		if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
		{
#if defined(DESKTOP_C_SHARP)

			if (nDataType == CAMARA_VIDEO_DATA && nVideoWidth < nVideoHeight)
			{
				int nNewHeight;
				int nNewWidth;

				CalculateAspectRatioWithScreenAndModifyHeightWidth(nVideoHeight, nVideoWidth, nNewHeight, nNewWidth);

				m_nVideoHeight = nNewHeight;
				m_nVideoWidth = nNewWidth;	
			}
			else
			{
				m_nVideoHeight = nVideoHeight;
				m_nVideoWidth = nVideoWidth;
			}
			
#else
			int nNewHeight;
			int nNewWidth;

			CalculateAspectRatioWithScreenAndModifyHeightWidth(nVideoHeight, nVideoWidth, nNewHeight, nNewWidth);

			m_nVideoHeight = nNewHeight;
			m_nVideoWidth = nNewWidth;
#endif

		}
		else
		{
			m_nVideoHeight = nVideoHeight;
			m_nVideoWidth = nVideoWidth;
		}

		SEncParamExt encoderParemeters;

		memset(&encoderParemeters, 0, sizeof(SEncParamExt));

		m_pcSVCVideoEncoder->GetDefaultParams(&encoderParemeters);

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


		if (!bCheckDeviceCapability)
		{
			if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
			{
				encoderParemeters.iRCMode = RC_BITRATE_MODE;
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
		if (!bCheckDeviceCapability)
		{
			if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
			{
				//LOGEF("fahad -->> VideoEncoder::SetHeightWidth -- SERVICE_TYPE_LIVE_STREAM -- %d", SERVICE_TYPE_LIVE_STREAM);
				encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_BEGIN_FOR_STREAM;
				encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_BEGIN_FOR_STREAM;
				m_nBitRate = BITRATE_BEGIN_FOR_STREAM;
			}
			else
			{
				//LOGEF("fahad -->> VideoEncoder::SetHeightWidth -- SERVICE_TYPE_Call -- %d", BITRATE_BEGIN);
				encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_BEGIN;
				encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_BEGIN;
				m_nBitRate = BITRATE_BEGIN;
			}
		}
		else
		{
			//LOGEF("fahad -->> VideoEncoder::SetHeightWidth -- BITRATE_CHECK_CAPABILITY -- %d", BITRATE_CHECK_CAPABILITY);
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_CHECK_CAPABILITY;
			encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_CHECK_CAPABILITY;
		}


		spartialLayerConfiguration->iDLayerQp = 24;
		//spartialLayerConfiguration->sSliceCfg.uiSliceMode = SM_SINGLE_SLICE;
		spartialLayerConfiguration->sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;

		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder encoder initializing");

		long nReturnedValueFromEncoder = m_pcSVCVideoEncoder->InitializeExt(&encoderParemeters);

		if (nReturnedValueFromEncoder != 0)
		{
			CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder unable to initialize OpenH264 encoder ");

			return 0;
		}

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoEncoder::CreateVideoEncoder Open h264 video encoder initialized");

		return 1;
	}

	int CVideoEncoder::CreateVideoEncoder(int nVideoHeight, int nVideoWidth, int nFPS, int nIFrameInterval, bool bCheckDeviceCapability, int nServiceType)
	{
		EncoderLocker lock(*m_pVideoEncoderMutex);

		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder");

		long nReturnedValueFromEncoder = WelsCreateSVCEncoder(&m_pcSVCVideoEncoder);

		if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
		{
#if defined(DESKTOP_C_SHARP)
			m_nVideoWidth = nVideoWidth;
			m_nVideoHeight = nVideoHeight;
#else
			int nNewHeight;
			int nNewWidth;

			CalculateAspectRatioWithScreenAndModifyHeightWidth(nVideoHeight, nVideoWidth, nNewHeight, nNewWidth);

			m_nVideoHeight = nNewHeight;
			m_nVideoWidth = nNewWidth;
#endif
		}
		else
		{
			m_nVideoWidth = nVideoWidth;
			m_nVideoHeight = nVideoHeight;
		}

		SEncParamExt encoderParemeters;

		memset(&encoderParemeters, 0, sizeof(SEncParamExt));

		m_pcSVCVideoEncoder->GetDefaultParams(&encoderParemeters);

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


		if (!bCheckDeviceCapability)
		{
			if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
			{
				encoderParemeters.iRCMode = RC_BITRATE_MODE;
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

		if (!bCheckDeviceCapability)
		{
			if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
			{
				//LOGEF("fahad -->> VideoEncoder::CreateVideoEncoder -- SERVICE_TYPE_LIVE_STREAM -- %d", SERVICE_TYPE_LIVE_STREAM);
				encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_BEGIN_FOR_STREAM;
				encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_BEGIN_FOR_STREAM;
				m_nBitRate = BITRATE_BEGIN_FOR_STREAM;
			}
			else
			{
				//LOGEF("fahad -->> VideoEncoder::CreateVideoEncoder -- SERVICE_TYPE_Call -- %d, bExist= %d, pVideoSession->GetServiceType() = %d", BITRATE_BEGIN, bExist, pVideoSession->GetServiceType());
				encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iSpatialBitrate = BITRATE_BEGIN;
				encoderParemeters.iTargetBitrate = spartialLayerConfiguration->iMaxSpatialBitrate = BITRATE_BEGIN;
				m_nBitRate = BITRATE_BEGIN;
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

		nReturnedValueFromEncoder = m_pcSVCVideoEncoder->InitializeExt(&encoderParemeters);

		if (nReturnedValueFromEncoder != 0)
		{
			CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder unable to initialize OpenH264 encoder ");

			return 0;
		}

		CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoEncoder::CreateVideoEncoder Open h264 video encoder initialized");

		return 1;
	}

	int CVideoEncoder::SetBitrate(int nBitRate, int nServiceType)
	{
		EncoderLocker lock(*m_pVideoEncoderMutex);

		int nTargetBitRate = nBitRate - (nBitRate % 25000);

		/*	if (m_nNetworkType == NETWORK_TYPE_NOT_2G && nTargetBitRate<BITRATE_MIN)
				nTargetBitRate = BITRATE_MIN;

				if (m_nNetworkType == NETWORK_TYPE_2G && nTargetBitRate<BITRATE_MIN_FOR_2G)
				nTargetBitRate = BITRATE_MIN_FOR_2G;*/

		if (nTargetBitRate < BITRATE_MIN)
			nTargetBitRate = BITRATE_MIN;

		if (nTargetBitRate > BITRATE_MAX)
			nTargetBitRate = BITRATE_MAX;

		if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
		{
			if (nTargetBitRate == 575000)
				nTargetBitRate = 580000;
		}

		SBitrateInfo targetEncoderBitrateInfo;

		targetEncoderBitrateInfo.iLayer = SPATIAL_LAYER_0;
		targetEncoderBitrateInfo.iBitrate = nTargetBitRate;

		LOGEF("fahad -->> VideoEncoder::SetBitrate -- nTargetBitRate = %d", nTargetBitRate);
		int nReturnedValueFromEncoder;

		if (m_pcSVCVideoEncoder)
		{
			nReturnedValueFromEncoder = m_pcSVCVideoEncoder->SetOption(ENCODER_OPTION_BITRATE, &targetEncoderBitrateInfo);

			if (nReturnedValueFromEncoder != 0)
			{
				CLogPrinter_WriteSpecific4(CLogPrinter::INFO, "BR~ CVideoEncoder::CreateVideoEncoder unable to set bitrate " + Tools::IntegertoStringConvert(nTargetBitRate));
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

	int CVideoEncoder::SetMaxBitrate(int nBitRate, int nServiceType)
	{
		EncoderLocker lock(*m_pVideoEncoderMutex);

		nBitRate = (int)(nBitRate * MAX_BITRATE_MULTIPLICATION_FACTOR);

		int nTargetBitRate = nBitRate - (nBitRate % 25000);

		if (nTargetBitRate<BITRATE_MIN)
			nTargetBitRate = BITRATE_MIN;

		if (nTargetBitRate>BITRATE_MAX + MAX_BITRATE_TOLERANCE)
			nTargetBitRate = BITRATE_MAX + MAX_BITRATE_TOLERANCE;

		if (nServiceType == SERVICE_TYPE_LIVE_STREAM || nServiceType == SERVICE_TYPE_SELF_STREAM || nServiceType == SERVICE_TYPE_CHANNEL)
		{
			if (nTargetBitRate == 725000)
				nTargetBitRate = 700000;
		}

		SBitrateInfo maxEncoderBitRateInfo;

		maxEncoderBitRateInfo.iLayer = SPATIAL_LAYER_0;
		maxEncoderBitRateInfo.iBitrate = nTargetBitRate;

		LOGEF("fahad -->> VideoEncoder::SetMaxBitrate -- nTargetBitRate = %d", nTargetBitRate);
		int nReturnedValueFromEncoder;

		if (m_pcSVCVideoEncoder)
		{
			nReturnedValueFromEncoder = m_pcSVCVideoEncoder->SetOption(ENCODER_OPTION_MAX_BITRATE, &maxEncoderBitRateInfo);

			if (nReturnedValueFromEncoder != 0)
			{
				CLogPrinter_WriteSpecific4(CLogPrinter::INFO, "$$*(BR~ CVideoEncoder::CreateVideoEncoder unable to set max bitrate " + Tools::IntegertoStringConvert(nTargetBitRate));
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

	int CVideoEncoder::EncodeVideoFrame(unsigned char *ucaEncodingVideoFrameData, unsigned int unLenght, unsigned char *ucaEncodedVideoFrameData, bool isForceIFrame)
	{
		EncoderLocker lock(*m_pVideoEncoderMutex);

		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::Encode");

		if (NULL == m_pcSVCVideoEncoder)
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

		if (isForceIFrame)
		{
			printf("TheKing--> Forcing IDR Frame\n");
			m_pcSVCVideoEncoder->ForceIntraFrame(true);
		}

		int nReturnedValueFromEncoder = m_pcSVCVideoEncoder->EncodeFrame(&sourcePicture, &frameBSInfo);

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

		LOGE("fahad -->> EncodeVideoFrame : iHeight = %d, iWidth = %d", m_nVideoHeight, m_nVideoWidth);

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

	void CVideoEncoder::CalculateAspectRatioWithScreenAndModifyHeightWidth(int inHeight, int inWidth, int &newHeight, int &newWidth)
	{
		double aspectRatio_Screen, aspectRatio_VideoData;

		int iScreenHeight = 1920;
		int	iScreenWidth = 1130;

		aspectRatio_Screen = iScreenHeight * 1.0 / iScreenWidth;
		aspectRatio_VideoData = inHeight * 1.0 / inWidth;

		if (fabs(aspectRatio_Screen - aspectRatio_VideoData) < 0.1)
		{
			//Do Nothing
			newHeight = inHeight;
			newWidth = inWidth;

		}
		else if (aspectRatio_Screen > aspectRatio_VideoData)
		{
			//We have to delete columns [reduce Width]
			newWidth = floor(inHeight / aspectRatio_Screen);

			//
			//int target = floor(inWidth * 0.82);
			//
			//if(newWidth < target)
			//{
			//    newWidth = target;
			//}
			//

			newWidth = newWidth - newWidth % 4;
			newHeight = inHeight;
		}
		else
		{
			//We have to delete rows [Reduce Height]
			newHeight = floor(inWidth * aspectRatio_Screen);
			newHeight = newHeight - newHeight % 4;
			newWidth = inWidth;
		}
	}

} //namespace MediaSDK

