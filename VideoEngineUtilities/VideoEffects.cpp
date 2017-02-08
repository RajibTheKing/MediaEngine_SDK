
#include "VideoEffects.h"
#include "../VideoEngineController/AverageCalculator.h"
#include "../VideoEngineController/Tools.h"

#define getMax(a,b) a>b?a:b
#define getMin(a,b) a<b?a:b

CVideoEffects::CVideoEffects()
{
    m_pAverageCalculator = new CAverageCalculator();
    //Do Nothing

}


CVideoEffects::~CVideoEffects()
{
    //Do Nothing
    delete m_pAverageCalculator;
}

int CVideoEffects::NegetiveColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth)
{
    int len = inHeight * inWidth * 3 / 2;
    
    long long startTime = Tools::CurrentTimestamp();
    unsigned char a = 255;
    
    for(int i=0; i<len; i++)
    {
        pConvertingData[i] = a - pConvertingData[i];
    }
    
    int timeDiff = Tools::CurrentTimestamp() - startTime;
    
    printf("TheKing--> NegativeColorEffect timeDiff = %d\n", timeDiff);
    
    return inHeight * inWidth * 3 / 2;
}

int CVideoEffects::BlackAndWhiteColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth)
{
    int YPlaneLength = inHeight * inWidth;
    int UVPlaneLength = YPlaneLength>>1;
    
    long long startTime = Tools::CurrentTimestamp();
    unsigned char a = 128;
    memset(pConvertingData + YPlaneLength,a,UVPlaneLength);
    int timeDiff = Tools::CurrentTimestamp() - startTime;
    
    printf("TheKing--> BlackAndWhiteColorEffect timeDiff = %d\n", timeDiff);
    
    return inHeight * inWidth * 3 / 2;
}

int CVideoEffects::SapiaColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth)
{
    int YPlaneLength = inHeight * inWidth;
    int UPlaneLength = YPlaneLength>>2;
    int VPlaneLength = UPlaneLength;
    int UVPlaneLength = YPlaneLength>>1;
    
    long long startTime = Tools::CurrentTimestamp();
    unsigned char a = 120;
    unsigned char b = 110;
    
    memset(pConvertingData + YPlaneLength,a,UPlaneLength);
    memset(pConvertingData + YPlaneLength + UPlaneLength, b, VPlaneLength);
    
    int timeDiff = Tools::CurrentTimestamp() - startTime;
    
    printf("TheKing--> SapiaColorEffect timeDiff = %d\n", timeDiff);
    
    return inHeight * inWidth * 3 / 2;
}
