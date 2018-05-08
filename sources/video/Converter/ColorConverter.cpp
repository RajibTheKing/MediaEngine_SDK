
#include "ColorConverter.h"
#include "LogPrinter.h"
#include "CommonElementsBucket.h"
#include <math.h>

#if defined(__ANDROID__) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_WINDOWS_PHONE)
typedef unsigned char byte;
#endif

namespace MediaSDK
{

CColorConverter::CColorConverter(int iVideoHeight, int iVideoWidth, CCommonElementsBucket* commonElementsBucket, long long lfriendID) :

/*m_iVideoHeight(iVideoHeight),
m_iVideoWidth(iVideoWidth),
m_YPlaneLength(m_iVideoHeight*m_iVideoWidth),
m_VPlaneLength(m_YPlaneLength >> 2),
m_UVPlaneMidPoint(m_YPlaneLength + m_VPlaneLength),
m_UVPlaneEnd(m_UVPlaneMidPoint + m_VPlaneLength),*/
m_bMergingSmallFrameEnabled(false),
m_pCommonElementsBucket(commonElementsBucket),
m_lfriendID(lfriendID)

{
	CLogPrinter_Write(CLogPrinter::INFO, "CColorConverter::CColorConverter");

	m_PrevAddValue = 0;
	m_AverageValue = 0;
	m_ThresholdValue = 0;
    
    //Initially small frame height and width is Zero
    m_iSmallFrameHeight = 0;
    m_iSmallFrameWidth = 0;
    
    if(m_iSmallFrameHeight%2) m_iSmallFrameHeight--;
    if(m_iSmallFrameWidth%2) m_iSmallFrameWidth--;
    
    m_iDeviceHeight = -1;
    m_iDeviceWidth = -1;

#if defined(DESKTOP_C_SHARP)

	//m_VideoBeautificationer = new CVideoBeautificationer(iVideoHeight, iVideoWidth);

#else

	int nNewHeight;
	int nNewWidth;

	CalculateAspectRatioWithScreenAndModifyHeightWidth(iVideoHeight, iVideoWidth, DEVICE_SCREEN_HEIGHT, DEVICE_SCREEN_WIDTH, nNewHeight, nNewWidth);

	//m_VideoBeautificationer = new CVideoBeautificationer(nNewHeight, nNewWidth);

#endif

	for (int i = 0; i < 641; i++)
		for (int j = 0; j < 641; j++)
		{
			m_Multiplication[i][j] = i*j;
		}

	m_pColorConverterMutex.reset(new CLockHandler);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CColorConverter::CColorConverter Prepared");


	//LOGE("fahad -->> CColorConverter::ConvertRGB32ToRGB24  inside constructor");
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(ANDROID)

    m_pNeonAssemblyWrapper = new NeonAssemblyWrapper();

#endif

}

CColorConverter::~CColorConverter()
{
	/*if (NULL != m_VideoBeautificationer)
	{
		delete m_VideoBeautificationer;
		m_VideoBeautificationer = NULL;
	}*/
}

int CColorConverter::TestVideoEffect(int *param, int size)
{
	//m_VideoBeautificationer->TestVideoEffect(param, size);

	return 1;
}


void CColorConverter::SetHeightWidth(int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);
    
#if defined(DESKTOP_C_SHARP)
    
	//m_VideoBeautificationer->SetHeightWidth(iVideoHeight, iVideoWidth);
    
#else
    
	int nNewHeight;
	int nNewWidth;

	CalculateAspectRatioWithScreenAndModifyHeightWidth(iVideoHeight, iVideoWidth, DEVICE_SCREEN_HEIGHT, DEVICE_SCREEN_WIDTH, nNewHeight, nNewWidth);

	//m_VideoBeautificationer->SetHeightWidth(nNewHeight, nNewWidth);
    
#endif

	/*m_iVideoHeight = iVideoHeight;
	m_iVideoWidth = iVideoWidth;
	m_YPlaneLength = m_iVideoHeight*m_iVideoWidth;
	m_VPlaneLength = m_YPlaneLength >> 2;
	m_UVPlaneMidPoint = m_YPlaneLength + m_VPlaneLength;
	m_UVPlaneEnd = m_UVPlaneMidPoint + m_VPlaneLength;*/

}

void CColorConverter::SetDeviceHeightWidth(int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

    m_iDeviceHeight = DEVICE_SCREEN_HEIGHT; //iVideoHeight;
    m_iDeviceWidth = DEVICE_SCREEN_WIDTH; //iVideoWidth;

	//m_VideoBeautificationer->SetDeviceHeightWidth(iVideoHeight, iVideoWidth);
}

int CColorConverter::ConvertI420ToNV21(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);
#if defined(HAVE_NEON)
    m_pNeonAssemblyWrapper->convert_i420_to_nv21_assembly(convertingData, iVideoHeight, iVideoWidth);
    return iVideoHeight * iVideoWidth * 3 / 2;
#else
	int i, j, k;

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	memcpy(m_pUPlane, convertingData + YPlaneLength, VPlaneLength);

	for (i = YPlaneLength, j = 0, k = UVPlaneMidPoint; i < UVPlaneEnd; i += 2, j++, k++)
	{
		convertingData[i] = convertingData[k];
		convertingData[i + 1] = m_pUPlane[j];
	}

	return UVPlaneEnd;
#endif
}
int CColorConverter::ConvertYV12ToI420(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
    ColorConverterLocker lock(*m_pColorConverterMutex);
    
    int YPlaneLength = iVideoHeight*iVideoWidth;
    int VPlaneLength = YPlaneLength >> 2;
    int UPlaneLength = YPlaneLength >> 2;
    int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
    int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;
    
    memcpy(m_pTempPlane, convertingData + YPlaneLength, UPlaneLength);
    memcpy(convertingData + YPlaneLength, convertingData + YPlaneLength + UPlaneLength, VPlaneLength);
    memcpy(convertingData + YPlaneLength + UPlaneLength, m_pTempPlane, UPlaneLength);
    
    return UVPlaneEnd;
}

int CColorConverter::ConvertNV21ToI420(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	int i, j, k;

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	for (i = YPlaneLength, j = 0, k = i; i < UVPlaneEnd; i += 2, j++, k++)
	{
		m_pVPlane[j] = convertingData[i];
		convertingData[k] = convertingData[i + 1];
	}

	memcpy(convertingData + UVPlaneMidPoint, m_pVPlane, VPlaneLength);

	return UVPlaneEnd;
}

int CColorConverter::ConvertYUY2ToI420(unsigned char * input, unsigned char * output, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	int pixels = iVideoHeight * iVideoWidth;
	int macropixels = pixels >> 1;

	long mpx_per_row = iVideoWidth >> 1;

	for (int i = 0, ci = 0; i < macropixels; i++)
	{
		output[i << 1] = input[i << 2];
		output[(i << 1) + 1] = input[(i << 2) + 2];

		long row_number = i / mpx_per_row;
		if (row_number % 2 == 0)
		{
			output[pixels + ci] = input[(i << 2) + 1];
			output[pixels + (pixels >> 2) + ci] = input[(i << 2) + 3];
			ci++;
		}
	}

	return pixels* 3 / 2;
}

    
int CColorConverter::ConvertI420ToNV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
    
	ColorConverterLocker lock(*m_pColorConverterMutex);
    
#if defined(HAVE_NEON) || defined(HAVE_NEON_AARCH64)
    //Total TimeDiff = 114, frame = 1000
    m_pNeonAssemblyWrapper->convert_i420_to_nv12_assembly(convertingData, iVideoHeight, iVideoWidth);
    return iVideoHeight * iVideoWidth * 3 / 2;
#else
    //Total TimeDiff = 129, frame = 1000
	int i, j, k;

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	memcpy(m_pUPlane, convertingData + YPlaneLength, VPlaneLength);

	for (i = YPlaneLength, j = 0, k = UVPlaneMidPoint; i < UVPlaneEnd; i += 2, j++, k++)
	{
		convertingData[i] = m_pUPlane[j];
		convertingData[i + 1] = convertingData[k];
	}
	return UVPlaneEnd;
#endif
    
}

int CColorConverter::ConvertI420ToYV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	memcpy(m_pTempPlane, convertingData + YPlaneLength, UPlaneLength);
	memcpy(convertingData + YPlaneLength, convertingData + YPlaneLength + UPlaneLength, VPlaneLength);
	memcpy(convertingData + YPlaneLength + UPlaneLength, m_pTempPlane, UPlaneLength);

	return UVPlaneEnd;
}
    
int CColorConverter::ConvertNV12ToI420(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#if defined(HAVE_NEON) || defined(HAVE_NEON_AARCH64)
    //printf("TheKing--> Here Inside convert_nv12_to_i420_assembly\n");
    m_pNeonAssemblyWrapper->convert_nv12_to_i420_assembly(convertingData, iVideoHeight, iVideoWidth);
    return iVideoHeight * iVideoWidth * 3 / 2;
#else
    //printf("TheKing--> Here Inside ConvertNV12ToI420\n");
	int i, j, k;

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	for (i = YPlaneLength, j = 0, k = i; i < UVPlaneEnd; i += 2, j++, k++)
	{
		m_pVPlane[j] = convertingData[i + 1];
		convertingData[k] = convertingData[i];
	}

	memcpy(convertingData + UVPlaneMidPoint, m_pVPlane, VPlaneLength);
    return UVPlaneEnd;
#endif
#else
	int i, j, k;

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	for (i = YPlaneLength, j = 0, k = i; i < UVPlaneEnd; i += 2, j++, k++)
	{
		m_pVPlane[j] = convertingData[i + 1];
		convertingData[k] = convertingData[i];
	}

	memcpy(convertingData + UVPlaneMidPoint, m_pVPlane, VPlaneLength);
	return UVPlaneEnd;
#endif
}

/*
void CColorConverter::mirrorRotateAndConvertNV21ToI420(unsigned char *pData)
{
	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	int i = 0;
	memcpy(m_pFrame, pData, iWidth*iHeight * 3 / 2);

	//Y
	for (int x = 0; x < iWidth; x++)
	{
		for (int y = 0; y < iHeight; y++)
		{
			pData[i] = m_pFrame[y*iWidth + x];
			i++;
		}
	}

	int halfWidth = iWidth / 2;
	int halfHeight = iHeight / 2;
	int dimention = iHeight*iWidth;
	int vIndex = dimention + halfHeight*halfWidth;

	for (int x = 0; x < halfWidth; x++)
		for (int y = 0; y < halfHeight; y++)
		{
			int ind = y*halfWidth + x;
			pData[i++] = m_pFrame[dimention + ind * 2 + 1];           //U
			pData[vIndex++] = m_pFrame[dimention + ind * 2];    //V
		}
}
*/


void CColorConverter::mirrorRotateAndConvertNV21ToI420(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth)
{
	//LOGE("fahad -->> avgValue= %d, addValue= %d, thresholdVal= %d, m_iVideoHeight=%d, m_iVideoWidth = %d", m_AverageValue, m_PrevAddValue, m_ThresholdValue,m_iVideoHeight, m_iVideoWidth);

	ColorConverterLocker lock(*m_pColorConverterMutex);

	int iWidth = iVideoHeight;
	int iHeight = iVideoWidth;

	int i = 0;
	//int totalYValue = 0;

	for (int x = iWidth - 1; x >-1; --x)
	{
		for (int y = 0; y <iHeight; ++y)
		{

			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];

			//totalYValue += (pData[i] & 0xFF);
			//m_VideoBeautificationer->MakePixelBright(&pData[i]);
			i++;
		}
	}

	//int m_AverageValue = totalYValue / m_Multiplication[iHeight][iWidth];

	//m_VideoBeautificationer->SetBrighteningValue(m_AverageValue , 10/*int brightnessPrecision*/);

	int halfWidth = iWidth >> 1;
	int halfHeight = iHeight >> 1;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

	for (int x = halfWidth - 1; x>-1; --x)
	{
		for (int y = 0; y < halfHeight; ++y)
		{
			int ind = ( m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind];
			pData[i++] = m_pFrame[dimention + ind + 1];
		}
	}

}

