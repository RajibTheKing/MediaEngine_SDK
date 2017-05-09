
#ifndef IPV_VIDEO_ENCODER_H
#define IPV_VIDEO_ENCODER_H

//#define _CRT_SECURE_NO_WARNINGS

#include "SmartPointer.h"
#include "LockHandler.h"
#include "codec_api.h"
#include "Tools.h"

namespace MediaSDK
{

	class CCommonElementsBucket;

	class CVideoEncoder
	{
	public:

		CVideoEncoder(CCommonElementsBucket* pSharedObject, long long llfriendID);
		~CVideoEncoder();

		int CreateVideoEncoder(int nVideoHeight, int nVideoWidth, int nFPS, int nIFrameInterval, bool bCheckDeviceCapability, int nServiceType);
		int EncodeVideoFrame(unsigned char *ucaEncodingVideoFrameData, unsigned int unLenght, unsigned char *ucaEncodedVideoFrameData, bool bIsForceIFrame);

		int SetBitrate(int nBitRate);
		void SetNetworkType(int nNetworkType);
		int SetMaxBitrate(int nBitRate);
		int GetBitrate();
		int GetMaxBitrate();
		int SetHeightWidth(int nVideoHeight, int nVideoWidth, int nFPS, int nIFrameInterval, bool bCheckDeviceCapability, int nServiceType);

		void CalculateAspectRatioWithScreenAndModifyHeightWidth(int inHeight, int inWidth, int &newHeight, int &newWidth);

	private:

		int m_nVideoHeight;
		int m_nVideoWidth;
		int m_nMaxBitRate;
		int m_nBitRate;
		int m_nNetworkType;
		long long m_llfriendID;

		Tools m_Tools;

		ISVCEncoder* m_pcSVCVideoEncoder;
		CCommonElementsBucket* m_pcCommonElementsBucket;

	protected:

		SmartPointer<CLockHandler> m_pVideoEncoderMutex;
	};

} //namespace MediaSDK

#endif
