#include "VideoEncoder.h"
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"
#include "Tools.h"

#define BITRATE_MAX 1000 * 5000


CVideoEncoder::CVideoEncoder(CCommonElementsBucket* sharedObject):
m_pCommonElementsBucket(sharedObject),
m_pSVCVideoEncoder(NULL)
{
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::CVideoEncoder");

	m_pMediaSocketMutex.reset(new CLockHandler);

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoEncoder::CVideoEncoder Video Encoder Created");
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
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder");

	long iRet = WelsCreateSVCEncoder(&m_pSVCVideoEncoder);

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	SEncParamExt encParam;
	memset(&encParam, 0, sizeof(SEncParamExt));
	m_pSVCVideoEncoder->GetDefaultParams(&encParam);
	encParam.iUsageType = CAMERA_VIDEO_REAL_TIME;
	encParam.iTemporalLayerNum = 0;
	encParam.uiIntraPeriod = 8;
	encParam.eSpsPpsIdStrategy = INCREASING_ID;
	encParam.bEnableSSEI = false;
	encParam.bEnableFrameCroppingFlag = true;
	encParam.iLoopFilterDisableIdc = 0;
	encParam.iLoopFilterAlphaC0Offset = 0;
	encParam.iLoopFilterBetaOffset = 0;
	encParam.iMultipleThreadIdc = 0;
	encParam.iRCMode = RC_OFF_MODE;
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
	pDLayer->uiProfileIdc = PRO_BASELINE;
	encParam.iPicWidth = pDLayer->iVideoWidth = m_iWidth;
	encParam.iPicHeight = pDLayer->iVideoHeight = m_iHeight;
	encParam.fMaxFrameRate = pDLayer->fFrameRate = (float)30;
	encParam.iTargetBitrate = pDLayer->iSpatialBitrate = BITRATE_MAX;
	pDLayer->iDLayerQp = 24;
	pDLayer->sSliceCfg.uiSliceMode = SM_SINGLE_SLICE;

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder encoder initializing");
	iRet = m_pSVCVideoEncoder->InitializeExt(&encParam);
	if (iRet != 0){
		CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::CreateVideoEncoder unable to initialize OpenH264 encoder ");
		return 0;
	}

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoEncoder::CreateVideoEncoder Open h264 video encoder initialized");

	return 1;
}

int CVideoEncoder::EncodeAndTransfer(unsigned char *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode");
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 0");

	if (NULL == m_pSVCVideoEncoder){
		cout << "OpenH264 encoder NULL!\n";
		return 0;
	}

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 1");

	SFrameBSInfo frameBSInfo;

	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 2");
	SSourcePicture sourcePicture;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 3");
	sourcePicture.iColorFormat = videoFormatI420;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 4");
	sourcePicture.uiTimeStamp = 0;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 5");
	sourcePicture.iPicWidth = m_iWidth;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 6");
	sourcePicture.iPicHeight = m_iHeight;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 7");
	sourcePicture.iStride[0] = m_iWidth;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 8");
	sourcePicture.iStride[1] = sourcePicture.iStride[2] = sourcePicture.iStride[0] >> 1;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 9");
	sourcePicture.pData[0] = (unsigned char *)in_data;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 10");
	sourcePicture.pData[1] = sourcePicture.pData[0] + (m_iWidth * m_iHeight);
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 11");
	sourcePicture.pData[2] = sourcePicture.pData[1] + (m_iWidth * m_iHeight >> 2);
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 12");

	int iRet = m_pSVCVideoEncoder->EncodeFrame(&sourcePicture, &frameBSInfo);
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 13");
	//cout << "OpenH264 encoding returned " << iRet + " " << ((int)frameBSInfo.eFrameType) << " " << (frameBSInfo.iLayerNum) + " " << (frameBSInfo.iFrameSizeInBytes);
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 14");
	if (iRet != 0){
        
        CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoEncoder::EncodeAndTransfer Encode FAILED");
    
		return 0;
	}
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 15");
	// fixed issue in case dismatch source picture introduced by frame skipped, 1/12/2010
	if (videoFrameTypeSkip == frameBSInfo.eFrameType || videoFrameTypeInvalid == frameBSInfo.eFrameType)
	{
		return 0;
	}
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 16");
	int iFrameSize = 0;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 17");
	int copy_index = 0;
	CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 18");
	for (int iLayer = 0; iLayer < frameBSInfo.iLayerNum; iLayer++){
		SLayerBSInfo* pLayerBsInfo = &frameBSInfo.sLayerInfo[iLayer];
		CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 19");
		if (pLayerBsInfo){
			CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 20");
			int iLayerSize = 0;
			for (int iNalIdx = pLayerBsInfo->iNalCount - 1; iNalIdx >= 0; iNalIdx--){
				CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 22");
				iLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIdx];
			}
			CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 23");
			memcpy(out_buffer + copy_index, pLayerBsInfo->pBsBuf, iLayerSize);
			CLogPrinter::Write(CLogPrinter::INFO, "CVideoEncoder::Encode 24");
			copy_index += iLayerSize;
			iFrameSize += iLayerSize;
		}
	}

    CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoEncoder::Encode done encoding --- > iFrameSize = "+Tools::IntegertoStringConvert(iFrameSize));

	return iFrameSize;
}