void CColorConverter::NegativeRotateAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth)
{
    ColorConverterLocker lock(*m_pColorConverterMutex);
    
    int iWidth = iVideoHeight;
    int iHeight = iVideoWidth;
    
    int i = iWidth * iHeight - iHeight;
    
    for(int y=iWidth-1;y>=0; y--)
    {
        int temp = i;
        for(int x = 0; x<iHeight;x++)
        {
            pData[temp++] = m_pFrame[x*iWidth + y];
        }
        
        i-=iHeight;
    }
    
    
    int halfWidth = iWidth / 2;
    int halfHeight = iHeight / 2;
    int dimention = m_Multiplication[iHeight][iWidth];
    int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];
    
    i = dimention+ halfWidth*halfHeight - halfHeight;
    vIndex = dimention + halfHeight*halfWidth + halfWidth*halfHeight - halfHeight;
    
    for(int y=halfWidth-1;y>=0; y--)
    {
        int temp = i;
        int temp2 = vIndex;
        for(int x = 0; x<halfHeight;x++)
        {
            int ind = (x*halfWidth + y) << 1;
            
            pData[temp2++] = m_pFrame[dimention + ind + 1];
            pData[temp++] = m_pFrame[dimention + ind];
        }
        i-=halfHeight;
        vIndex-=halfHeight;
    }
    
}


void CColorConverter::mirrorRotateAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	int iWidth = iVideoHeight;
	int iHeight = iVideoWidth;

	int i = 0;

	for (int x = iWidth - 1; x >-1; --x)
	{
		for (int y = 0; y < iHeight; ++y)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];
			i++;
		}
	}

	int halfWidth = iWidth / 2;
	int halfHeight = iHeight / 2;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

	for (int x = halfWidth - 1; x>-1; --x)
		for (int y = 0; y < halfHeight; ++y)
		{
			int ind = (m_Multiplication[y][halfWidth] + x) << 1 ;
			pData[vIndex++] = m_pFrame[dimention + ind + 1];
			pData[i++] = m_pFrame[dimention + ind];
		}

}


int CColorConverter::mirrorI420_XDirection(unsigned char *inData, unsigned char *outData, int iHeight, int iWidth)
{
    int halfHeight = iHeight>>1;
    int iTotalHeight = iHeight + halfHeight;
    int indx = 0;
    for (int y = 0; y < iTotalHeight; ++y)
    {
        for (int x = iWidth - 1; x > -1; --x)
        {
            outData[indx++] = inData[y*iWidth + x];
            
        }
    }
    return indx;
}

void CColorConverter::mirrorAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth)
{
	int iWidth = iVideoWidth;
	int iHeight =  iVideoHeight;

	int i = 0;
	for (int y = 0; y < iHeight; ++y)
	{
		for (int x = iWidth - 1; x > -1; --x)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];
			i++;
		}
	}

	int halfWidth = iWidth >> 1;
	int halfHeight = iHeight >> 1;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + iWidth;
	int nYUV12Height = iHeight + halfHeight;

	for (int y = iHeight; y < nYUV12Height; ++y)
		for (int x = halfWidth - 1; x > -1; --x)
		{
			int ind = m_Multiplication[y][iWidth] + (x << 1);
			pData[vIndex++] = m_pFrame[ind + 1];
			pData[i++] = m_pFrame[ind];
		}

}

void CColorConverter::mirrorRotateAndConvertNV21ToI420ForBackCam90(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	int iWidth = iVideoHeight;
	int iHeight = iVideoWidth;

	int i = 0;

	//int totalYValue = 0;

	for (int x = 0; x < iWidth; x++)
	{
		for (int y = iHeight - 1; y > -1; --y)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];

			//totalYValue += (pData[i] & 0xFF);
			//m_VideoBeautificationer->MakePixelBright(&pData[i]);

			i++;
		}
	}

	//int m_AverageValue = totalYValue / m_Multiplication[iHeight][iWidth];

	//m_VideoBeautificationer->SetBrighteningValue(m_AverageValue , 10/*int brightnessPrecision*/);

	int halfWidth = iWidth / 2;
	int halfHeight = iHeight / 2;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

//	for (int x = halfWidth - 1; x>-1; --x)
	for (int x = 0; x < halfWidth; x++)
		for (int y = halfHeight - 1; y > -1; --y)
		{
			int ind = ( m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind];
			pData[i++] = m_pFrame[dimention + ind + 1];
		}

}

void CColorConverter::mirrorRotateAndConvertNV21ToI420ForBackCam270(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	int iWidth = iVideoHeight;
	int iHeight = iVideoWidth;

	int i = 0;

	for (int x = iWidth - 1; x >-1; --x)
	{
		for (int y = 0; y <iHeight; ++y)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];
			i++;
		}
	}

	int halfWidth = iWidth >> 1;
	int halfHeight = iHeight >> 1;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

	for (int x = halfWidth - 1; x>-1; --x)
	{
		for (int y = 0; y < halfHeight; ++y)
		{
			int ind = ( m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind];
			pData[i++] = m_pFrame[dimention + ind + 1];
		}
	}

}

void CColorConverter::mirrorRotateAndConvertNV12ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	int iWidth = iVideoHeight;
	int iHeight = iVideoWidth;

	int i = 0;

	//	for (int x = iWidth - 1; x >-1; --x)
	for (int x = 0; x < iWidth; x++)
	{
		for (int y = iHeight - 1; y > -1; --y)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];
			i++;
		}
	}

	int halfWidth = iWidth / 2;
	int halfHeight = iHeight / 2;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

	//	for (int x = halfWidth - 1; x>-1; --x)
	for (int x = 0; x < halfWidth; x++)
		for (int y = halfHeight - 1; y > -1; --y)
		{
			int ind = (m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind + 1];
			pData[i++] = m_pFrame[dimention + ind];
		}

}

int CColorConverter::ConverterYUV420ToRGB24(unsigned char * pYUVs, unsigned char * pRGBs, int height, int width)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	int yIndex = 0;
	int uIndex = width * height;
	int vIndex = (width * height * 5) / 4;


	for (int r = 0; r < height; r++)
	{
		byte* pRGB = pRGBs + r * width * 3;

		if (r % 2 != 0)
		{
			uIndex -= (width >> 1);
			vIndex -= (width >> 1);
		}
		for (int c = 0; c < width; c += 2)
		{
			int C1 = pYUVs[yIndex++] - 16;
			int C2 = pYUVs[yIndex++] - 16;


			int D = pYUVs[vIndex++] - 128;
			int E = pYUVs[uIndex++] - 128;

			int R1 = (298 * C1 + 409 * E + 128) >> 8;
			int G1 = (298 * C1 - 100 * D - 208 * E + 128) >> 8;
			int B1 = (298 * C1 + 516 * D + 128) >> 8;

			int R2 = (298 * C2 + 409 * E + 128) >> 8;
			int G2 = (298 * C2 - 100 * D - 208 * E + 128) >> 8;
			int B2 = (298 * C2 + 516 * D + 128) >> 8;


			pRGB[0] = (byte)(R1 < 0 ? 0 : R1 > 255 ? 255 : R1);
			pRGB[1] = (byte)(G1 < 0 ? 0 : G1 > 255 ? 255 : G1);
			pRGB[2] = (byte)(B1 < 0 ? 0 : B1 > 255 ? 255 : B1);

			pRGB[3] = (byte)(R2 < 0 ? 0 : R2 > 255 ? 255 : R2);
			pRGB[4] = (byte)(G2 < 0 ? 0 : G2 > 255 ? 255 : G2);
			pRGB[5] = (byte)(B2 < 0 ? 0 : B2 > 255 ? 255 : B2);


			pRGB += 6;

		}
	}
	return width * height * 3;
}

void CColorConverter::mirrorYUVI420(unsigned char *pFrame, unsigned char *pData, int iHeight, int iWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);
#if defined(HAVE_NEON) || defined(HAVE_NEON_AARCH64)
    m_pNeonAssemblyWrapper->Mirror_YUV420_Assembly(pFrame, pData, iHeight, iWidth);
#else
	int yLen = m_Multiplication[iHeight][iWidth];;
	int uvLen = yLen >> 2;
	int vStartIndex = yLen + uvLen;
	int vEndIndex = (yLen * 3) >> 1;

	for(int i=0; i<iHeight;i++)
	{
		int k = iWidth-1;
		for(int j=0; j <iWidth; j++)
		{
			pData[i*iWidth +k] = pFrame[i*iWidth+j];
			k--;
		}

	}


	int uIndex = vStartIndex-1;
	int smallHeight = iHeight >> 1;
	int smallWidth = iWidth >> 1;

	for(int i=0; i<smallHeight;i++)
	{
		int k = smallWidth -1;
		for(int j=0; j <smallWidth; j++)
		{
			pData[yLen + i*smallWidth +k] = pFrame[yLen + i*smallWidth+j];
			k--;
		}

	}

	for(int i=0; i<smallHeight;i++)
	{
		int k = smallWidth - 1;
		for(int j=0; j <smallWidth; j++)
		{
			pData[vStartIndex+i*smallWidth +k] = pFrame[vStartIndex+i*smallWidth+j];
			k--;
		}

	}
#endif
    
}


static unsigned char clip[896];

static void InitClip() 
{
	memset(clip, 0, 320);
	for (int i = 0; i<256; ++i) clip[i + 320] = i;
	memset(clip + 320 + 256, 255, 320);
}

static inline unsigned char Clip(int x)
{
	return clip[320 + ((x + 0x8000) >> 16)];
}

int CColorConverter::ConvertRGB24ToI420(unsigned char* lpIndata, unsigned char* lpOutdata, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	static bool bInit = false;

	if (!bInit)
	{
		bInit = true;
		InitClip();
	}

	const int cyb = int(0.114 * 219 / 255 * 65536 + 0.5);
	const int cyg = int(0.587 * 219 / 255 * 65536 + 0.5);
	const int cyr = int(0.299 * 219 / 255 * 65536 + 0.5);

	unsigned char* py = lpOutdata;
	unsigned char* pu = lpOutdata + iVideoWidth * iVideoHeight;
	unsigned char* pv = pu + iVideoWidth*iVideoHeight / 4;

	for (int row = 0; row < iVideoHeight; ++row)
	{
		unsigned char* rgb = lpIndata + iVideoWidth * 3 * (iVideoHeight - 1 - row);
		for (int col = 0; col < iVideoWidth; col += 2)
		{
			// y1 and y2 can't overflow
			int y1 = (cyb*rgb[0] + cyg*rgb[1] + cyr*rgb[2] + 0x108000) >> 16;
			*py++ = y1;
			int y2 = (cyb*rgb[3] + cyg*rgb[4] + cyr*rgb[5] + 0x108000) >> 16;
			*py++ = y2;

			if ((row & 1) == 0)
			{
				int scaled_y = (y1 + y2 - 32) * int(255.0 / 219.0 * 32768 + 0.5);
				int b_y = ((rgb[0] + rgb[3]) << 15) - scaled_y;
				unsigned char u = *pu++ = Clip((b_y >> 10) * int(1 / 2.018 * 1024 + 0.5) + 0x800000);  // u
				int r_y = ((rgb[2] + rgb[5]) << 15) - scaled_y;
				unsigned char v = *pv++ = Clip((r_y >> 10) * int(1 / 1.596 * 1024 + 0.5) + 0x800000);  // v
			}
			rgb += 6;
		}
	}

	return iVideoHeight * iVideoWidth * 3 / 2;
}


