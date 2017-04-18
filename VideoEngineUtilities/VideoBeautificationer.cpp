
#include "VideoBeautificationer.h"
#include <cmath>
#include "../VideoEngineController/LogPrinter.h"


#define NV21 21
#define NV12 12

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

#import <sys/utsname.h> // import it in your header or implementation file.
//#import <UIKit/UIKit.h>

int m_sigma = 64;

#elif defined(__ANDROID__)

int m_sigma = 128;

#elif defined(DESKTOP_C_SHARP)

int m_sigma = 100;

#else 

int m_sigma = 35;

#endif

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

int m_sigmaDigit = 6;

#elif defined(__ANDROID__)

int m_sigmaDigit = 7;

#elif defined(DESKTOP_C_SHARP)

int m_sigmaDigit = 7;

#else 

int m_sigmaDigit = 5;

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

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

    std::string sDeviceModel = getDeviceModel();
    //printf("TheKing--> GetDeviceModel = %s\n", sDeviceModel.c_str());
	m_nIsGreaterThen5s = isGreaterThanIphone5s();
    
    //printf("TheKing--> ansGot = %d\n", ansGot);

	if (m_nIsGreaterThen5s > 0)
	{
		m_sigma = 128;
		m_sigmaDigit = 7;
	}
	else
	{
		m_sigma = 64;
		m_sigmaDigit = 6;
	}

#endif
    
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;

	GenerateUVIndex(m_nVideoHeight, m_nVideoWidth, NV12);

	string str = "";

	for (int y = 1; y <= 255; y++)
	{
		double gray = y;
		double sqrt_value = sqrt(gray);
		gray = gray / (0.89686516089772L + 0.002502159061032L*gray - 0.040292372843353L*sqrt_value);
		gray = gray<256.0L ? gray : 255.0L;

		//#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

		//modifYUV[y] = getMax(getMin(((unsigned char)1.1643*(gray - 24)), 255),0);
        
		int a = (int)getMin((1.1643*(gray - 24)), 255);
		int b = getMax(a,0);

		unsigned char c = (unsigned char)((double)y * 0.803921 + 50.0);

		modifYUV[y] = (unsigned char)c;
        
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


	for (int i = 0; i < 641; i++)
	{
		for (int j = 0; j < 641; j++)
		{
			m_Multiplication[i][j] = i*j;
		}
	}

	int firstDif = 125 - 25;
	int secondDif = 225 - 125;
	
	for (int i = 0; i < 256; i++)
	{
		m_preBrightness[i] = i;
		
		if (i >= 25 && i <= 125) 
		{
			m_preBrightness[i] += (i - 25) * 15 / firstDif;
		}
		else if (i >= 125 && i <= 225) 
		{
			m_preBrightness[i] += (225 - i) * 15 / secondDif;
		}
	}

	for (int i = 0; i < 256; i++)
		m_ucpreBrightness[i] = (unsigned char)m_preBrightness[i];

	memset(m_mean, m_nVideoHeight*m_nVideoWidth, 0);
	memset(m_variance, m_nVideoHeight*m_nVideoWidth, 0);

	luminaceHigh = 255;
}

CVideoBeautificationer::~CVideoBeautificationer()
{

}


void CVideoBeautificationer::SetHeightWidth(int iVideoHeight, int iVideoWidth)
{
	m_nVideoHeight = iVideoHeight;
	m_nVideoWidth = iVideoWidth;

	GenerateUVIndex(m_nVideoHeight, m_nVideoWidth, NV12);

	memset(m_mean, m_nVideoHeight*m_nVideoWidth, 0);
	memset(m_variance, m_nVideoHeight*m_nVideoWidth, 0);
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
	memcpy(convertingData,  m_pBluredImage, iVideoHeight*iVideoWidth );
}

