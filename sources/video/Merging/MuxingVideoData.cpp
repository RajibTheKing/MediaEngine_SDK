

#include "MuxingVideoData.h"
#include "../VideoEngineController/LogPrinter.h"
#include "../VideoEngineController/ThreadTools.h"


#if defined(__ANDROID__) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_WINDOWS_PHONE)
typedef unsigned char byte;
#endif

namespace MediaSDK
{

	CMuxingVideoData::CMuxingVideoData()
	{
		m_bBMP32FrameIsSet = false;
		m_pMuxingVideoMutex.reset(new CLockHandler);
	}

	CMuxingVideoData::~CMuxingVideoData()
	{

		SHARED_PTR_DELETE(m_pMuxingVideoMutex);
	}

	void CMuxingVideoData::SetBMP32Frame(unsigned char *pBMP32Data, int iLen, int iHeight, int iWidth)
	{

		GenerateCheckMatrix(pBMP32Data, iHeight, iWidth);

		GenerateUVIndexMatrix(iHeight, iWidth);

		m_bBMP32FrameIsSet = true;

	}

	void CMuxingVideoData::GenerateUVIndexMatrix(int iHeight, int iWidth) //Generating U-V index maping Based on YUV_I420
	{
		int yLength = iHeight * iWidth;
		int uLength = yLength / 2;

		int yConIndex = 0, vIndex = 0, uIndex = 0;


		uIndex = yLength;
		vIndex = yLength + yLength / 4;



		int heightIndex = 1;

		for (int i = 0;;)
		{
			if (i == iWidth*heightIndex)
			{
				i += iWidth;
				heightIndex += 2;
			}
			if (i >= yLength) break;
			yConIndex = i;

			m_IndexFor_U[yConIndex] = uIndex;
			m_IndexFor_U[yConIndex + 1] = uIndex;
			m_IndexFor_U[yConIndex + iWidth] = uIndex;
			m_IndexFor_U[yConIndex + iWidth + 1] = uIndex;

			m_IndexFor_V[yConIndex] = vIndex;
			m_IndexFor_V[yConIndex + 1] = vIndex;
			m_IndexFor_V[yConIndex + iWidth] = vIndex;
			m_IndexFor_V[yConIndex + iWidth + 1] = vIndex;

			uIndex++;
			vIndex++;
			i += 2;
		}

	}
	void CMuxingVideoData::GenerateCheckMatrix(unsigned char *pBMP32Data, int iHeight, int iWidth)
	{

		int indx = iHeight*iWidth;
		int temp = indx - iWidth;

		int bmp32Len = iHeight*iWidth * 4; //RGBA

		for (int i = 4; i < bmp32Len; i += 4)
		{
			if (pBMP32Data[i - 3] == 0 && pBMP32Data[i - 2] == 0 && pBMP32Data[i - 1] == 0 && pBMP32Data[i] == 0)
			{
				m_bCheckMatrix[temp++] = false;
			}
			else
			{
				m_bCheckMatrix[temp++] = true;
			}

			if (i % (iWidth * 4) == 0)
			{
				indx -= iWidth;
				temp = indx - iWidth;
			}
		}

	}

	int CMuxingVideoData::MergeFrameYUV_With_VideoYUV(unsigned char* pFrameYuv, unsigned char *pVideoYuv, int iHeight, int iWidth, unsigned char *pMergedData)
	{
		if (m_bBMP32FrameIsSet == false) //Check Matrix And UV Index maping is not initialized;
		{
			return -1;
		}

		int iLen = iHeight * iWidth * 3 / 2;

		for (int i = 0; i < iHeight; i++)
		{
			for (int j = 0; j < iWidth; j++)
			{
				int now = i*iWidth + j;

				if (m_bCheckMatrix[now] == true)
				{
					int yAddedVal = pFrameYuv[now] + pVideoYuv[now];
					pVideoYuv[now] = (unsigned char)(yAddedVal / 2);

					int uIndex = m_IndexFor_U[now];
					int vIndex = m_IndexFor_V[now];

					int uAddedVal = pFrameYuv[uIndex] + pVideoYuv[uIndex];
					pVideoYuv[uIndex] = (unsigned char)(uAddedVal / 2);
					int vAddedVal = pFrameYuv[vIndex] + pVideoYuv[vIndex];
					pVideoYuv[vIndex] = (unsigned char)(vAddedVal / 2);

				}
			}
		}

		memcpy(pMergedData, pVideoYuv, iLen);
		return iLen;
	}

} //namespace MediaSDK