/*
int CColorConverter::ConvertRGB24ToI420(unsigned char *input, unsigned char *output)
{
	if (m_bClipInitialization == false)
	{
		m_bClipInitialization = true;
		memset(m_pClip, 0, 320);
		for (int i = 0; i < 256; ++i) m_pClip[i + 320] = i;
		memset(m_pClip + 320 + 256, 255, 320);

		cyb = int(0.114 * 219 / 255 * 65536 + 0.5);
		cyg = int(0.587 * 219 / 255 * 65536 + 0.5);
		cyr = int(0.299 * 219 / 255 * 65536 + 0.5);

	}

	

	int py = 0;
	int pu = m_iVideoWidth*m_iVideoHeight;
	int pv = pu + m_iVideoWidth*m_iVideoHeight / 4;

	for (int row = 0; row < m_iVideoHeight; ++row)
	{
		unsigned char *rgb = input + m_iVideoWidth * 3 * (m_iVideoHeight - 1 - row);
		
		

		for (int col = 0; col < m_iVideoWidth; col += 2)
		{
			// y1 and y2 can't overflow
			int y1 = (cyb*rgb[0] + cyg*rgb[1] + cyr*rgb[2] + 0x108000) >> 16;
			output[py++] = (unsigned char)y1;
			int y2 = (cyb*rgb[3] + cyg*rgb[4] + cyr*rgb[5] + 0x108000) >> 16;
			output[py++] = (unsigned char)y2;

			if ((row & 1) == 0)
			{
				int scaled_y = (y1 + y2 - 32) * int(255.0 / 219.0 * 32768 + 0.5);
					
				int b_y = ((rgb[0] + rgb[3]) << 15) - scaled_y;
				int x1 = (b_y >> 10) * int(1 / 2.018 * 1024 + 0.5) + 0x800000;
				unsigned char u = output[pu++] = m_pClip[320 + ((x1 + 0x8000) >> 16)];  // u


				int r_y = ((rgb[2] + rgb[5]) << 15) - scaled_y;
				int x2 = (r_y >> 10) * int(1 / 1.596 * 1024 + 0.5) + 0x800000;
				unsigned char v = output[pv++] = m_pClip[320 + ((x2 + 0x8000) >> 16)];  // v
			}
			rgb += 6;
		}
		
	}

	return m_iVideoHeight * m_iVideoWidth * 3 / 2;
}
*/

int CColorConverter::ConvertRGB32ToRGB24(unsigned char *input, int iHeight, int iWidth, unsigned char *output)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

    int in_len = iHeight * iWidth * 4;
    
    int indx = 0;
    for(int i=0;i<in_len; i+=4)
    {
        output[indx++] = input[i];
        output[indx++] = input[i+1];
        output[indx++] = input[i+2];
    }
    
    return indx;
}


int CColorConverter::DownScaleYUVNV12_YUVNV21_AverageNotApplied(byte* pData, int &iHeight, int &iWidth, byte* outputData)
{
    
    int YPlaneLength = iHeight*iWidth;
    int indx = 0;
    
    for(int i=0;i<iHeight;i+=4)
    {
        for(int j=0;j<iWidth;j+=2)
        {
            outputData[indx++] = pData[i*iWidth + j];
        }
        
        for(int j=0;j<iWidth;j+=2)
        {
            outputData[indx++] = pData[(i+1)*iWidth + j];
        }
    }
    
    byte*p = pData+YPlaneLength;
    
    for(int i=0;i<iHeight/2;i+=2)
    {
        for(int j=0;j<iWidth;j+=4)
        {
            outputData[indx++] = p[i*iWidth + j];
            outputData[indx++] = p[i*iWidth + j+1];
        }
    }
    
    
    
    iHeight = iHeight>>1;
    iWidth = iWidth>>1;
    
    
    return indx;
    
}

int CColorConverter::DownScaleYUVNV12_YUVNV21_AverageVersion1(byte* pData, int &iHeight, int &iWidth, byte* outputData)
{
    
    int YPlaneLength = iHeight*iWidth;
    int indx = 0;
    
    for(int i=0;i<iHeight;i+=4)
    {
        for(int j=0;j<iWidth;j+=2)
        {
            //outputData[indx++] = pData[i*iWidth + j];
            
            int w,x,y,z;
            w = pData[i*iWidth + j];
            x = pData[i*iWidth + j+2];
            y = pData[(i+2)*iWidth + j];
            z = pData[(i+2)*iWidth + j+2];
            int avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
            
        }
        
        for(int j=0;j<iWidth;j+=2)
        {
            int I = i+1;
            
            int w,x,y,z;
            w = pData[I*iWidth + j];
            x = pData[I*iWidth + j+2];
            y = pData[(I+2)*iWidth + j];
            z = pData[(I+2)*iWidth + j+2];
            int avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
        }
    }
    
    byte*p = pData+YPlaneLength;
    for(int i=0;i<iHeight/2;i+=2)
    {
        for(int j=0;j<iWidth;j+=4)
        {
            int w,x,y,z, J, avg;
            
            
            w = p[i*iWidth + j];
            x = p[i*iWidth + j+2];
            y = p[(i+1)*iWidth + j];
            z = p[(i+1)*iWidth + j+2];
            avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
            //outputData[indx++] = p[i*iWidth + j];
            
            J = j+1;
            w = p[i*iWidth + J];
            x = p[i*iWidth + J+2];
            y = p[(i+1)*iWidth + J];
            z = p[(i+1)*iWidth + J+2];
            avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
            //outputData[indx++] = p[i*iWidth + j+1];
        }
    }
    
    
    
    iHeight = iHeight>>1;
    iWidth = iWidth>>1;
    
    
    return indx;
    
}


int CColorConverter::DownScaleYUVNV12_YUVNV21_AverageVersion2(byte* pData, int &iHeight, int &iWidth, byte* outputData)
{
    
    int YPlaneLength = iHeight*iWidth;
    int indx = 0;
    
    for(int i=0;i<iHeight;i+=4)
    {
        for(int j=0;j<iWidth;j+=2)
        {
            //outputData[indx++] = pData[i*iWidth + j];
            int w,x,y,z;
            if(j%2==0)
            {
                w = pData[i*iWidth + j];
                x = pData[i*iWidth + j+1];
                y = pData[(i+1)*iWidth + j];
                z = pData[(i+1)*iWidth + j+1];
                int avg = (w+x+y+z)/4;
                outputData[indx++] = (byte)avg;
            }
            else
            {
                w = pData[i*iWidth + j+1];
                x = pData[i*iWidth + j+2];
                y = pData[(i+1)*iWidth + j+1];
                z = pData[(i+1)*iWidth + j+2];
                int avg = (w+x+y+z)/4;
                outputData[indx++] = (byte)avg;
            }
        }
        
        for(int j=0;j<iWidth;j+=2)
        {
            int I = i+1;
            
            int w,x,y,z;
            if(j%2==0)
            {
                w = pData[(I+1)*iWidth + j];
                x = pData[(I+1)*iWidth + j+1];
                y = pData[(I+2)*iWidth + j];
                z = pData[(I+2)*iWidth + j+1];
                int avg = (w+x+y+z)/4;
                outputData[indx++] = (byte)avg;
            }
            else
            {
                w = pData[(I+1)*iWidth + j+1];
                x = pData[(I+1)*iWidth + j+2];
                y = pData[(I+2)*iWidth + j+1];
                z = pData[(I+2)*iWidth + j+2];
                int avg = (w+x+y+z)/4;
                outputData[indx++] = (byte)avg;
            }
        }
    }
    
    byte*p = pData+YPlaneLength;
    for(int i=0;i<iHeight/2;i+=2)
    {
        for(int j=0;j<iWidth;j+=4)
        {
            int w,x,y,z, J, avg;
            
            
            w = p[i*iWidth + j];
            x = p[i*iWidth + j+2];
            y = p[(i+1)*iWidth + j];
            z = p[(i+1)*iWidth + j+2];
            avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
            
            J = j+1;
            w = p[i*iWidth + J];
            x = p[i*iWidth + J+2];
            y = p[(i+1)*iWidth + J];
            z = p[(i+1)*iWidth + J+2];
            avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
        }
    }
    
    
    
    iHeight = iHeight>>1;
    iWidth = iWidth>>1;
    
    
    return indx;
    
}

int CColorConverter::DownScaleYUV420_EvenVersion(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData)
{
	//cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
	int YPlaneLength = iHeight*iWidth;
	int UPlaneLength = YPlaneLength >> 2;


	int indx = 0;

	int iNewHeight = iHeight >> 1;
	int iNewWidth = iWidth >> 1;

	if (iNewHeight % 2 != 0) iNewHeight = iNewHeight - 1;
	if (iNewWidth % 2 != 0) iNewWidth = iNewWidth - 1;

	iNewHeight = iNewHeight * 2;
	iNewWidth = iNewWidth * 2;

	

	for (int i = 0; i<iNewHeight; i += 2)
	{
		for (int j = 0; j<iNewWidth; j += 2)
		{
			int w, x, y, z;
			w = pData[i*iWidth + j];
			x = pData[i*iWidth + j + 1];
			y = pData[(i + 1)*iWidth + j];
			z = pData[(i + 1)*iWidth + j + 1];
			int avg = (w + x + y + z) / 4;
			outputData[indx++] = (byte)avg;

		}
	}




	byte *p = pData + YPlaneLength;
	byte *q = pData + YPlaneLength + UPlaneLength;
	int uIndex = indx;
	int vIndex = indx + iNewHeight * iNewWidth;


	for (int i = 0; i<iNewHeight / 2; i += 2)
	{
		for (int j = 0; j<iNewWidth; j += 2)
		{
			int w, x, y, z, avg;


			w = p[i*iWidth + j];
			x = p[i*iWidth + j + 1];
			y = p[(i + 1)*iWidth + j];
			z = p[(i + 1)*iWidth + j + 1];
			avg = (w + x + y + z) / 4;
			outputData[uIndex++] = (byte)avg;


			w = q[i*iWidth + j];
			x = q[i*iWidth + j + 1];
			y = q[(i + 1)*iWidth + j];
			z = q[(i + 1)*iWidth + j + 1];
			avg = (w + x + y + z) / 4;
			outputData[vIndex++] = (byte)avg;
		}
	}

	iHeight = iNewHeight >> 1;
	iWidth = iNewWidth >> 1;

	return iHeight * iWidth * 3 / 2;

}

//Date: 07-January-2017
//Receive YUV420 Data, and address of Height and Width, and Diff. Example: Diff = 3 means video will become 1/3 of original video.

