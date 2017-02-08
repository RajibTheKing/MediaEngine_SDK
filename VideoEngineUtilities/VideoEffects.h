
#ifndef VIDEO_EFFECTS_H
#define VIDEO_EFFECTS_H

#include <string>

class CAverageCalculator;

class CVideoEffects
{

public:

	CVideoEffects();
    ~CVideoEffects();
    
    int NegetiveColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth);

private:
    
    CAverageCalculator *m_pAverageCalculator;

};

#endif //end of VIDEO_EFFECTS_H
