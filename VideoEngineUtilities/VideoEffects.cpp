
#include "VideoEffects.h"


CVideoEffects::CVideoEffects()
{
    //Do Nothing

}


CVideoEffects::~CVideoEffects()
{
    //Do Nothing
}

int CVideoEffects::NegetiveColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth)
{
    for(int i=0; i<inHeight; i++)
    {
        for(int j=0; j<inWidth; j++)
        {
            pConvertingData[i*inWidth + j] = 255 - pConvertingData[i*inWidth + j];
        }
    }
    
    return inHeight * inWidth * 3 / 2;
}