int CColorConverter::DownScaleYUV420_Dynamic(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData, int diff)
{
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
    int YPlaneLength = iHeight*iWidth;
    int UPlaneLength = YPlaneLength >> 2;
    
    
    int indx = 0;
    int H, W;
    
    int iNewHeight = iHeight/diff;
    int iNewWidth = iWidth/diff;
    
    H =  iHeight - iHeight%diff;
    W = iWidth - iWidth % diff;
    
    if(iNewHeight%2!=0)
    {
        iNewHeight--;
        H = iNewHeight * diff;
    }
    
    if(iNewWidth%2!=0)
    {
        iNewWidth--;
        W = iNewWidth * diff;
    }
    
    
    
   // printf("iNewHeight, iNewWidth --> %d, %d\n", iNewHeight, iNewWidth);
    
    int avg;
    
    for(int i=0; i<H; i+=diff)
    {
        for(int j=0; j<W; j+=diff)
        {
            int sum = 0;
            for(int k=i; k<(i+diff); k++)
            {
                for(int l=j; l<(j+diff); l++)
                {
                    sum+=pData[k*iWidth + l];
                }
            }
            avg = sum/(diff*diff);
            outputData[indx++] = (byte)avg;
            
        }
    }
    
    //printf("index = %d\n", indx);
    
    
    
    
    byte *p = pData + YPlaneLength;
    byte *q = pData + YPlaneLength + UPlaneLength;
    int uIndex = indx;
    int vIndex = indx + (iNewHeight * iNewWidth)/4;
    
    int halfH = H>>1, halfW = W>>1;
    int www = iWidth>>1;
    
    for(int i=0;i<halfH;i+=diff)
    {
        for(int j=0;j<halfW;j+=diff)
        {
            int sum1 = 0, sum2 = 0;
            
            for(int k=i; k<(i+diff); k++)
            {
                for(int l=j; l<(j+diff); l++)
                {
                    sum1+=p[k*www + l];
                    sum2+=q[k*www + l];
                }
            }
            
            avg = sum1/(diff*diff);
            outputData[uIndex++] = (byte)avg;
            
            avg = sum2/(diff*diff);
            outputData[vIndex++] = (byte)avg;
        }
    }
    
    //printf("uIndex, vIndex = %d, %d\n", uIndex, vIndex);
    
    //cout<<"CurrentLen = "<<indx<<endl;
    
    iHeight = iNewHeight;
    iWidth = iNewWidth;
    
    return iHeight * iWidth * 3 / 2;
    
}


int CColorConverter::CreateFrameBorder(unsigned char* pData, int iHeight, int iWidth, int Y, int U, int V)
{
    int iTotal = iHeight * iWidth;
    
    for(int i=0;i<iHeight;i++) //Traverse through each row
    {
        int rowIndx = i*iWidth;
        pData[rowIndx + 0] = Y;
        //pData[rowIndx + 1] = Y;
        pData[rowIndx + iWidth-1] = Y;
       // pData[rowIndx + iWidth-2] = Y;
        
        pData[getUIndex(iHeight, iWidth, i, 0, iTotal)] = U;
        //pData[getUIndex(iHeight, iWidth, i, 1, iTotal)] = U;
        pData[getUIndex(iHeight, iWidth, i, iWidth-1, iTotal)] = U;
        //pData[getUIndex(iHeight, iWidth, i, iWidth-2, iTotal)] = U;
        
        pData[getVIndex(iHeight, iWidth, i, 0, iTotal)] = V;
        //pData[getVIndex(iHeight, iWidth, i, 1, iTotal)] = V;
        pData[getVIndex(iHeight, iWidth, i, iWidth -1, iTotal)] = V;
        //pData[getVIndex(iHeight, iWidth, i, iWidth-2, iTotal)] = V;
        
    }
    
    for(int j=0;j<iWidth;j++) //Traverse through each Column
    {
        int colIndx = j;
        pData[0 + colIndx] = Y;
        //pData[iWidth + colIndx] = Y;
        pData[(iHeight-1)*iWidth + colIndx] = Y;
        //pData[(iHeight-2)*iWidth + colIndx] = Y;
        
        pData[getUIndex(iHeight, iWidth,  0, colIndx, iTotal)] = U;
        //pData[getUIndex(iHeight, iWidth,  1, colIndx, iTotal)] = U;
        pData[getUIndex(iHeight, iWidth,  (iHeight-1), colIndx, iTotal)] = U;
        //pData[getUIndex(iHeight, iWidth,  (iHeight-2), colIndx, iTotal)] = U;
        
        pData[getVIndex(iHeight, iWidth,  0, colIndx, iTotal)] = V;
        //pData[getVIndex(iHeight, iWidth,  1, colIndx, iTotal)] = V;
        pData[getVIndex(iHeight, iWidth,  (iHeight-1), colIndx, iTotal)] = V;
        //pData[getVIndex(iHeight, iWidth,  (iHeight-2), colIndx, iTotal)] = V;
        
        
    }
    
    return iHeight * iWidth * 3 / 2;
}

void CColorConverter::SetSmallFrame(unsigned char * smallFrame, int iHeight, int iWidth, int nLength, int iTargetHeight, int iTargetWidth, bool bShouldBeCropped, int libraryVersion)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	memcpy(m_pSSSmallFrame, smallFrame, iHeight * iWidth * 3 / 2);

	m_nOponentHeight = iHeight;
	m_nOponentWidth = iWidth;

	CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CColorConverter::SetSmallFrame 0 iHeight %d, iWidth %d, iTargetHeight %d, iTargetWidth %d", iHeight, iWidth, iTargetHeight, iTargetWidth);
    
    //int iLen = DownScaleYUV420_Dynamic(smallFrame, iHeight, iWidth, m_pSmallFrame, 3 /*Making 1/3 rd of original Frame*/);
    int iOutputHeight, iOutputWidth;
    
    iOutputHeight = iTargetHeight/3;
    double ratio = (iHeight*1.0)/(iWidth*1.0);
    iOutputWidth = ceil(iOutputHeight*1.0/ratio); //todo: Need to analysis later

    iOutputHeight = iOutputHeight - iOutputHeight%4;
    iOutputWidth = iOutputWidth - iOutputWidth%4;
    printf("TheKing--> iOutputHeight:iOutputWidth = %d:%d\n", iOutputHeight, iOutputWidth);


	CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CColorConverter::SetSmallFrame 1 iOutputHeight %d, iOutputWidth %d", iOutputHeight, iOutputWidth);
    
    CLogPrinter::Log("TheKing--> iHeight, iWidth, iTargetHeight, iTargetWidth, outputHeight, outputWidth = %d:%d, %d:%d, %d:%d\n", iHeight, iWidth, iTargetHeight, iTargetWidth, iOutputHeight, iOutputWidth);
    
    int iLen = DownScaleYUV420_Dynamic_Version2(smallFrame, iHeight, iWidth, m_pSmallFrame, iOutputHeight, iOutputWidth);

    

        
    
    iHeight = iOutputHeight;
    iWidth = iOutputWidth;
    
    int iNewHeight, iNewWidth, diff_width, diff_height;
    
    if(bShouldBeCropped)
    {
        CalculateAspectRatioWithScreenAndModifyHeightWidth(iHeight, iWidth, m_iDeviceHeight, m_iDeviceWidth, iNewHeight, iNewWidth);

		CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CColorConverter::SetSmallFrame 2 iHeight %d, iWidth %d, iNewHeight %d, iNewWidth %d", iHeight, iWidth, iNewHeight, iNewWidth);
    
        if(iHeight == iNewHeight && iWidth == iNewWidth)
        {
            //Do Nothing
        }
        else
        {
            memcpy(smallFrame, m_pSmallFrame, iHeight * iWidth * 3 / 2);
        
            diff_width = iWidth - iNewWidth;
            diff_height = iHeight - iNewHeight;
            Crop_YUV420(smallFrame, iHeight, iWidth, diff_width/2, diff_width/2, diff_height/2, diff_height/2, m_pSmallFrame, iNewHeight, iNewWidth);

			CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CColorConverter::SetSmallFrame 3 iHeight %d, iWidth %d, iNewHeight %d, iNewWidth %d", iHeight, iWidth, iNewHeight, iNewWidth);
        }
    
        iHeight = iNewHeight;
        iWidth = iNewWidth;
    }
    
    //TheKing--> Trying to reduce the Height
    printf("TheKing--> libraryVersion = %d\n", libraryVersion);
    if(libraryVersion >= 9 )
    {
        iOutputHeight = iHeight;
        iOutputWidth = iWidth;
        
        iOutputHeight = iOutputHeight * 4;
        iOutputHeight = iOutputHeight / 5;
        iOutputHeight = iOutputHeight - iOutputHeight%4;
        memcpy(smallFrame, m_pSmallFrame, iHeight * iWidth * 3 / 2);
        Crop_YUV420(smallFrame, iHeight, iWidth, 0, 0, (iHeight - iOutputHeight)/2, (iHeight - iOutputHeight)/2, m_pSmallFrame, iOutputHeight, iOutputWidth);
        printf("TheKing--> Updated: iOutputHeight:iOutputWidth = %d:%d\n", iOutputHeight, iOutputWidth);
        
        iHeight = iOutputHeight;
        iWidth = iOutputWidth;
    }
    //End Reducing Height
    
    
    m_iSmallFrameHeight = iHeight;
    m_iSmallFrameWidth = iWidth;

	CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CColorConverter::SetSmallFrame 4 m_iSmallFrameHeight %d, m_iSmallFrameWidth %d", m_iSmallFrameHeight, m_iSmallFrameWidth);

	m_iSmallFrameSize = iHeight * iWidth * 3 / 2;
    //iLen = CreateFrameBorder(m_pSmallFrame, iHeight, iWidth, 0, 128, 128); // [Y:0, U:128, V:128] = Black

	if (m_bMergingSmallFrameEnabled == false)
	{
		m_bMergingSmallFrameEnabled = true;

		m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->LIVE_CALL_INSET_ON);
	}
	
 
	//memcpy(m_pSmallFrame, smallFrame, nLength);
}



