#include "VideoEncoder.h"
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "codec_def.h"
#include "size.h"

extern int g_OppNotifiedByterate;

CVideoEncoder::CVideoEncoder(CCommonElementsBucket* sharedObject):
m_pCommonElementsBucket(sharedObject),
m_iMaxBitrate(BITRATE_MAX),
m_iBitrate(BITRATE_MAX - 25000),
m_pSVCVideoEncoder(NULL)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CVideoEncoder");

	m_pMediaSocketMutex.reset(new CLockHandler);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoEncoder::CVideoEncoder Video Encoder Created");
}

CVideoEncoder::~CVideoEncoder()
{
	if(NULL != m_pSVCVideoEncoder)
    {
        m_pSVCVideoEncoder->Uninitialize();
        
        m_pSVCVideoEncoder = NULL;
    }

	SHARED_PTR_DELETE(m_pMediaSocketMutex);
}

int CVideoEncoder::CreateVideoEncoder(int iHeight, int iWidth)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder");

	long iRet = WelsCreateSVCEncoder(&m_pSVCVideoEncoder);

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	SEncParamExt encParam;
	memset(&encParam, 0, sizeof(SEncParamExt));
	m_pSVCVideoEncoder->GetDefaultParams(&encParam);
	encParam.iUsageType = CAMERA_VIDEO_REAL_TIME;
	encParam.iTemporalLayerNum = 0;
	encParam.uiIntraPeriod = I_INTRA_PERIOD;
	encParam.eSpsPpsIdStrategy = INCREASING_ID;
	encParam.bEnableSSEI = false;
	encParam.bEnableFrameCroppingFlag = true;
	encParam.iLoopFilterDisableIdc = 0;
	encParam.iLoopFilterAlphaC0Offset = 0;
	encParam.iLoopFilterBetaOffset = 0;
	encParam.iMultipleThreadIdc = 0;
#ifdef BITRATE_ENABLED
	encParam.iRCMode = RC_BITRATE_MODE;//RC_OFF_MODE;
	encParam.iMinQp = 0;
	encParam.iMaxQp = 52;
#else
 	encParam.iRCMode = RC_OFF_MODE;
#endif


	encParam.bEnableDenoise = false;
	encParam.bEnableSceneChangeDetect = false;
	encParam.bEnableBackgroundDetection = true;
	encParam.bEnableAdaptiveQuant = false;
	encParam.bEnableFrameSkip = true;
	encParam.bEnableLongTermReference = true;
	encParam.iLtrMarkPeriod = 20;
	encParam.bPrefixNalAddingCtrl = false;
	encParam.iSpatialLayerNum = 1;



	SSpatialLayerConfig *pDLayer = &encParam.sSpatialLayers[0];
	pDLayer->uiProfileIdc = PRO_BASELINE;//;
	encParam.iPicWidth = pDLayer->iVideoWidth = m_iWidth;
	encParam.iPicHeight = pDLayer->iVideoHeight = m_iHeight;
	encParam.fMaxFrameRate = pDLayer->fFrameRate = (float)FRAME_RATE;
	encParam.iTargetBitrate = pDLayer->iSpatialBitrate = BITRATE_MAX - 25000;
    encParam.iTargetBitrate = pDLayer->iMaxSpatialBitrate = BITRATE_MAX;
    
	pDLayer->iDLayerQp = 24;
	pDLayer->sSliceCfg.uiSliceMode = SM_SINGLE_SLICE;

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder encoder initializing");
	iRet = m_pSVCVideoEncoder->InitializeExt(&encParam);
	if (iRet != 0){
		CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder unable to initialize OpenH264 encoder ");
		return 0;
	}

#ifdef BITRATE_ENABLED
//	SetBitrate(12);
//	SetMaxBitrate(12);
#endif

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoEncoder::CreateVideoEncoder Open h264 video encoder initialized");

	return 1;
}

int CVideoEncoder::SetBitrate(int iFps)
{
	int iBitRate = iFps - (iFps%25000);
    
    if(iBitRate<BITRATE_MIN) iBitRate = BITRATE_MIN;
    
    if(iBitRate>BITRATE_MAX) iBitRate = BITRATE_MAX;
    
	SBitrateInfo targetEncoderBitrateInfo;

		targetEncoderBitrateInfo.iLayer = SPATIAL_LAYER_0;
		targetEncoderBitrateInfo.iBitrate = (iBitRate);

		int iRet;
		if(m_pSVCVideoEncoder)
		{
			iRet = m_pSVCVideoEncoder->SetOption(ENCODER_OPTION_BITRATE, &targetEncoderBitrateInfo);
			if (iRet != 0)
			{
				CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "BR~ CVideoEncoder::CreateVideoEncoder unable to set bitrate "+ Tools::IntegertoStringConvert(iBitRate));
			}
			else
			{
				CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "BR~ CVideoEncoder::CreateVideoEncoder bitrate set to " + Tools::IntegertoStringConvert(iBitRate));
                
                m_iBitrate = iBitRate;
			}
		}
    
    //printf("VampireEngg--> SetBitrate(%d) = %d\n", iBitRate, iRet);
    
    
    return iRet;

}

