
#include "ColorConverter.h"
#include "../VideoEngineController/LogPrinter.h"


#if defined(__ANDROID__)
typedef unsigned char byte;
#endif

double dconversionmatrix[3][3] = { 0.299, 0.587, 0.114,
								   -0.14317, -0.28886, 0.436,
								   0.615, -0.51499, -0.10001 };

double dDecodematrix[3][3] = { 1, 0, 1.13983,
							   1, -0.39465, -0.58060,
							   1, 2.03211, 0 };


CColorConverter::CColorConverter(int iVideoHeight, int iVideoWidth) :

m_iVideoHeight(iVideoHeight),
m_iVideoWidth(iVideoWidth),
m_YPlaneLength(m_iVideoHeight*m_iVideoWidth),
m_VPlaneLength(m_YPlaneLength >> 2),
m_UVPlaneMidPoint(m_YPlaneLength + m_VPlaneLength),
m_UVPlaneEnd(m_UVPlaneMidPoint + m_VPlaneLength)

{
	CLogPrinter_Write(CLogPrinter::INFO, "CColorConverter::CColorConverter");

	for (int i = 0; i < 481; i++)
		for (int j = 0; j < 641; j++)
		{
			m_Multiplication[i][j] = i*j;
		}

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CColorConverter::CColorConverter Prepared");
}

CColorConverter::~CColorConverter()
{

}

int CColorConverter::ConvertI420ToNV21(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
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
}

int CColorConverter::ConvertNV21ToI420(unsigned char *convertingData)
{
	int i, j, k;

	for (i = m_YPlaneLength, j = 0, k = i; i < m_UVPlaneEnd; i += 2, j++, k++)
	{
		m_pVPlane[j] = convertingData[i];
		convertingData[k] = convertingData[i + 1];
	}

	memcpy(convertingData + m_UVPlaneMidPoint, m_pVPlane, m_VPlaneLength);

	return m_UVPlaneEnd;
}

int CColorConverter::ConvertYUY2ToI420( unsigned char * input, unsigned char * output )
{
	int pixels = m_YPlaneLength;
	int macropixels = pixels >> 1;

	long mpx_per_row = m_iVideoWidth >> 1;

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

	return m_YPlaneLength* 3 / 2;
}

int CColorConverter::ConvertI420ToNV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
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
}

int CColorConverter::ConvertI420ToYV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	int i, j, k;

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

int CColorConverter::ConvertNV12ToI420(unsigned char *convertingData)
{
	int i, j, k;

	for (i = m_YPlaneLength, j = 0, k = i; i < m_UVPlaneEnd; i += 2, j++, k++)
	{
		m_pVPlane[j] = convertingData[i + 1];
		convertingData[k] = convertingData[i];
	}

	memcpy(convertingData + m_UVPlaneMidPoint, m_pVPlane, m_VPlaneLength);

	return m_UVPlaneEnd;
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

void CColorConverter::mirrorRotateAndConvertNV21ToI420(unsigned char *m_pFrame, unsigned char *pData)
{
	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	int i = 0;

	for (int x = iWidth - 1; x >-1; --x)
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

	for (int x = halfWidth - 1; x>-1; --x)
		for (int y = halfHeight - 1; y > -1; --y)
		{
			int ind = ( m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind];
			pData[i++] = m_pFrame[dimention + ind + 1];
		}
}

void CColorConverter::mirrorRotateAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData)
{
	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	int i = 0;

	for (int x = iWidth - 1; x >-1; --x)
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

	for (int x = halfWidth - 1; x>-1; --x)
		for (int y = halfHeight - 1; y > -1; --y)
		{
			int ind = (m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind + 1];
			pData[i++] = m_pFrame[dimention + ind];
		}
}

void CColorConverter::mirrorRotateAndConvertNV21ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData)
{
	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

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
			int ind = ( m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind];
			pData[i++] = m_pFrame[dimention + ind + 1];
		}
}

void CColorConverter::mirrorRotateAndConvertNV12ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData)
{
	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

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