int CColorConverter::DownScaleYUV420_Dynamic_Version2(unsigned char* pData, int inHeight, int inWidth, unsigned char* outputData, int outHeight, int outWidth)
{
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
    double ratioHeight, ratioWidth;
    
    int YPlaneLength = inHeight*inWidth;
    int UPlaneLength = YPlaneLength >> 2;
    int halfH = inHeight>>1, halfW = inWidth>>1;
    
    ratioHeight = inHeight * (1.0) / outHeight;
    ratioWidth = inWidth * (1.0) / outWidth;
    int MaximumFraction = 10000;
    int factorH = (int)floor(ratioHeight);
    int factorW = (int)floor(ratioWidth);
    int fractionH = (int)((ratioHeight - factorH) * MaximumFraction);
    int fractionW = (int)((ratioWidth - factorW) * MaximumFraction);
    
    
    
    int indx = 0;
    
    int avg, sum, sum1, sum2, Valuecounter;
    int ii, iiFraction,jj, jjFraction;
    
    int iHeightCounter = 0, iWidthCounter = 0;
    
    
    CumulativeSum[0][0] = (int)pData[0];
    
    for(int i=1, iw = inWidth;i<inHeight; i++, iw += inWidth)
    {
        CumulativeSum[i][0] = (int)(CumulativeSum[i-1][0] + pData[iw]);
    }
    for(int j=1;j<inWidth;j++)
    {
        CumulativeSum[0][j] = (int)(CumulativeSum[0][j-1] + (int)pData[j]);
    }
    
    for(int i=1, iw = inWidth;i<inHeight;i++, iw += inWidth)
    {
        for(int j=1;j<inWidth;j++)
        {
            CumulativeSum[i][j] = (int)(CumulativeSum[i][j-1]  + CumulativeSum[i-1][j] - CumulativeSum[i-1][j-1] + pData[iw+j]);
        }
    }
    
    for(ii=0, iiFraction = 0; ii<inHeight ; ii+=factorH, iiFraction+=fractionH)
    {
        if(iiFraction>=MaximumFraction)
        {
            ii++;
            iiFraction-=MaximumFraction;
        }
        iHeightCounter++;
        
        if(iHeightCounter>outHeight) break;
        iWidthCounter = 0;
        
        for(jj=0, jjFraction = 0; jj<inWidth ; jj+=factorW, jjFraction+=fractionW)
        {
            if(jjFraction>=MaximumFraction)
            {
                jj++;
                jjFraction-=MaximumFraction;
            }
            
            iWidthCounter++;
            if(iWidthCounter>outWidth) break;
            
            sum = 0;
            Valuecounter = 0;
            
            int startY = ii;
            int endY = ii+factorH-1;
            if(iiFraction + fractionH >= MaximumFraction) endY++;
            
            int startX = jj;
            int endX = jj+factorW-1;
            if(jjFraction + fractionW >= MaximumFraction) endX++;
            
            
            Valuecounter = (endY - startY + 1) * (endX - startX + 1);
            int now, corner, up, left;
            
            corner = (startX-1) < 0 ? 0: (startY-1) < 0 ? 0 : CumulativeSum[startY-1][startX-1];
            left = (startX-1) < 0 ? 0: endY >= inHeight ? 0 : CumulativeSum[endY][startX-1];
            up = endX >= inWidth ? 0: (startY-1) < 0 ? 0 : CumulativeSum[startY-1][endX];
            now = endX >= inWidth ? 0: (endY) >= inHeight ? 0 : CumulativeSum[endY][endX];
            
            sum = now - up - left + corner;
            avg = sum/Valuecounter;
            outputData[indx++] = (byte)avg;
            
            //printf("RajibTheKing--> sum = %d values = %d ,now = %d ,up = %d, left = %d, corner = %d\n", sum, Valuecounter,now,up,left,corner);
            
        }
    }
    
    //printf("index = %d\n", indx);
    
    byte *p = pData + YPlaneLength;
    byte *q = pData + YPlaneLength + UPlaneLength;
    int uIndex = indx;
    int vIndex = indx + (outHeight * outWidth)/4;
    iHeightCounter = 0;
    
    
    CumulativeSum_U[0][0] = (int)p[0];
    CumulativeSum_V[0][0] = (int)q[0];
    
    for(int i=1, iw = halfW;i<halfH; i++, iw += halfW)
    {
        CumulativeSum_U[i][0] = (int)(CumulativeSum_U[i-1][0] + p[iw]);
        CumulativeSum_V[i][0] = (int)(CumulativeSum_V[i-1][0] + q[iw]);
    }
    for(int j=1;j<halfW;j++)
    {
        CumulativeSum_U[0][j] = (int)(CumulativeSum_U[0][j-1] + (int)p[j]);
        CumulativeSum_V[0][j] = (int)(CumulativeSum_V[0][j-1] + (int)q[j]);
    }
    
    for(int i=1, iw = halfW;i<halfH;i++, iw += halfW)
    {
        for(int j=1;j<halfW;j++)
        {
            CumulativeSum_U[i][j] = (int)(CumulativeSum_U[i][j-1] + p[iw+j] + CumulativeSum_U[i-1][j] - CumulativeSum_U[i-1][j-1]);
            CumulativeSum_V[i][j] = (int)(CumulativeSum_V[i][j-1] + q[iw+j] + CumulativeSum_V[i-1][j] - CumulativeSum_V[i-1][j-1]);
        }
    }
    
    for(ii=0, iiFraction = 0; ii<halfH ; ii+=factorH, iiFraction+=fractionH)
    {
        if(iiFraction>=MaximumFraction)
        {
            ii++;
            iiFraction-=MaximumFraction;
        }
        iHeightCounter++;
        
        if(iHeightCounter > (outHeight>>1) ) break;
        iWidthCounter = 0;
        
        for(jj=0, jjFraction = 0; jj<halfW ; jj+=factorW, jjFraction+=fractionW)
        {
            if(jjFraction>=MaximumFraction)
            {
                jj++;
                jjFraction-=MaximumFraction;
            }
            iWidthCounter++;
            if(iWidthCounter> (outWidth>>1) )  break;
            
            sum1 = 0;
            sum2 = 0;
            Valuecounter = 0;
            
            int startY = ii;
            int endY = ii+factorH-1;
            if(iiFraction + fractionH >= MaximumFraction) endY++;
            
            int startX = jj;
            int endX = jj+factorW-1;
            if(jjFraction + fractionW >= MaximumFraction) endX++;
            
            
            Valuecounter = (endY - startY + 1) * (endX - startX + 1);
            //printf("Valuecounter = %d\n", Valuecounter);
            int now, corner, up, left;
            
            corner = (startX-1) < 0 ? 0: (startY-1) < 0 ? 0 : CumulativeSum_U[startY-1][startX-1];
            left = (startX-1) < 0 ? 0: endY >= halfH ? 0 : CumulativeSum_U[endY][startX-1];
            up = endX >= halfW ? 0: (startY-1) < 0 ? 0 : CumulativeSum_U[startY-1][endX];
            now = endX >= halfW ? 0: (endY) >= halfH ? 0 : CumulativeSum_U[endY][endX];
            
            sum = now - up - left + corner;
            avg = sum/Valuecounter;
            outputData[uIndex++] = (byte)avg;
            
            
            corner = (startX-1) < 0 ? 0: (startY-1) < 0 ? 0 : CumulativeSum_V[startY-1][startX-1];
            left = (startX-1) < 0 ? 0: endY >= halfH ? 0 : CumulativeSum_V[endY][startX-1];
            up = endX >= halfW ? 0: (startY-1) < 0 ? 0 : CumulativeSum_V[startY-1][endX];
            now = endX >= halfW ? 0: (endY) >= halfH ? 0 : CumulativeSum_V[endY][endX];
            
            sum = now - up - left + corner;
            avg = sum/Valuecounter;
            outputData[vIndex++] = (byte)avg;
            
        }
    }
    
    
    //printf("uIndex, vIndex = %d, %d\n", uIndex, vIndex);
    
    
    return outHeight * outWidth * 3 / 2;
    
}

int CColorConverter::DownScaleYUV420_Dynamic_Version222(unsigned char* pData, int inHeight, int inWidth, unsigned char* outputData, int outHeight, int outWidth)
{
	//cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
	double ratioHeight, ratioWidth;

	int YPlaneLength = inHeight*inWidth;
	int UPlaneLength = YPlaneLength >> 2;
	int halfH = inHeight >> 1, halfW = inWidth >> 1;

	ratioHeight = inHeight * (1.0) / outHeight;
	ratioWidth = inWidth * (1.0) / outWidth;
	int MaximumFraction = 10000;
	int factorH = (int)floor(ratioHeight);
	int factorW = (int)floor(ratioWidth);
	int fractionH = (int)((ratioHeight - factorH) * MaximumFraction);
	int fractionW = (int)((ratioWidth - factorW) * MaximumFraction);



	int indx = 0;

	int avg, sum, sum1, sum2, Valuecounter;
	int ii, iiFraction, jj, jjFraction;

	int iHeightCounter = 0, iWidthCounter = 0;


	CumulativeSum2[0][0] = (int)pData[0];

	for (int i = 1, iw = inWidth; i<inHeight; i++, iw += inWidth)
	{
		CumulativeSum2[i][0] = (int)(CumulativeSum2[i - 1][0] + pData[iw]);
	}
	for (int j = 1; j<inWidth; j++)
	{
		CumulativeSum2[0][j] = (int)(CumulativeSum2[0][j - 1] + (int)pData[j]);
	}

	for (int i = 1, iw = inWidth; i<inHeight; i++, iw += inWidth)
	{
		for (int j = 1; j<inWidth; j++)
		{
			CumulativeSum2[i][j] = (int)(CumulativeSum2[i][j - 1] + CumulativeSum2[i - 1][j] - CumulativeSum2[i - 1][j - 1] + pData[iw + j]);
		}
	}

	for (ii = 0, iiFraction = 0; ii<inHeight; ii += factorH, iiFraction += fractionH)
	{
		if (iiFraction >= MaximumFraction)
		{
			ii++;
			iiFraction -= MaximumFraction;
		}
		iHeightCounter++;

		if (iHeightCounter>outHeight) break;
		iWidthCounter = 0;

		for (jj = 0, jjFraction = 0; jj<inWidth; jj += factorW, jjFraction += fractionW)
		{
			if (jjFraction >= MaximumFraction)
			{
				jj++;
				jjFraction -= MaximumFraction;
			}

			iWidthCounter++;
			if (iWidthCounter>outWidth) break;

			sum = 0;
			Valuecounter = 0;

			int startY = ii;
			int endY = ii + factorH - 1;
			if (iiFraction + fractionH >= MaximumFraction) endY++;

			int startX = jj;
			int endX = jj + factorW - 1;
			if (jjFraction + fractionW >= MaximumFraction) endX++;


			Valuecounter = (endY - startY + 1) * (endX - startX + 1);
			int now, corner, up, left;

			corner = (startX - 1) < 0 ? 0 : (startY - 1) < 0 ? 0 : CumulativeSum2[startY - 1][startX - 1];
			left = (startX - 1) < 0 ? 0 : endY >= inHeight ? 0 : CumulativeSum2[endY][startX - 1];
			up = endX >= inWidth ? 0 : (startY - 1) < 0 ? 0 : CumulativeSum2[startY - 1][endX];
			now = endX >= inWidth ? 0 : (endY) >= inHeight ? 0 : CumulativeSum2[endY][endX];

			sum = now - up - left + corner;
			avg = sum / Valuecounter;
			outputData[indx++] = (byte)avg;

			//printf("RajibTheKing--> sum = %d values = %d ,now = %d ,up = %d, left = %d, corner = %d\n", sum, Valuecounter,now,up,left,corner);

		}
	}

	//printf("index = %d\n", indx);

	byte *p = pData + YPlaneLength;
	byte *q = pData + YPlaneLength + UPlaneLength;
	int uIndex = indx;
	int vIndex = indx + (outHeight * outWidth) / 4;
	iHeightCounter = 0;


	CumulativeSum_U2[0][0] = (int)p[0];
	CumulativeSum_V2[0][0] = (int)q[0];

	for (int i = 1, iw = halfW; i<halfH; i++, iw += halfW)
	{
		CumulativeSum_U2[i][0] = (int)(CumulativeSum_U2[i - 1][0] + p[iw]);
		CumulativeSum_V2[i][0] = (int)(CumulativeSum_V2[i - 1][0] + q[iw]);
	}
	for (int j = 1; j<halfW; j++)
	{
		CumulativeSum_U2[0][j] = (int)(CumulativeSum_U2[0][j - 1] + (int)p[j]);
		CumulativeSum_V2[0][j] = (int)(CumulativeSum_V2[0][j - 1] + (int)q[j]);
	}

	for (int i = 1, iw = halfW; i<halfH; i++, iw += halfW)
	{
		for (int j = 1; j<halfW; j++)
		{
			CumulativeSum_U2[i][j] = (int)(CumulativeSum_U2[i][j - 1] + p[iw + j] + CumulativeSum_U2[i - 1][j] - CumulativeSum_U2[i - 1][j - 1]);
			CumulativeSum_V2[i][j] = (int)(CumulativeSum_V2[i][j - 1] + q[iw + j] + CumulativeSum_V2[i - 1][j] - CumulativeSum_V2[i - 1][j - 1]);
		}
	}

	for (ii = 0, iiFraction = 0; ii<halfH; ii += factorH, iiFraction += fractionH)
	{
		if (iiFraction >= MaximumFraction)
		{
			ii++;
			iiFraction -= MaximumFraction;
		}
		iHeightCounter++;

		if (iHeightCounter >(outHeight >> 1)) break;
		iWidthCounter = 0;

		for (jj = 0, jjFraction = 0; jj<halfW; jj += factorW, jjFraction += fractionW)
		{
			if (jjFraction >= MaximumFraction)
			{
				jj++;
				jjFraction -= MaximumFraction;
			}
			iWidthCounter++;
			if (iWidthCounter>(outWidth >> 1))  break;

			sum1 = 0;
			sum2 = 0;
			Valuecounter = 0;

			int startY = ii;
			int endY = ii + factorH - 1;
			if (iiFraction + fractionH >= MaximumFraction) endY++;

			int startX = jj;
			int endX = jj + factorW - 1;
			if (jjFraction + fractionW >= MaximumFraction) endX++;


			Valuecounter = (endY - startY + 1) * (endX - startX + 1);
			//printf("Valuecounter = %d\n", Valuecounter);
			int now, corner, up, left;

			corner = (startX - 1) < 0 ? 0 : (startY - 1) < 0 ? 0 : CumulativeSum_U2[startY - 1][startX - 1];
			left = (startX - 1) < 0 ? 0 : endY >= halfH ? 0 : CumulativeSum_U2[endY][startX - 1];
			up = endX >= halfW ? 0 : (startY - 1) < 0 ? 0 : CumulativeSum_U2[startY - 1][endX];
			now = endX >= halfW ? 0 : (endY) >= halfH ? 0 : CumulativeSum_U2[endY][endX];

			sum = now - up - left + corner;
			avg = sum / Valuecounter;
			outputData[uIndex++] = (byte)avg;


			corner = (startX - 1) < 0 ? 0 : (startY - 1) < 0 ? 0 : CumulativeSum_V2[startY - 1][startX - 1];
			left = (startX - 1) < 0 ? 0 : endY >= halfH ? 0 : CumulativeSum_V2[endY][startX - 1];
			up = endX >= halfW ? 0 : (startY - 1) < 0 ? 0 : CumulativeSum_V2[startY - 1][endX];
			now = endX >= halfW ? 0 : (endY) >= halfH ? 0 : CumulativeSum_V2[endY][endX];

			sum = now - up - left + corner;
			avg = sum / Valuecounter;
			outputData[vIndex++] = (byte)avg;

		}
	}


	//printf("uIndex, vIndex = %d, %d\n", uIndex, vIndex);


	return outHeight * outWidth * 3 / 2;

}

