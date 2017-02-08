
#ifndef VIDEO_EFFECTS_H
#define VIDEO_EFFECTS_H

#include <string>

class CVideoEffects
{

public:

	CVideoEffects();
    ~CVideoEffects();
    
    int NegetiveColorEffect(unsigned char *pConvertingData, int inHeight, int inWidth);

private:

};

#endif //end of VIDEO_EFFECTS_H