void CVideoBeautificationer::MakeFrameBeautiful(unsigned char *pixel)
{
	int iTotLen = m_nVideoWidth * m_nVideoHeight;
	int iLen = m_nVideoWidth * m_nVideoHeight;//(int)(modifData.length / 1.5);
	int totalYValue = 0;

	for (int i = 0; i<iLen; i++)
	{
		if (IsSkinPixel(pixel[i], m_pUIndex[pixel[i]], m_pVIndex[pixel[i]]))
		{
			pixel[i] = m_pBluredImage[i];
		}

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
	m_nPreviousAddValueForBrightening = (m_nPreviousAddValueForBrightening >> 1);
	if (m_nPreviousAddValueForBrightening < 0)
		m_nPreviousAddValueForBrightening = 0;

	//m_nBrightnessPrecision = brightnessPrecision;
	m_nPreviousAddValueForBrightening += m_nBrightnessPrecision;
}

void CVideoBeautificationer::MakePixelBright(unsigned char *pixel)
{
	int iPixelValue = *pixel + m_nPreviousAddValueForBrightening;
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
		int ti = m_Multiplication[i][w], li = ti, ri = ti + r;
		int fv = (scl[ti]), lv = (scl[ti + w - 1]), val = m_Multiplication[(r + 1)][fv];

		for (int j = 0; j<r; j++) val += (scl[ti + j]);
		for (int j = 0; j <= r; j++) { val += (scl[ri++]) - fv;   tcl[ti++] = (unsigned char)((unsigned char)(val / iarr) & 0xFF); }
		for (int j = r + 1; j<w - r; j++) {
			val += (scl[ri++]) - (scl[li++]);   tcl[ti++] = (unsigned char)((unsigned char)(val / iarr) & 0xFF);
		}
		for (int j = w - r; j<w; j++) {
			val += (scl[ri++]) - (scl[li++] );   tcl[ti++] = (unsigned char)((unsigned char)(val / iarr) & 0xFF);
		}//(unsigned char)floor(val*iarr + 0.5); }
	}
	//return tcl;
}

pair<int, int> CVideoBeautificationer::BeautificationFilter2(unsigned char *pBlurConvertingData, int iLen, int iHeight, int iWidth, int *effectParam)
{
	/*if (effectParam[0] != 0)m_sigma = effectParam[0];
	if (effectParam[1] != 0)m_radius = effectParam[1];
	if (effectParam[2] != 0)m_EffectValue = effectParam[2];*/

	long long startSharpingTime = m_Tools.CurrentTimestamp();

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(__ANDROID__)
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

	int ll = iHeight * iWidth;
	int totalYValue = 0;

	for (int i = 0; i < ll; i++)
	{
		totalYValue += pBlurConvertingData[i];

		if (pBlurConvertingData[i] >= luminaceHigh - m_nPreviousAddValueForBrightening)
			pBlurConvertingData[i] = luminaceHigh;
		else
			pBlurConvertingData[i] += m_nPreviousAddValueForBrightening;

		//pBlurConvertingData[i] = modifYUV[pBlurConvertingData[i]];
	}

	int m_AvarageValue = totalYValue / ll;

	SetBrighteningValue(m_AvarageValue, 10);

	long long endFilterTime = m_Tools.CurrentTimestamp();

	//LOGE("VideoBeautificcationer -->> sharpingTimeDiff = %lld, filterTimeDiff = %lld, totalTimeDiff =% lld", -(startSharpingTime - endSharpingTime), -(endSharpingTime - endFilterTime), -(startSharpingTime - endFilterTime));
	pair<int, int> result = { m_mean[iHeight][iWidth] / (iHeight*iWidth), m_variance[iHeight][iWidth] / (iHeight*iWidth) };
	return result;
}

