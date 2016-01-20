
#include "ColorConverter.h"
#include "../VideoEngineController/LogPrinter.h"

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
	CLogPrinter::Write(CLogPrinter::INFO, "CColorConverter::CColorConverter");

	for (int i = 0; i < 481; i++)
		for (int j = 0; j < 641; j++)
		{
			m_Multiplication[i][j] = i*j;
		}

	CLogPrinter::Write(CLogPrinter::DEBUGS, "CColorConverter::CColorConverter Prepared");
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