int CColorConverter::DownScaleYUVNV12_YUVNV21_OneFourth(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData)
{
    if(iHeight % 4 == 0 && iWidth % 16 == 0)
    {
#if defined(HAVE_NEON) || defined(HAVE_NEON_AARCH64)
        //LOGE_MAIN("TheKing--> Here Inside DownScaleOneFourthAssembly = ");
        m_pNeonAssemblyWrapper->DownScaleOneFourthAssembly(pData, iHeight, iWidth, outputData);
#else
        
        
        int idx = 0;
        for (int i = 0; i < iHeight; i += 4)
        {
            for (int j = 0; j < iWidth; j += 4)
            {
                int tmp = 0;
                for(int k = i; k < i + 4; k++)
                {
                    int kw = k*iWidth;
                    for(int l = j; l < j + 4; l++)
                    {
                        tmp += pData[kw + l];
                    }
                }
                outputData[idx++] = tmp >> 4;
            }
        }
        
        int halfHeight = iHeight >> 1;
        int offset = iHeight*iWidth;
        
        for (int i = 0; i < halfHeight; i += 4)
        {
            for (int j = 0; j < iWidth; j += 8)
            {
                int tmpU = 0;
                int tmpV = 0;
                for(int k = i; k < i + 4; k++)
                {
                    int kw = offset + k*iWidth;
                    for(int l = j; l < j + 8; l++)
                    {
                        if (l % 2 == 0)
                        {
                            tmpU += pData[kw + l];
                        }
                        else
                        {
                            tmpV += pData[kw + l];
                        }
                    }
                }
                outputData[idx++] = tmpU >> 4;
                outputData[idx++] = tmpV >> 4;
            }
        }
#endif
    }
    else
    {
        int idx = 0;
        for (int i = 0; i < iHeight; i += 4)
        {
            for (int j = 0; j < iWidth; j += 4)
            {
                int tmp = 0;
                for(int k = i; k < i + 4; k++)
                {
                    int kw = k*iWidth;
                    for(int l = j; l < j + 4; l++)
                    {
                        tmp += pData[kw + l];
                    }
                }
                outputData[idx++] = tmp >> 4;
            }
        }
        
        int halfHeight = iHeight >> 1;
        int offset = iHeight*iWidth;
        
        for (int i = 0; i < halfHeight; i += 4)
        {
            for (int j = 0; j < iWidth; j += 8)
            {
                int tmpU = 0;
                int tmpV = 0;
                for(int k = i; k < i + 4; k++)
                {
                    int kw = offset + k*iWidth;
                    for(int l = j; l < j + 8; l++)
                    {
                        if (l % 2 == 0)
                        {
                            tmpU += pData[kw + l];
                        }
                        else
                        {
                            tmpV += pData[kw + l];
                        }
                    }
                }
                outputData[idx++] = tmpU >> 4;
                outputData[idx++] = tmpV >> 4;
            }
        }
    }

    
    int outHeight = iHeight >> 2;
    int outWidth = iWidth >> 2;
    
    return (outHeight * outWidth * 3) >> 1;
    
}

int CColorConverter::DownScaleYUV420_OneFourth(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData)
{
    int idx = 0;
    for (int i = 4; i < iHeight - 4; i += 4)
    {
        for (int j = 0; j < iWidth; j += 4)
        {
            int tmp = 0;
            for(int k = i; k < i + 4; k++)
            {
                int kw = k*iWidth;
                for(int l = j; l < j + 4; l++)
                {
                    tmp += pData[kw + l];
                }
            }
            outputData[idx++] = tmp >> 4;
        }
    }

	int quarterHeight = iHeight >> 2;
	int offset = iHeight*iWidth;
    int incr = 4;

    for (int i = 1; i < quarterHeight - 1; i += 4)
    {
        for (int j = 0; j < iWidth; j += incr)
        {
            int tmpU = 0;
            if(i + 4 > quarterHeight)
            {
                incr = 8;
                for(int k = i; k < i + 2; k++)
                {
                    int kw = offset + k*iWidth;
                    for(int l = j; l < j + 8; l++)
                    {
                        tmpU += pData[kw + l];
                    }
                }
            }
            else
            {
                for(int k = i; k < i + 4; k++)
                {
                    int kw = offset + k*iWidth;
                    for(int l = j; l < j + 4; l++)
                    {
                        tmpU += pData[kw + l];
                    }
                }
            }
            outputData[idx++] = tmpU >> 4;
        }
    }

	offset += quarterHeight*iWidth;
    incr = 4;

    for (int i = 1; i < quarterHeight - 1; i += 4)
    {
        for (int j = 0; j < iWidth; j += incr)
        {
            int tmpV = 0;
            if(i + 4 > quarterHeight)
            {
                incr = 8;
                for(int k = i; k < i + 2; k++)
                {
                    int kw = offset + k*iWidth;
                    for(int l = j; l < j + 8; l++)
                    {
                        tmpV += pData[kw + l];
                    }
                }
            }
            else
            {
                for(int k = i; k < i + 4; k++)
                {
                    int kw = offset + k*iWidth;
                    for(int l = j; l < j + 4; l++)
                    {
                        tmpV += pData[kw + l];
                    }
                }
            }
            outputData[idx++] = tmpV >> 4;
        }
    }

	int outHeight = iHeight >> 2;
	int outWidth = iWidth >> 2;

	return (outHeight * outWidth * 3) >> 1;
}

//This Function will return UIndex based on YUV_420 Data
int CColorConverter::getUIndex(int h, int w, int yVertical, int xHorizontal, int& total)
{
    return (yVertical>>1) * (w>>1) + (xHorizontal>>1) + total; //total = h * w
}

//This Function will return VIndex based on YUV_420 Data
int CColorConverter::getVIndex(int h, int w, int yVertical, int xHorizontal, int& total)
{
    return (yVertical>>1) * (w>>1) + (xHorizontal>>1) + total + (total>>2); //total = h * w
}

//This Function will return UIndex based on NV12 Data
int CColorConverter::getUIndexforNV12(int h, int w, int yVertical, int xHorizontal, int& total)
{
	return (((yVertical >> 1) * (w >> 1) + (xHorizontal >> 1)) << 1) + total; //total = h * w
}

//This Function will return VIndex based on NV12 Data
int CColorConverter::getVIndexforNV12(int h, int w, int yVertical, int xHorizontal, int& total)
{
	return ((((yVertical >> 1) * (w >> 1) + (xHorizontal >> 1)) << 1) | 1) + total; //total = h * w
}

//This Function will return UIndex based on NV21 Data
int CColorConverter::getUIndexforNV21(int h, int w, int yVertical, int xHorizontal, int& total)
{
	return ((((yVertical >> 1) * (w >> 1) + (xHorizontal >> 1)) << 1) | 1) + total; //total = h * w
}

//This Function will return VIndex based on NV21 Data
int CColorConverter::getVIndexforNV21(int h, int w, int yVertical, int xHorizontal, int& total)
{
	return (((yVertical >> 1) * (w >> 1) + (xHorizontal >> 1)) << 1) + total; //total = h * w
}

//Date: 28-December-2016
//Constraits:
// i) Receive Big YUV420 Data  and  Small YUV420 Data, Finally Output Merged YUV420 Data
// ii) iPosX is Distance in Horizontal Direction. Value must be in Range iPosX >= 0 and iPosX < (BigDataWidth - SmallDataWidth)
// ii) iPosY is Distance in Vertical Direction. Value must be in Range iPosY >= 0 and iPosY < (BigDataHeight - SmallDataHeight)

int CColorConverter::Merge_Two_Video(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	if (m_bMergingSmallFrameEnabled == false)
		return 0;

	int h1 = iVideoHeight;
	int w1 = iVideoWidth;
	int h2 = /* m_iVideoHeight >> 2 */ m_iSmallFrameHeight;
	int w2 = /* m_iVideoWidth >> 2 */ m_iSmallFrameWidth;

	CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoEncodingThread::Merge_Two_Video h1 %d w1 %d h2 %d w2 %d", h1, w1, h2, w2);

	int iLen1 = h1 * w1 * 3 / 2;
	int iLen2 = h2 * w2 * 3 / 2;

	int total1 = h1 * w1, total2 = h2 * w2;

	for (int i = iPosY; i<(iPosY + h2); i++)
	{
        int j = iPosX;
        int ii = i - iPosY;
        int jj = j - iPosX;
        int now1 = i*w1 + j;
        int now2 = ii*w2 + jj;
        memcpy(&pInData1[now1], &m_pSmallFrame[now2], w2);
        memcpy(&pInData1[getUIndex(h1, w1, i, j, total1)], &m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)], w2/2);
        memcpy(&pInData1[getVIndex(h1, w1, i, j, total1)], &m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)], w2/2);
        /*
		for (int j = iPosX; j<(iPosX + w2); j++)
		{
			int ii = i - iPosY;
			int jj = j - iPosX;
			int now1 = i*w1 + j;
			int now2 = ii*w2 + jj;

			pInData1[now1] = m_pSmallFrame[now2];
			//pInData1[getUIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)];
			//pInData1[getVIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)];
		}
        */
	}
	return iLen1;
}

int CColorConverter::Merge_Two_Video2(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth, unsigned char *pSmallData, int iSmallHeight, int iSmallWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	if (m_bMergingSmallFrameEnabled == false)
		return 0;

	int h1 = iVideoHeight;
	int w1 = iVideoWidth;
	int h2 = /* m_iVideoHeight >> 2 */ iSmallHeight;
	int w2 = /* m_iVideoWidth >> 2 */ iSmallWidth;

	CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoEncodingThread::Merge_Two_Video h1 %d w1 %d h2 %d w2 %d", h1, w1, h2, w2);

	int iLen1 = h1 * w1 * 3 / 2;
	int iLen2 = h2 * w2 * 3 / 2;

	int total1 = h1 * w1, total2 = h2 * w2;

	for (int i = iPosY; i<(iPosY + h2); i++)
	{
		for (int j = iPosX; j<(iPosX + w2); j++)
		{
			int ii = i - iPosY;
			int jj = j - iPosX;
			int now1 = i*w1 + j;
			int now2 = ii*w2 + jj;

			pInData1[now1] = pSmallData[now2];
			pInData1[getUIndex(h1, w1, i, j, total1)] = pSmallData[getUIndex(h2, w2, ii, jj, total2)];
			pInData1[getVIndex(h1, w1, i, j, total1)] = pSmallData[getVIndex(h2, w2, ii, jj, total2)];
		}
	}

	return iLen1;
}

