
#include "VideoBeautificationer.h"
#include <cmath>
#include "../VideoEngineController/LogPrinter.h"


#define NV21 21
#define NV12 12

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

int m_sigma = 100;

#else

int m_sigma = 100;

#endif

int m_radius = 5;

int m_rr = (m_radius << 1) + 1;
double m_pixels = m_rr * m_rr;

#define getMin(a,b) a<b?a:b
#define getMax(a,b) a>b?a:b

CVideoBeautificationer::CVideoBeautificationer(int iVideoHeight, int iVideoWidth) :

m_nPreviousAddValueForBrightening(0),
m_nBrightnessPrecision(0),
m_EffectValue(10)
{
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;

	GenerateUVIndex(m_nVideoHeight, m_nVideoWidth, NV12);

	string str = "";

	for (int y = 1; y <= 255; y++)
	{
		double gray = y;
		double sqrt_value = sqrt(gray);
		gray = gray / (0.89686516089772L + 0.003202159061032L*gray - 0.044292372843353L*sqrt_value);
		gray = gray<256.0L ? gray : 255.0L;

		//#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		//modifYUV[y] = getMax(getMin(((unsigned char)1.1643*(gray - 24)), 255),0);
        
        modifYUV[y] = getMin(((unsigned char)1.1643*(gray - 24)), 255);
        modifYUV[y] = getMax(modifYUV[y], 0);
        
		//#else

		//       modifYUV[y] = gray;

		//#endif

	}


	boxesForGauss(1, 3);

	for (int i = 0; i < 256; i++)
	{
		m_square[i] = i*i;
	}

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 2300; j++)
		{
			m_precSharpness[i][j] = min(255., max(0., i + (10. * i - j)/8.));
		}
	}

}

CVideoBeautificationer::~CVideoBeautificationer()
{

}


void CVideoBeautificationer::SetHeightWidth(int iVideoHeight, int iVideoWidth)
{
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;

	GenerateUVIndex(m_nVideoHeight, m_nVideoWidth, NV12);
}

void CVideoBeautificationer::SetDeviceHeightWidth(int iVideoHeight, int iVideoWidth)
{
	m_iDeviceHeight = iVideoHeight;
	m_iDeviceWidth = iVideoWidth;
}

void CVideoBeautificationer::GenerateUVIndex(int iVideoHeight, int iVideoWidth, int dataFormat)
{
	//LOGE("fahad -->> ----------------- iHeight = %d, iWdith = %d", iVideoHeight, iVideoWidth);
	int iHeight = iVideoHeight;
	int iWidth = iVideoWidth;
	int yLength = iHeight * iWidth;
	int uLength = yLength / 2;

	int yConIndex = 0;
	int vIndex;
	int uIndex;
	vIndex = yLength + (yLength / 4);
	uIndex = yLength;


	//LOGE("fahad -->> ----------------- iHeight = %d, iWdith = %d , vIndex = %d, uIndex = %d, yLength = %d", iVideoHeight, iVideoWidth, vIndex, uIndex, yLength);
	int heightIndex = 1;
	for (int i = 0;;)
	{
		if (i == iWidth*heightIndex) {
			i += iWidth;
			heightIndex += 2;
		}
		if (i >= yLength) break;
		yConIndex = i;



		m_pUIndex[yConIndex] = uIndex;
		m_pUIndex[yConIndex + 1] = uIndex;
		m_pUIndex[yConIndex + iWidth] = uIndex;
		m_pUIndex[yConIndex + iWidth + 1] = uIndex;

		m_pVIndex[yConIndex] = vIndex;
		m_pVIndex[yConIndex + 1] = vIndex;
		m_pVIndex[yConIndex + iWidth] = vIndex;
		m_pVIndex[yConIndex + iWidth + 1] = vIndex;

		//LOGE("fahad -->> ----------------- iHeight = %d, iWdith = %d , vIndex = %d, uIndex = %d, yLength = %d", iVideoHeight, iVideoWidth, vIndex, uIndex, yLength);

		uIndex += 1;
		vIndex += 1;
		i += 2;
	}
}

