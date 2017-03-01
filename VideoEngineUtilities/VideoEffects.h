
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
    int BlackAndWhiteColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth);
    int SapiaColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth);
    int WarmColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth);
    int TintColorBlueEffect(unsigned char *pConvertingData, int inHeight, int inWidth);
    int TintColorPinkEffect(unsigned char *pConvertingData, int inHeight, int inWidth);
	void SaturationChangeEffect(unsigned char *pConvertingData, int inHeight, int inWidth, double scale);
	void ContrastChangeEffect(unsigned char *pConvertingData, int inHeight, int inWidth, double contrast);

private:
    
    CAverageCalculator *m_pAverageCalculator;

};

#endif //end of VIDEO_EFFECTS_H