int CColorConverter::Merge_Two_Video3(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth, unsigned char *pSmallData, int iSmallHeight, int iSmallWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	if (m_bMergingSmallFrameEnabled == false)
		return 0;

	int h1 = iVideoHeight;
	int w1 = iVideoWidth;
	int h2 = /* m_iVideoHeight >> 2 */ iSmallHeight;
	int w2 = /* m_iVideoWidth >> 2 */ iSmallWidth;

	CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CVideoEncodingThread::Merge_Two_Video h1 %d w1 %d h2 %d w2 %d", h1, w1, h2, w2);

	int iLen1 = h1 * w1 * 3 / 2;
	int iLen2 = h2 * w2 * 3 / 2;

	int total1 = h1 * w1, total2 = h2 * w2;

	for (int i = iPosY; i<(iPosY + h2); i++)
	{
		for (int j = iPosX; j<(iPosX + w2); j++)
		{
			int ii = i - iPosY;
			int jj = j - iPosX;
			int now1 = i*w1 + j;
			int now2 = ii*w2 + jj;

			pInData1[now1] = (unsigned char)0;
			pInData1[getUIndex(h1, w1, i, j, total1)] = (unsigned char)128;
			pInData1[getVIndex(h1, w1, i, j, total1)] = (unsigned char)128;
		}
	}

	return iLen1;
}

int CColorConverter::Merge_Two_Video_With_Round_Corner(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	if (m_bMergingSmallFrameEnabled == false)
		return 0;

	int h1 = iVideoHeight;
	int w1 = iVideoWidth;
	int h2 = /* m_iVideoHeight >> 2 */ m_iSmallFrameHeight;
	int w2 = /* m_iVideoWidth >> 2 */ m_iSmallFrameWidth;

	int iLen1 = h1 * w1 * 3 / 2;
	int iLen2 = h2 * w2 * 3 / 2;

	int total1 = h1 * w1, total2 = h2 * w2;

	int smallRacRadius = 10;

	int centerPointX_1 = iPosX + smallRacRadius;
	int centerPointY_1  = iPosY + smallRacRadius;
	int centerPointX_2 = iPosX + w2 - smallRacRadius;
	int centerPointY_2  = iPosY + smallRacRadius;
	int centerPointX_3 = iPosX + smallRacRadius;
	int centerPointY_3  = iPosY + h2 - smallRacRadius;
	int centerPointX_4 = iPosX + w2 - smallRacRadius;
	int centerPointY_4  = iPosY + h2 - smallRacRadius;
    
	for (int i = iPosY; i<(iPosY + h2); i++)
	{
        if(i>centerPointY_1 && i<centerPointY_3)
        {
            int j = iPosX;
            int ii = i - iPosY;
            int jj = j - iPosX;
            int now1 = i*w1 + j;
            int now2 = ii*w2 + jj;
            memcpy(&pInData1[now1], &m_pSmallFrame[now2], w2);
            memcpy(&pInData1[getUIndex(h1, w1, i, j, total1)], &m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)], w2/2);
            memcpy(&pInData1[getVIndex(h1, w1, i, j, total1)], &m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)], w2/2);
            
        }
        else
        {
            for (int j = iPosX; j<(iPosX + w2); j++)
            {
                int ii = i - iPosY;
                int jj = j - iPosX;
                int now1 = i*w1 + j;
                int now2 = ii*w2 + jj;
                
                
                if(j <= centerPointX_1 && i <= centerPointY_1)
                {
                    int distanse_1 = (int)ceil(sqrt(((centerPointX_1 - j)*(centerPointX_1 - j)) + ((centerPointY_1 - i)*(centerPointY_1 - i))));
                    if(distanse_1 <= smallRacRadius)
                    {
                        pInData1[now1] = m_pSmallFrame[now2];
                        pInData1[getUIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)];
                        pInData1[getVIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)];
                    }
                }else if(j >= centerPointX_2 && i <= centerPointY_2)
                {
                    int distanse_2 = (int)ceil(sqrt(((centerPointX_2 - j)*(centerPointX_2 - j)) + ((centerPointY_2 - i)*(centerPointY_2 - i))));
                    if(distanse_2 <= smallRacRadius)
                    {
                        pInData1[now1] = m_pSmallFrame[now2];
                        pInData1[getUIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)];
                        pInData1[getVIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)];
                    }
                }else if(j <= centerPointX_3 && i >= centerPointY_3)
                {
                    int distanse_3 = (int)ceil(sqrt(((centerPointX_3 - j)*(centerPointX_3 - j)) + ((centerPointY_3 - i)*(centerPointY_3 - i))));
                    if(distanse_3 <= smallRacRadius)
                    {
                        pInData1[now1] = m_pSmallFrame[now2];
                        pInData1[getUIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)];
                        pInData1[getVIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)];
                    }
                }else if(j >= centerPointX_4 && i >= centerPointY_4)
                {
                    int distanse_4 = (int)ceil(sqrt(((centerPointX_4 - j)*(centerPointX_4 - j)) + ((centerPointY_4 - i)*(centerPointY_4 - i))));
                    if(distanse_4 <= smallRacRadius)
                    {
                        pInData1[now1] = m_pSmallFrame[now2];
                        pInData1[getUIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)];
                        pInData1[getVIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)];
                    }
                }else{
                    pInData1[now1] = m_pSmallFrame[now2];
                    pInData1[getUIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)];
                    pInData1[getVIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)];
                }
                
            }
        }
		
	}
    
    
	return iLen1;
}