pair<int, int> CVideoBeautificationer::BeautificationFilter(unsigned char *pBlurConvertingData, int iLen, int iHeight, int iWidth, int *effectParam)
{
	/*if (effectParam[0] != 0)m_sigma = effectParam[0];
	if (effectParam[1] != 0)m_radius = effectParam[1];
	if (effectParam[2] != 0)m_EffectValue = effectParam[2];*/

	long long startSharpingTime = m_Tools.CurrentTimestamp();
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(__ANDROID__)
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
	int totalYValue = 0;
	int yLen = iWidth * iHeight;
#if defined(__ANDROID__)
	/*int totalYValue = 0;*/
	//int yLen = iWidth * iHeight;
#endif

	for (int i = 1, iw = 0; i <= iHeight; i++, iw += iWidth)
	{
		tmp = 0, tmp2 = 0;
		m_mean[i][0] = 0;

		for (int j = 1; j <= iWidth; j++)
		{

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

			//MakePixelBrightNew(&pBlurConvertingData[iw + j - 1]);

#elif defined(__ANDROID__)

			totalYValue += pBlurConvertingData[i];

			if (pBlurConvertingData[i] >= luminaceHigh - m_nPreviousAddValueForBrightening)
				pBlurConvertingData[i] = luminaceHigh;
			else
				pBlurConvertingData[i] += m_nPreviousAddValueForBrightening;

			//pBlurConvertingData[i] = modifYUV[pBlurConvertingData[i]];

#endif
			tmp += pBlurConvertingData[iw + j - 1];
			m_mean[i][j] = tmp + m_mean[i - 1][j];

			tmp2 += (pBlurConvertingData[iw + j - 1] * pBlurConvertingData[iw + j - 1]);
			m_variance[i][j] = tmp2 + m_variance[i - 1][j];

			//pBlurConvertingData[m_pUIndex[iw + j - 1]] -= 1;
			//pBlurConvertingData[m_pVIndex[iw + j - 1]] += 1;
		}
	}


	int m_AvarageValue = totalYValue/yLen;

	SetBrighteningValue(m_AvarageValue, 10);

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

bool CVideoBeautificationer::IsNotSkinPixel(unsigned char UPixel, unsigned char VPixel)
{
	return (UPixel <= 94 || UPixel >= 126 || VPixel <= 134 || VPixel >= 176);
}

pair<int, int> CVideoBeautificationer::BeautificationFilter(unsigned char *pBlurConvertingData, int iLen, int iHeight, int iWidth, int iNewHeight, int iNewWidth, int *effectParam)
{
	/*if (effectParam[0] != 0)m_sigma = effectParam[0];
	if (effectParam[1] != 0)m_radius = effectParam[1];
	if (effectParam[2] != 0)m_EffectValue = effectParam[2];*/

	int startWidth = (iWidth - iNewWidth)/2 - m_rr;
	int endWidth = iWidth - startWidth + m_rr;

	//for (int i = 0; i <= iHeight; i++) 
	//{
	//	m_mean[i][0] = 0;
	//}

	//memset(m_mean, iWidth, 0);
	/*
	for (int i = 1, iw = 0; i <= iHeight; i++, iw += iWidth)
	{
		for (int j = startWidth; j <= endWidth; j++)
		{
			m_mean[i][j] = pBlurConvertingData[iw + j - 1];
		}
	}
	*/

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	if (m_nIsGreaterThen5s > 0)

#endif

	{
		for (int i = 1, iw = 0, iw2 = -iWidth; i <= iHeight; i++, iw += iWidth, iw2 += iWidth)
		{
			for (int j = startWidth; j <= endWidth; j++)
			{
				m_mean[i][j] = pBlurConvertingData[iw + j - 1];

				if (i > 2)
				{
					//if (pBlurConvertingData[m_pUIndex[iw2 + j - 1]] < 95 || pBlurConvertingData[m_pUIndex[iw2 + j - 1]] > 125 || pBlurConvertingData[m_pVIndex[iw2 + j - 2]] < 135 || pBlurConvertingData[m_pVIndex[iw2 + j - 2]] > 175)
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

					pBlurConvertingData[iw2 + j - 1] = min(255, max(0, pBlurConvertingData[iw2 + j - 1] + (((m_mean[i - 1][j] << 2) - m_mean[i - 2][j] - m_mean[i][j] - m_mean[i - 1][j - 1] - m_mean[i - 1][j + 1]) >> 2)));
#else
					pBlurConvertingData[iw2 + j - 1] = min(255, max(0, pBlurConvertingData[iw2 + j - 1] + (((m_mean[i - 1][j] << 2) - m_mean[i - 2][j] - m_mean[i][j] - m_mean[i - 1][j - 1] - m_mean[i - 1][j + 1]) >> 3)));
#endif
				}
			}
		}
	}
	
	
	

	//for (int i = 0; i <= iHeight; i++) 
	//{
	//	m_mean[i][startWidth - 1] = 0;
	//	m_variance[i][startWidth - 1] = 0;
	//}

	//memset(m_mean, iWidth, 0);
	//memset(m_variance, iWidth, 0);

	int cur_pixel, tmp, tmp2;
	int totalYValue = 0;
	int yLen = iWidth * iHeight;

#if defined(__ANDROID__)
	/*int totalYValue = 0;*/
	//int yLen = iWidth * iHeight;
#endif

	for (int i = 1, iw = 0; i <= iHeight; i++, iw += iWidth)
	{
		tmp = 0, tmp2 = 0;
		m_mean[i][startWidth - 1] = 0;

		for (int j = startWidth; j <= endWidth; j++)
		{

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

			//MakePixelBrightNew(&pBlurConvertingData[iw + j - 1]);

#elif defined(__ANDROID__)

			//if (pBlurConvertingData[iw + j - 1] >= luminaceHigh - m_nPreviousAddValueForBrightening)
			//	pBlurConvertingData[iw + j - 1] = luminaceHigh;
			//else
			//	pBlurConvertingData[iw + j - 1] += m_nPreviousAddValueForBrightening;

			//pBlurConvertingData[iw + j - 1] = modifYUV[pBlurConvertingData[iw + j - 1]];


			pBlurConvertingData[iw + j - 1] = m_ucpreBrightness[pBlurConvertingData[iw + j - 1]];
            
#endif
            
			totalYValue += pBlurConvertingData[iw + j - 1];


			tmp += pBlurConvertingData[iw + j - 1];
			m_mean[i][j] = tmp + m_mean[i - 1][j];

			tmp2 += (pBlurConvertingData[iw + j - 1] * pBlurConvertingData[iw + j - 1]);
			m_variance[i][j] = tmp2 + m_variance[i - 1][j];

			//pBlurConvertingData[m_pUIndex[iw + j - 1]] -= 1;
			//pBlurConvertingData[m_pVIndex[iw + j - 1]] += 1;
		}
	}

	int m_AvarageValue = totalYValue/yLen;

#if defined(__ANDROID__)

	if (m_AvarageValue < 25)
	{
		m_sigma = 16;
		m_sigmaDigit = 4;
	}
	else if (m_AvarageValue < 50)
	{
		m_sigma = 32;
		m_sigmaDigit = 5;
	}
	else if (m_AvarageValue < 75)
	{
		m_sigma = 64;
		m_sigmaDigit = 6;
	}
	else
	{
		m_sigma = 128;
		m_sigmaDigit = 7;
	}

#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	if (m_AvarageValue < 50)
	{
		m_sigma = 32;
		m_sigmaDigit = 5;
	}
	else
	{
		if (m_nIsGreaterThen5s > 0)
		{
			m_sigma = 128;
			m_sigmaDigit = 7;
		}
		else
		{
			m_sigma = 64;
			m_sigmaDigit = 6;
		}	
	}

#endif

	//SetBrighteningValue(m_AvarageValue, 10);

	int niHeight = iHeight - m_rr;
	int niWidth = endWidth - m_rr;
	int iw = m_radius * iWidth + m_radius;
	double sigmaPix = m_sigma * m_pixels;

	//m_sigma = 255 - m_mean[iHeight][iWidth] / (iHeight * iWidth);

	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG_2, "sigma value " + m_Tools.getText(m_sigma));

	for (int hl = 0, hr = m_rr; hl < niHeight; hl++, hr++)
	{
		for (int wl = startWidth, wr = m_rr + startWidth; wl < niWidth; wl++, wr++)
		{
			int miu = m_mean[hl][wl] + m_mean[hr][wr] - m_mean[hl][wr] - m_mean[hr][wl];
			int viu = m_variance[hl][wl] + m_variance[hr][wr] - m_variance[hl][wr] - m_variance[hr][wl];
			
			//double men = miu / m_pixels;
			//double var = (viu - (miu * miu) / m_pixels) / m_pixels;
			//var = abs(var);

			//m_tmpPixel[iw + wl] = (m_sigma * men + var * pBlurConvertingData[iw + wl]) / (var + m_sigma);
			
			double var = viu - (miu * miu / m_pixels);

			pBlurConvertingData[iw + wl] = min(255., max(0., ((miu << m_sigmaDigit) + var * pBlurConvertingData[iw + wl]) / (var + sigmaPix)));
		}

		iw += iWidth;
	}

	long long endFilterTime = m_Tools.CurrentTimestamp();

	//LOGE("VideoBeautificcationer -->> sharpingTimeDiff = %lld, filterTimeDiff = %lld, totalTimeDiff =% lld", -(startSharpingTime - endSharpingTime), -(endSharpingTime - endFilterTime), -(startSharpingTime - endFilterTime));
	
	pair<int, int> result = { m_mean[iHeight][iWidth] / (iHeight*iWidth), m_variance[iHeight][iWidth] / (iHeight*iWidth) };

	return result;
}

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