int CVideoEncoder::SetMaxBitrate(int iFps)
{
    iFps = iFps * 1.25;
	int iBitRate = iFps - (iFps%25000);
    
    if(iBitRate<BITRATE_MIN) iBitRate = BITRATE_MIN;
    
    if(iBitRate>BITRATE_MAX+100000) iBitRate = BITRATE_MAX+100000;


	SBitrateInfo maxEncoderBitRateInfo, targetEncoderBitrateInfo;
	maxEncoderBitRateInfo.iLayer = SPATIAL_LAYER_0;
	maxEncoderBitRateInfo.iBitrate = (iBitRate);

		int iRet;
		if(m_pSVCVideoEncoder)
		{
			iRet = m_pSVCVideoEncoder->SetOption(ENCODER_OPTION_MAX_BITRATE, &maxEncoderBitRateInfo);
			if (iRet != 0){
				CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "$$*(BR~ CVideoEncoder::CreateVideoEncoder unable to set max bitrate "+ Tools::IntegertoStringConvert(iBitRate));
                
                
			}
			else
			{
				CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "$$*(BR~ CVideoEncoder::CreateVideoEncoder max bitrate set to " + Tools::IntegertoStringConvert(iBitRate));
                
                m_iMaxBitrate = iBitRate;
			}

		}
    
    //printf("VampireEngg--> SetmaxBitrate(%d) = %d\n", iBitRate, iRet);
    return iRet;
}

void CVideoEncoder::SetFramerate(int iFps)
{
	/*SRateThresholds FrameRateTh;
	FrameRateTh.iMinThresholdFrameRate = iFps;
	FrameRateTh.iHeight = m_iHeight;
	FrameRateTh.iWidth = m_iWidth;
	FrameRateTh.iSkipFrameRate = 1;
	FrameRateTh.iSkipFrameStep = 2;
	FrameRateTh.iThresholdOfMaxRate = 15;
	FrameRateTh.iThresholdOfMinRate = 8;

	int iRet;
	if(m_pSVCVideoEncoder)
	{
		iRet = m_pSVCVideoEncoder->SetOption(ENCODER_OPTION_FRAME_RATE, &FrameRateTh);
		if (iRet != 0){
			CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder unable to set framerate---<< "+ Tools::IntegertoStringConvert(iFps));

		}
		else
		{
			CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder framerate set to---<< " + Tools::IntegertoStringConvert(iFps));
		}
	}*/
}

int CVideoEncoder::EncodeAndTransfer(unsigned char *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoEncoder::Encode");

	if (NULL == m_pSVCVideoEncoder){
		CLogPrinter_Write("OpenH264 encoder NULL!");
		return 0;
	}


	SFrameBSInfo frameBSInfo;
	SSourcePicture sourcePicture;
	sourcePicture.iColorFormat = videoFormatI420;
	sourcePicture.uiTimeStamp = 0;
	sourcePicture.iPicWidth = m_iWidth;
	sourcePicture.iPicHeight = m_iHeight;
	sourcePicture.iStride[0] = m_iWidth;
	sourcePicture.iStride[1] = sourcePicture.iStride[2] = sourcePicture.iStride[0] >> 1;
	sourcePicture.pData[0] = (unsigned char *)in_data;
	sourcePicture.pData[1] = sourcePicture.pData[0] + (m_iWidth * m_iHeight);
	sourcePicture.pData[2] = sourcePicture.pData[1] + (m_iWidth * m_iHeight >> 2);

	int iRet = m_pSVCVideoEncoder->EncodeFrame(&sourcePicture, &frameBSInfo);

	if (iRet != 0){
        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoEncoder::EncodeAndTransfer Encode FAILED");
		return 0;
	}
	// fixed issue in case dismatch source picture introduced by frame skipped, 1/12/2010
	if (videoFrameTypeSkip == frameBSInfo.eFrameType || videoFrameTypeInvalid == frameBSInfo.eFrameType)
	{
		return 0;
	}
	int iFrameSize = 0;
	int copy_index = 0;
	for (int iLayer = 0; iLayer < frameBSInfo.iLayerNum; iLayer++){
		SLayerBSInfo* pLayerBsInfo = &frameBSInfo.sLayerInfo[iLayer];
		if (pLayerBsInfo){
			int iLayerSize = 0;
			for (int iNalIdx = pLayerBsInfo->iNalCount - 1; iNalIdx >= 0; iNalIdx--){
				iLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIdx];
			}
			memcpy(out_buffer + copy_index, pLayerBsInfo->pBsBuf, iLayerSize);
			copy_index += iLayerSize;
			iFrameSize += iLayerSize;
		}
	}

	return iFrameSize;
}

int CVideoEncoder::GetBitrate()
{
    return m_iBitrate;
}
int CVideoEncoder::GetMaxBitrate()
{
    return m_iMaxBitrate;
}


