
#include "VideoEffects.h"
#include "../VideoEngineController/AverageCalculator.h"
#include "../VideoEngineController/Tools.h"


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
    int len = inHeight * inWidth;
    
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