int CVideoBeautificationer::isGreaterThanIphone5s()
{
    string sDeviceInfo = getDeviceModel();
    CLogPrinter::Log("TheKing--> Here Devicetype ");
    
    if(sDeviceInfo=="iPhone7,1" ||
       sDeviceInfo=="iPhone7,2" ||
       sDeviceInfo=="iPhone8,1" ||
       sDeviceInfo=="iPhone8,2" ||
       sDeviceInfo=="iPhone8,4" ||
       sDeviceInfo=="iPhone9,1" ||
       sDeviceInfo=="iPhone9,2" ||
       sDeviceInfo=="iPhone9,3" ||
       sDeviceInfo=="iPhone9,4")
    {
        return 1;
    }
    else if(sDeviceInfo=="iPhone1,1" ||
            sDeviceInfo=="iPhone1,2" ||
            sDeviceInfo=="iPhone2,1" ||
            sDeviceInfo=="iPhone3,1" ||
            sDeviceInfo=="iPhone3,3" ||
            sDeviceInfo=="iPhone4,1" ||
            sDeviceInfo=="iPhone5,1" ||
            sDeviceInfo=="iPhone5,2" ||
            sDeviceInfo=="iPhone5,3" ||
            sDeviceInfo=="iPhone5,4" ||
            sDeviceInfo=="iPhone6,1" ||
            sDeviceInfo=="iPhone6,2")
    {
        return 0;
    }
    else
    {
        return -1;
    }
    
}