void CVideoBeautificationer::MakeFrameBlur(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{

}

void CVideoBeautificationer::MakeFrameBlurAndStore(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	GaussianBlur_4thApproach(convertingData, m_pBluredImage, iVideoHeight, iVideoWidth, 2);
	//memcpy(convertingData,  m_pBluredImage, iVideoHeight*iVideoWidth );
}

void CVideoBeautificationer::MakeFrameBeautiful(unsigned char *pixel)
{
	int iTotLen = m_nVideoWidth * m_nVideoHeight;
	int iLen = m_nVideoWidth * m_nVideoHeight;//(int)(modifData.length / 1.5);
	int totalYValue = 0;
	for (int i = 0; i<iLen; i++)
	{
		totalYValue += pixel[i];
		MakePixelBright(&pixel[i]);
	}



	int m_AverageValue = totalYValue / iLen;

	SetBrighteningValue(m_AverageValue, 10/*int brightnessPrecision*/);

	//IncreaseTemperatureOfFrame(pixel, m_nVideoHeight, m_nVideoWidth, 4);

}

bool CVideoBeautificationer::IsSkinPixel(unsigned char YPixel, unsigned char UPixel, unsigned char VPixel)
{
	if ((UPixel > 94 && UPixel < 126) && (VPixel > 134 && VPixel < 176))
	{
		return true;
	}

	return false;
}

void CVideoBeautificationer::StartBrightening(int iVideoHeight, int iVideoWidth, int nPrecision)
{
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;
	m_nPreviousAddValueForBrightening = 0;
}


void CVideoBeautificationer::SetBrighteningValue(int m_AverageValue, int brightnessPrecision)
{
	if (m_AverageValue < 10)
	{
		m_nThresholdValue = 60;
	}
	else if (m_AverageValue < 15)
	{
		m_nThresholdValue = 65;
	}
	else if (m_AverageValue < 20)
	{
		m_nThresholdValue = 70;
	}
	else if (m_AverageValue < 30)
	{
		m_nThresholdValue = 85;
	}
	else if (m_AverageValue < 40)
	{
		m_nThresholdValue = 90;
	}
	else if (m_AverageValue < 50)
	{
		m_nThresholdValue = 95;
	}
	else if (m_AverageValue < 60)
	{
		m_nThresholdValue = 100;
	}
	else if (m_AverageValue < 70)
	{
		m_nThresholdValue = 110;
	}
	else if (m_AverageValue < 80)
	{
		m_nThresholdValue = 115;
	}
	else{
		m_nThresholdValue = 115;
	}

	m_nPreviousAddValueForBrightening = (m_nThresholdValue - m_AverageValue);
	m_nPreviousAddValueForBrightening = m_nPreviousAddValueForBrightening >> 1;
	if (m_nPreviousAddValueForBrightening < 0)
		m_nPreviousAddValueForBrightening = 0;

	m_nBrightnessPrecision = brightnessPrecision;
}

void CVideoBeautificationer::MakePixelBright(unsigned char *pixel)
{
	int iPixelValue = *pixel + m_nPreviousAddValueForBrightening + m_nBrightnessPrecision;
	*pixel = getMin(iPixelValue, 255);
}

void CVideoBeautificationer::MakePixelBrightNew(unsigned char *pixel)
{
	*pixel = modifYUV[*pixel];//& 0xFF;
}

void CVideoBeautificationer::SetTemperatureThreshold(int nThreshold)
{

}

void CVideoBeautificationer::IncreaseTemperatureOfPixel(unsigned char *pixel)
{

}

void CVideoBeautificationer::StopBrightening()
{

}

void CVideoBeautificationer::SetBrightnessPrecision(int nPrecision)
{
	m_nBrightnessPrecision = nPrecision;
}

void CVideoBeautificationer::SetBlurScale(int nScale)
{
	m_nBlurScale = nScale;
}

void CVideoBeautificationer::MakeFrameBright(unsigned char *convertingData, int iVideoHeight, int iVideoWidth, int nPrecision)
{

}

void CVideoBeautificationer::IncreaseTemperatureOfFrame(unsigned char *convertingData, int iVideoHeight, int iVideoWidth,
	unsigned char nThreshold)
{
	int startUIndex = iVideoHeight * iVideoWidth;
	int uLen = startUIndex + (startUIndex / 4);
	int vLen = startUIndex + (startUIndex / 2);
	for (int i = startUIndex; i <uLen; i++)
	{
		convertingData[i] = getMax(convertingData[i] - nThreshold, 0);
	}

	for (int i = uLen; i <vLen; i++)
	{
		convertingData[i] = getMin(convertingData[i] + nThreshold, 255);
	}
}

void CVideoBeautificationer::boxesForGauss(float sigma, int n)  // standard deviation, number of boxes
{
	float wIdeal = (float)sqrt((12 * sigma*sigma / n) + 1);  // Ideal averaging filter width
	int wl = (int)floor(wIdeal);
	if (wl % 2 == 0) wl--;
	int wu = wl + 2;

	float mIdeal = (12 * sigma*sigma - n*wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
	int m = (int)floor(mIdeal + 0.5);

	// var sigmaActual = sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );

	for (int i = 0; i<n; i++)
	{
		m_Sizes[i] = i<m ? wl : wu;
	}

	return;
}

void CVideoBeautificationer::GaussianBlur_4thApproach(unsigned char *scl, unsigned char *tcl, int h, int w, float r)
{
	boxBlur_4(scl, tcl, h, w, (m_Sizes[2] - 1) / 2);

}


void CVideoBeautificationer::boxBlur_4(unsigned char *scl, unsigned char *tcl, int h, int w, int r)
{
	boxBlurH_4(scl, tcl, h, w, r);
}

/*void CVideoBeautificationer::boxBlurH_4 (unsigned char *scl, unsigned char *tcl, int h, int w, int r)
{
int iarr = (r+r+1);
for(int i=0; i<h; i++)
{
int ti = i*w, li = ti, ri = ti+r;
int fv = scl[ti], lv = scl[ti+w-1], val = (r+1)*fv;

for(int j=0; j<r; j++) val += scl[ti+j] ;
for(int j=0  ; j<=r ; j++) {
val += scl[ri++]  - fv;
tcl[ti++] = ( unsigned char)(val/iarr) & 0xFF;
}
for(int j=r+1; j<w-r; j++) {
val += scl[ri++]  - scl[li++] ;
tcl[ti++] = (unsigned char)(val/iarr) & 0xFF;;
}
//for(int j=w-r; j<w  ; j++) { val += lv        - scl[li++];   tcl[ti++] = (unsigned char)floor(val*iarr + 0.5); }
}
}*/

void CVideoBeautificationer::boxBlurH_4(unsigned char *scl, unsigned char *tcl, int h, int w, int r)
{
	int iarr = (r + r + 1);
	for (int i = 0; i<h; i++)
	{
		int ti = i*w, li = ti, ri = ti + r;
		int fv = (scl[ti] & 0xFF), lv = (scl[ti + w - 1] & 0xFF), val = (r + 1)*fv;

		for (int j = 0; j<r; j++) val += (scl[ti + j] & 0xFF);
		for (int j = 0; j <= r; j++) { val += (scl[ri++] & 0xFF) - fv;   tcl[ti++] = (unsigned char)((unsigned char)(val / iarr) & 0xFF); }
		for (int j = r + 1; j<w - r; j++) {
			val += (scl[ri++] & 0xFF) - (scl[li++] & 0xFF);   tcl[ti++] = (unsigned char)((unsigned char)(val / iarr) & 0xFF);
		}
		for (int j = w - r; j<w; j++) {
			val += (scl[ri++] & 0xFF) - (scl[li++] & 0xFF);   tcl[ti++] = (unsigned char)((unsigned char)(val / iarr) & 0xFF);
		}//(unsigned char)floor(val*iarr + 0.5); }
	}
	//return tcl;
}

pair<int, int> CVideoBeautificationer::BeautificationFilter(unsigned char *pBlurConvertingData, int iLen, int iHeight, int iWidth, int *effectParam)
{
	/*if (effectParam[0] != 0)m_sigma = effectParam[0];
	if (effectParam[1] != 0)m_radius = effectParam[1];
	if (effectParam[2] != 0)m_EffectValue = effectParam[2];*/

	long long startSharpingTime = m_Tools.CurrentTimestamp();
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    //Do nothing
    //Not Needed Yet...
#else

	for (int i = 0; i <= iHeight; i++) 
	{
		m_mean[i][0] = 0;
	}

	memset(m_mean, iWidth, 0);


	for (int i = 1, iw = 0; i <= iHeight; i++, iw += iWidth)
	{
		int tmp = 0;

		for (int j = 1; j <= iWidth; j++)
		{
			tmp += pBlurConvertingData[i * iWidth + j - 1];
			m_mean[i][j] = tmp + m_mean[i - 1][j];

			if (i > 2 && j > 2) 
			{
				int indx = m_mean[i][j] - m_mean[i - 3][j] - m_mean[i][j - 3] + m_mean[i - 3][j - 3];

				pBlurConvertingData[iw + j - 2] = m_precSharpness[pBlurConvertingData[iw + j - 2]][indx];
			}
		}
	}

#endif

	long long endSharpingTime = m_Tools.CurrentTimestamp();

	for (int i = 0; i <= iHeight; i++) 
	{
		m_mean[i][0] = 0;
		m_variance[i][0] = 0;
	}

	memset(m_mean, iWidth, 0);
	memset(m_variance, iWidth, 0);

	int cur_pixel, tmp, tmp2;

#if defined(__ANDROID__)
	int totalYValue = 0;
	int yLen = iWidth * iHeight;
#endif

	for (int i = 1, iw = 0; i <= iHeight; i++, iw += iWidth)
	{
		tmp = 0, tmp2 = 0;
		m_mean[i][0] = 0;

		for (int j = 1; j <= iWidth; j++)
		{

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

			MakePixelBrightNew(&pBlurConvertingData[iw + j - 1]);

#elif defined(__ANDROID__)

			totalYValue += pBlurConvertingData[iw + j - 1];
			MakePixelBright(&pBlurConvertingData[iw + j - 1]);

#endif
			cur_pixel = pBlurConvertingData[iw + j - 1];

			tmp += cur_pixel;
			m_mean[i][j] = tmp + m_mean[i - 1][j];

			tmp2 += m_square[cur_pixel];
			m_variance[i][j] = tmp2 + m_variance[i - 1][j];

			//pBlurConvertingData[m_pUIndex[iw + j - 1]] -= 1;
			//pBlurConvertingData[m_pVIndex[iw + j - 1]] += 1;
		}
	}

	int niHeight = iHeight - m_rr;
	int niWidth = iWidth - m_rr;
	int iw = m_radius * iWidth + m_radius;

	//m_sigma = 255 - m_mean[iHeight][iWidth] / (iHeight * iWidth);

	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "sigma value " + m_Tools.getText(m_sigma));

	for (int hl = 0, hr = m_rr; hl < niHeight; hl++, hr++)
	{
		for (int wl = 0, wr = m_rr; wl < niWidth; wl++, wr++)
		{
			int miu = m_mean[hl][wl] + m_mean[hr][wr] - m_mean[hl][wr] - m_mean[hr][wl];
			int viu = m_variance[hl][wl] + m_variance[hr][wr] - m_variance[hl][wr] - m_variance[hr][wl];

			//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_3, "viu " + m_Tools.getText(viu) + " alter " + m_Tools.getText(miu * miu / 121));

			//LOGE("viu %d miu %d\n",viu,miu/121*miu);


			double men = miu / m_pixels;
			double var = (viu - (miu * men)) / m_pixels;

			pBlurConvertingData[iw + wl] = min(255., max(0., (m_sigma * men + var * pBlurConvertingData[iw + wl]) / (var + m_sigma)));

		}
		iw += iWidth;
	}

	long long endFilterTime = m_Tools.CurrentTimestamp();

	//LOGE("VideoBeautificcationer -->> sharpingTimeDiff = %lld, filterTimeDiff = %lld, totalTimeDiff =% lld", -(startSharpingTime - endSharpingTime), -(endSharpingTime - endFilterTime), -(startSharpingTime - endFilterTime));
	pair<int, int> result = { m_mean[iHeight][iWidth] / (iHeight*iWidth), m_variance[iHeight][iWidth] / (iHeight*iWidth) };
	return result;
}