int CColorConverter::Merge_Two_VideoNV12(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	if (m_bMergingSmallFrameEnabled == false)
		return 0;

	int h1 = iVideoHeight;
	int w1 = iVideoWidth;
	int h2 = /* m_iVideoHeight >> 2 */ m_iSmallFrameHeight;
	int w2 = /* m_iVideoWidth >> 2 */ m_iSmallFrameWidth;

	int iLen1 = h1 * w1 * 3 / 2;
	int iLen2 = h2 * w2 * 3 / 2;

	int total1 = h1 * w1, total2 = h2 * w2;

	for (int i = iPosY; i<(iPosY + h2); i++)
	{
		for (int j = iPosX; j<(iPosX + w2); j++)
		{
			int ii = i - iPosY;
			int jj = j - iPosX;
			int now1 = i*w1 + j;
			int now2 = ii*w2 + jj;

			pInData1[now1] = m_pSmallFrame[now2];
			pInData1[getUIndexforNV12(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndexforNV12(h2, w2, ii, jj, total2)];
			pInData1[getVIndexforNV12(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndexforNV12(h2, w2, ii, jj, total2)];
		}
	}

	return iLen1;
}

int CColorConverter::Merge_Two_VideoNV21(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	if (m_bMergingSmallFrameEnabled == false)
		return 0;

	int h1 = iVideoHeight;
	int w1 = iVideoWidth;
	int h2 = /* m_iVideoHeight >> 2 */ m_iSmallFrameHeight;
	int w2 = /* m_iVideoWidth >> 2 */ m_iSmallFrameWidth;

    int iLen1 = h1 * w1 * 3 / 2;
    int iLen2 = h2 * w2 * 3 / 2;
    
    int total1 = h1 * w1, total2 = h2 * w2;
    
    for(int i=iPosY;i<(iPosY+h2);i++)
    {
        for(int j=iPosX;j<(iPosX+w2);j++)
        {
            int ii = i-iPosY;
            int jj = j-iPosX;
            int now1 = i*w1 + j;
            int now2 = ii*w2 + jj;
            
			pInData1[now1] = m_pSmallFrame[now2];
			pInData1[getUIndexforNV21(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndexforNV21(h2, w2, ii, jj, total2)];
			pInData1[getVIndexforNV21(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndexforNV21(h2, w2, ii, jj, total2)];
        }
    }
    
    return iLen1;
}

void CColorConverter::CalculateAspectRatioWithScreenAndModifyHeightWidth(int inHeight, int inWidth, int iScreenHeight, int iScreenWidth, int &newHeight, int &newWidth)
{
    double aspectRatio_Screen, aspectRatio_VideoData;
    
    aspectRatio_Screen = iScreenHeight * 1.0 / iScreenWidth;
    aspectRatio_VideoData = inHeight * 1.0 / inWidth;
    
    if(fabs(aspectRatio_Screen - aspectRatio_VideoData) < 0.1)
    {
        //Do Nothing
        newHeight = inHeight;
        newWidth = inWidth;
        
    }
    else if(aspectRatio_Screen > aspectRatio_VideoData)
    {
        //We have to delete columns [reduce Width]
        newWidth = (int)floor(inHeight / aspectRatio_Screen);
        
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
        newHeight = (int)floor(inWidth * aspectRatio_Screen);
        newHeight = newHeight - newHeight%4;
        newWidth = inWidth;
    }
    //printf("ratio_screen = %f, ratio_VideoData = %f, diff = %f, inH:inW = %d:%d --> newH:newW = %d:%d\n", aspectRatio_Screen, aspectRatio_VideoData, fabs(aspectRatio_Screen-aspectRatio_VideoData), inHeight, inWidth, newHeight, newWidth);
}
int CColorConverter::GetInsetLocation(int inHeight, int inWidth, int &iPosX, int &iPosY)
{
    int newHeight, newWidth, diff_Height, diff_Width;
    
    CalculateAspectRatioWithScreenAndModifyHeightWidth(inHeight, inWidth, m_iDeviceHeight, m_iDeviceWidth, newHeight, newWidth);
    
    int iInsetLowerPadding = (int)((inHeight*10)/100);
    
    diff_Width = inWidth - newWidth;
    diff_Height = inHeight - newHeight;
    
    iPosX = inWidth - m_iSmallFrameWidth - diff_Width/2;
    iPosY = inHeight - m_iSmallFrameHeight - iInsetLowerPadding - diff_Height/2;

	CLogPrinter_LOG(LIVE_INSET_LOG, "LIVE_INSET_LOG CColorConverter::GetInsetLocation 0 iPosX %d, iPosY %d m_iSmallFrameWidth %d m_iSmallFrameHeight %d iInsetLowerPadding %d", iPosX, iPosY, m_iSmallFrameWidth, m_iSmallFrameHeight, iInsetLowerPadding);

    
    //printf("TheKing--> H:W = %d:%d, nH:nW = %d:%d, iPosY:iPosX = %d:%d\n", inHeight, inWidth, newHeight, newWidth, iPosY, iPosX);
    
    return 1;
}

int CColorConverter::CropWithAspectRatio_YUVNV12_YUVNV21_RGB24(unsigned char* pData, int inHeight, int inWidth, int screenHeight, int screenWidth, unsigned char* outputData, int &outHeight, int &outWidth, int pColorFormat)
{
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;

	if (screenHeight == -1 || screenWidth == -1)
		return 0;
    
    int newHeight, newWidth, diff_width, diff_height;
    
    CalculateAspectRatioWithScreenAndModifyHeightWidth(inHeight, inWidth, screenHeight, screenWidth, newHeight, newWidth);
    //LOGE_MAIN("TheKing--> fahad -->> CropWithAspectRatio_YUVNV12_YUVNV21_RGB24 : inHeight = %d, newHeight = %d, inWidth = %d, newWidth = %d",inHeight, newHeight, inWidth, newWidth);
    if(inHeight == newHeight && inWidth == newWidth)
    {
        //Do Nothing
		if (pColorFormat == RGB24)
			memcpy(outputData, pData, inHeight*inWidth * 3);
		else
			memcpy(outputData, pData, inHeight*inWidth * 3 / 2);

    }
    else
    {
        diff_width = inWidth - newWidth;
        diff_height = inHeight - newHeight;
        if(pColorFormat == RGB24)
        {
            Crop_RGB24(pData, inHeight, inWidth, diff_width/2, diff_width/2, diff_height/2, diff_height/2, outputData, newHeight, newWidth);
        }
        else if(pColorFormat == YUVNV12 || pColorFormat == YUVNV21)
        {
            Crop_YUVNV12_YUVNV21(pData, inHeight, inWidth, diff_width/2, diff_width/2, diff_height/2, diff_height/2, outputData, newHeight, newWidth);
        }
        else if(pColorFormat == YUVYV12)
        {
            //This function will for YUVYV12 and YUV420 color format
            Crop_YUV420(pData, inHeight, inWidth, diff_width/2, diff_width/2, diff_height/2, diff_height/2, outputData, newHeight, newWidth);
        }
		
    }
    
    outHeight = newHeight;
    outWidth = newWidth;
	
	if (pColorFormat == RGB24)
		return outHeight*outWidth*3;
	else
		return outHeight*outWidth * 3 / 2;
    
}
int CColorConverter::Crop_RGB24(unsigned char* pData, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* outputData, int &outHeight, int &outWidth)
{
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
    int indx = 0;
    outHeight = inHeight - startYDiff - endYDiff;
    outWidth = inWidth - startXDiff - endXDiff;
    
    
    for(int i=startYDiff; i<(inHeight-endYDiff); i++)
    {
        for(int j=startXDiff*3; j<(inWidth-endXDiff)*3; j++)
       	{
            outputData[indx++] = pData[i*(inWidth*3) + j];
            
        }
    }
    //cout<<"indx = "<<indx<<endl;
    
    return outWidth * outHeight * 3;
}
    
int CColorConverter::Crop_YUV420(unsigned char* pData, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* outputData, int &outHeight, int &outWidth)
{
   
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(ANDROID)
#if defined(HAVE_NEON) || defined(HAVE_NEON_AARCH64)
    //LOGE_MAIN("TheKing--> Here Inside Crop_yuv420_assembly = inHeight = %d, inWidth= %d, startXDiff = %d, endXDiff = %d", inHeight, inWidth, startXDiff, endXDiff);
    m_pNeonAssemblyWrapper->Crop_yuv420_assembly(pData, inHeight, inWidth, startXDiff, endXDiff, startYDiff, endYDiff, outputData, outHeight, outWidth);
    return outHeight * outWidth * 3 / 2;
#else
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
    //printf("TheKing--> Here Inside Crop_YUV420\n");
    int YPlaneLength = inHeight*inWidth;
    int UPlaneLength = YPlaneLength >> 2;
    int indx = 0;
    outHeight = inHeight - startYDiff - endYDiff;
    outWidth = inWidth - startXDiff - endXDiff;
    
    
    for(int i=startYDiff; i<(inHeight-endYDiff); i++)
    {
        for(int j=startXDiff; j<(inWidth-endXDiff); j++)
        {
            outputData[indx++] = pData[i*inWidth + j];
        }
    }
    
    
    byte *p = pData + YPlaneLength;
    byte *q = pData + YPlaneLength + UPlaneLength;
    
    int uIndex = indx;
    int vIndex = indx + (outHeight * outWidth)/4;
    
    int halfH = inHeight>>1, halfW = inWidth>>1;
    
    for(int i=startYDiff/2; i<(halfH-endYDiff/2); i++)
    {
        for(int j=startXDiff/2; j<(halfW-endXDiff/2); j++)
        {
            outputData[uIndex] = p[i*halfW + j];
            outputData[vIndex] = q[i*halfW + j];
            uIndex++;
            vIndex++;
        }
    }
    
    
    
    //printf("Now, First Block, H:W -->%d,%d  Indx = %d, uIndex = %d, vIndex = %d\n", outHeight, outWidth, indx, uIndex, vIndex);
    
    return outHeight*outWidth*3/2;
    
#endif
#else

	int YPlaneLength = inHeight*inWidth;
	int UPlaneLength = YPlaneLength >> 2;
	int indx = 0;
	outHeight = inHeight - startYDiff - endYDiff;
	outWidth = inWidth - startXDiff - endXDiff;

	for (int i = startYDiff; i<(inHeight - endYDiff); i++)
	{
		for (int j = startXDiff; j<(inWidth - endXDiff); j++)
		{
			outputData[indx++] = pData[i*inWidth + j];
		}
	}

	byte *p = pData + YPlaneLength;
	byte *q = pData + YPlaneLength + UPlaneLength;

	int uIndex = indx;
	int vIndex = indx + (outHeight * outWidth) / 4;

	int halfH = inHeight >> 1, halfW = inWidth >> 1;

	for (int i = startYDiff / 2; i<(halfH - endYDiff / 2); i++)
	{
		for (int j = startXDiff / 2; j<(halfW - endXDiff / 2); j++)
		{
			outputData[uIndex] = p[i*halfW + j];
			outputData[vIndex] = q[i*halfW + j];
			uIndex++;
			vIndex++;
		}
	}

	//printf("Now, First Block, H:W -->%d,%d  Indx = %d, uIndex = %d, vIndex = %d\n", outHeight, outWidth, indx, uIndex, vIndex);

	return outHeight*outWidth * 3 / 2;

#endif
    
}

int CColorConverter::Crop_YUVNV12_YUVNV21(unsigned char* pData, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* outputData, int &outHeight, int &outWidth)
{
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
    int YPlaneLength = inHeight*inWidth;
    int UPlaneLength = YPlaneLength >> 2;
    int indx = 0;
    
    for(int i=startYDiff; i<(inHeight-endYDiff); i++)
    {
        /*
        for(int j=startXDiff; j<(inWidth-endXDiff); j++)
        {
            outputData[indx++] = pData[i*inWidth + j];
        }
         */
        
        memcpy(outputData+indx, pData+(i*inWidth+startXDiff), (inWidth-endXDiff-startXDiff));
        indx+=(inWidth-endXDiff-startXDiff);
    }
    
    
    byte *p = pData + YPlaneLength;
    int uIndex = indx;
    int vIndex = indx + 1;
    
    
    int halfH = inHeight>>1, halfW = inWidth>>1;
    
    for(int i=startYDiff/2; i<(halfH-endYDiff/2); i++)
    {
        /*
        for(int j=startXDiff; j<(inWidth-endXDiff); j+=2)
        {
            outputData[uIndex] = p[i*inWidth + j];
            outputData[vIndex] = p[i*inWidth + j + 1];
            uIndex+=2;
            vIndex+=2;
        }
        */
        
        memcpy(outputData+indx, p+(i*inWidth + startXDiff), inWidth-endXDiff-startXDiff);
        indx+=(inWidth-endXDiff-startXDiff);
    }
    
    outHeight = inHeight - startYDiff - endYDiff;
    outWidth = inWidth - startXDiff - endXDiff;
    //printf("Now, First Block, H:W -->%d,%d  Indx = %d, uIndex = %d, vIndex = %d\n", outHeight, outWidth, indx, uIndex, vIndex);
    return outHeight*outWidth*3/2;
    
}

    
    int CColorConverter::RotateI420(byte *pInput, int inHeight, int inWidth, byte *pOutput, int &outHeight, int &outWidth, int rotationParameter)
    {
        int iLen = inHeight * inWidth * 3 / 2;
        int indx = 0;
        
        if(rotationParameter == 1)
        {
            for(int j=0;j<inWidth;j++)
            {
                for(int i=inHeight-1; i>=0;i--)
                {
                    pOutput[indx++] = pInput[i*inWidth + j];
                }
            }
            
            int halfW = inWidth>>1;
            int halfH = inHeight>>1;
            
            byte *Udata = pInput + indx;
            byte *VData = Udata + (halfH * halfW);
            
            int uIndex = indx;
            int vIndex = indx + (halfH * halfW);
            
            //printf("indx = %d, uIndex = %d, vIndex = %d\n", indx, uIndex, vIndex);
            
            
            for(int j=0;j<halfW;j++)
            {
                for(int i=halfH-1; i>=0;i--)
                {
                    pOutput[uIndex++] = Udata[i*halfW + j];
                    pOutput[vIndex++] = VData[i*halfW + j];
                }
            }
            
            outHeight = inWidth;
            outWidth = inHeight;
        }
        else if(rotationParameter == 3)
        {
            for(int j=inWidth-1;j>=0;j--)
            {
                for(int i=0; i<inHeight;i++)
                {
                    pOutput[indx++] = pInput[i*inWidth + j];
                }
            }
            
            int halfW = inWidth>>1;
            int halfH = inHeight>>1;
            
            byte *Udata = pInput + indx;
            byte *VData = Udata + (halfH * halfW);
            
            int uIndex = indx;
            int vIndex = indx + (halfH * halfW);
            
            //printf("indx = %d, uIndex = %d, vIndex = %d\n", indx, uIndex, vIndex);
            
            
            for(int j=halfW-1;j>=0;j--)
            {
                for(int i=0; i<halfH;i++)
                {
                    pOutput[uIndex++] = Udata[i*halfW + j];
                    pOutput[vIndex++] = VData[i*halfW + j];
                }
            }
            
            outHeight = inWidth;
            outWidth = inHeight;
        }
        else if(rotationParameter == 2)
        {
            for(int i=inHeight-1; i>=0;i--)
            {
                for(int j=0;j<inWidth;j++)
                {
                    pOutput[indx++] = pInput[i*inWidth + j];
                }
            }
            
            int halfW = inWidth>>1;
            int halfH = inHeight>>1;
            
            byte *Udata = pInput + indx;
            byte *VData = Udata + (halfH * halfW);
            
            int uIndex = indx;
            int vIndex = indx + (halfH * halfW);
            
            //printf("indx = %d, uIndex = %d, vIndex = %d\n", indx, uIndex, vIndex);
            
            
            for(int i=halfH-1; i>=0;i--)
            {
                for(int j=0;j<halfW;j++)
                {
                    pOutput[uIndex++] = Udata[i*halfW + j];
                    pOutput[vIndex++] = VData[i*halfW + j];
                }
            } 
            
            outHeight = inHeight;
            outWidth = inWidth;
        }
        else
        {
            memcpy(pOutput, pInput, iLen);
            outHeight = inHeight;
            outWidth = inWidth;
        }
        
        return iLen;
    }
    


int CColorConverter::GetSmallFrameWidth()
{
    ColorConverterLocker lock(*m_pColorConverterMutex);
    
    return m_iSmallFrameWidth;
}
int CColorConverter::GetSmallFrameHeight()
{
    ColorConverterLocker lock(*m_pColorConverterMutex);
    
    return m_iSmallFrameHeight;
}

int CColorConverter::GetScreenHeight()
{
    ColorConverterLocker lock(*m_pColorConverterMutex);
    return m_iDeviceHeight;
}

int CColorConverter::GetScreenWidth()
{
    ColorConverterLocker lock(*m_pColorConverterMutex);
    return m_iDeviceWidth;
}

void CColorConverter::ClearSmallScreen()
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	m_bMergingSmallFrameEnabled = false;
}

bool CColorConverter::GetSmallFrameStatus()
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	return m_bMergingSmallFrameEnabled;
}

void CColorConverter::GetSmallFrame(unsigned char *pSmallFrame)
{
	ColorConverterLocker lock(*m_pColorConverterMutex);

	memcpy(pSmallFrame, m_pSmallFrame, m_iSmallFrameSize);
}

} //namespace MediaSDK