std::string  CVideoBeautificationer::getDeviceModel()
{
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    /*
    //Simultor
    @"i386"      on 32-bit Simulator
    @"x86_64"    on 64-bit Simulator
    
    //iPhone
    @"iPhone1,1" on iPhone
    @"iPhone1,2" on iPhone 3G
    @"iPhone2,1" on iPhone 3GS
    @"iPhone3,1" on iPhone 4 (GSM)
    @"iPhone3,3" on iPhone 4 (CDMA/Verizon/Sprint)
    @"iPhone4,1" on iPhone 4S
    @"iPhone5,1" on iPhone 5 (model A1428, AT&T/Canada)
    @"iPhone5,2" on iPhone 5 (model A1429, everything else)
    @"iPhone5,3" on iPhone 5c (model A1456, A1532 | GSM)
    @"iPhone5,4" on iPhone 5c (model A1507, A1516, A1526 (China), A1529 | Global)
    @"iPhone6,1" on iPhone 5s (model A1433, A1533 | GSM)
    @"iPhone6,2" on iPhone 5s (model A1457, A1518, A1528 (China), A1530 | Global)
    @"iPhone7,1" on iPhone 6 Plus
    @"iPhone7,2" on iPhone 6
    @"iPhone8,1" on iPhone 6S
    @"iPhone8,2" on iPhone 6S Plus
    @"iPhone8,4" on iPhone SE
    @"iPhone9,1" on iPhone 7 (CDMA)
    @"iPhone9,3" on iPhone 7 (GSM)
    @"iPhone9,2" on iPhone 7 Plus (CDMA)
    @"iPhone9,4" on iPhone 7 Plus (GSM)
    
    //iPad 1
    @"iPad1,1" on iPad - Wifi (model A1219)
    @"iPad1,1" on iPad - Wifi + Cellular (model A1337)
    
    //iPad 2
    @"iPad2,1" - Wifi (model A1395)
    @"iPad2,2" - GSM (model A1396)
    @"iPad2,3" - 3G (model A1397)
    @"iPad2,4" - Wifi (model A1395)
    
    // iPad Mini
    @"iPad2,5" - Wifi (model A1432)
    @"iPad2,6" - Wifi + Cellular (model  A1454)
    @"iPad2,7" - Wifi + Cellular (model  A1455)
    
    //iPad 3
    @"iPad3,1" - Wifi (model A1416)
    @"iPad3,2" - Wifi + Cellular (model  A1403)
    @"iPad3,3" - Wifi + Cellular (model  A1430)
    
    //iPad 4
    @"iPad3,4" - Wifi (model A1458)
    @"iPad3,5" - Wifi + Cellular (model  A1459)
    @"iPad3,6" - Wifi + Cellular (model  A1460)
    
    //iPad AIR
    @"iPad4,1" - Wifi (model A1474)
    @"iPad4,2" - Wifi + Cellular (model A1475)
    @"iPad4,3" - Wifi + Cellular (model A1476)
    
    // iPad Mini 2
    @"iPad4,4" - Wifi (model A1489)
    @"iPad4,5" - Wifi + Cellular (model A1490)
    @"iPad4,6" - Wifi + Cellular (model A1491)
    
    // iPad Mini 3
    @"iPad4,7" - Wifi (model A1599)
    @"iPad4,8" - Wifi + Cellular (model A1600)
    @"iPad4,9" - Wifi + Cellular (model A1601)
    
    // iPad Mini 4
    @"iPad5,1" - Wifi (model A1538)
    @"iPad5,2" - Wifi + Cellular (model A1550)
    
    //iPad AIR 2
    @"iPad5,3" - Wifi (model A1566)
    @"iPad5,4" - Wifi + Cellular (model A1567)
    
    // iPad PRO 12.9"
    @"iPad6,3" - Wifi (model A1673)
    @"iPad6,4" - Wifi + Cellular (model A1674)
    @"iPad6,4" - Wifi + Cellular (model A1675)
    
    //iPad PRO 9.7"
    @"iPad6,7" - Wifi (model A1584)
    @"iPad6,8" - Wifi + Cellular (model A1652)
    
    //iPod Touch
    @"iPod1,1"   on iPod Touch
    @"iPod2,1"   on iPod Touch Second Generation
    @"iPod3,1"   on iPod Touch Third Generation
    @"iPod4,1"   on iPod Touch Fourth Generation
    @"iPod7,1"   on iPod Touch 6th Generation
    */
    
    struct utsname systemInfo;
    uname(&systemInfo);
    char *p = systemInfo.machine;
    
    //NSString *nsDeviceModel = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    
    std::string ans(p);
    
    return ans;
#else
    return "";
#endif
}

#endif
