//
//  NeonAssemblyWrapper.cpp
//  TestCamera 
//
//  Created by Rajib Chandra Das on 7/26/17.
//
//

#include "NeonAssemblyWrapper.h"
#ifdef HAVE_NEON
#include <arm_neon.h>
#endif

#include <string.h>
#include <stdio.h>



NeonAssemblyWrapper::NeonAssemblyWrapper()
{
    param = new unsigned int[10];
}

NeonAssemblyWrapper::~NeonAssemblyWrapper()
{
    delete[] param;
}


void NeonAssemblyWrapper::convert_nv12_to_i420_assembly(unsigned char*  src, int iHeight, int iWidth)
{
#if defined(HAVE_NEON)
    convert_nv12_to_i420_arm_neon(src, m_pTempArray, iHeight, iWidth);
    memcpy(src, m_pTempArray, iHeight * iWidth * 3 / 2);
#elif defined(HAVE_NEON_AARCH64)
    convert_nv12_to_i420_arm_neon_aarch64(src, m_pTempArray, iHeight, iWidth);
    memcpy(src, m_pTempArray, iHeight * iWidth * 3 / 2);
#endif
    
}
void NeonAssemblyWrapper::convert_i420_to_nv12_assembly(unsigned char*  src, int iHeight, int iWidth)
{
#if defined(HAVE_NEON)
    convert_i420_to_nv12_arm_neon(src, m_pTempArray, iHeight, iWidth);
    memcpy(src, m_pTempArray, iHeight * iWidth * 3 / 2);
#elif defined(HAVE_NEON_AARCH64)
    convert_i420_to_nv12_arm_neon_aarch64(src, m_pTempArray, iHeight, iWidth);
    memcpy(src, m_pTempArray, iHeight * iWidth * 3 / 2);
#endif
}

void NeonAssemblyWrapper::convert_i420_to_nv21_assembly(unsigned char*  src, int iHeight, int iWidth)
{
#if defined(HAVE_NEON)
    convert_i420_to_nv21_arm_neon(src, m_pTempArray, iHeight, iWidth);
    memcpy(src, m_pTempArray, iHeight * iWidth * 3 / 2);
#endif
}


void NeonAssemblyWrapper::Crop_yuv420_assembly(unsigned char* src, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* dst, int &outHeight, int &outWidth)
{

    outHeight = inHeight - startYDiff - endYDiff;
    outWidth = inWidth - startXDiff - endXDiff;
    param[0] = inHeight;
    param[1] = inWidth;
    param[2] = startXDiff;
    param[3] = endXDiff;
    param[4] = startYDiff;
    param[5] = endYDiff;
    param[6] = outHeight;
    param[7] = outWidth;
    
#if defined(HAVE_NEON)
    crop_yuv420_arm_neon(src, dst, param);
#elif defined(HAVE_NEON_AARCH64)
    crop_yuv420_arm_neon_aarch64(src, dst, param);
#endif
    
    //ARM_NEON: 2017-08-26 19:45:28.245923 MediaEngine[442:110984] TimeElapsed = 0, frames = 1016, totalDiff = 123
    //C++: 2017-08-26 19:46:39.203911 MediaEngine[445:111660] TimeElapsed = 0, frames = 1016, totalDiff = 588
}

void NeonAssemblyWrapper::Mirror_YUV420_Assembly(unsigned char *pInData, unsigned char *pOutData, int iHeight, int iWidth)
{
#if defined(HAVE_NEON)
    mirror_YUV420_arm_neon(pInData, pOutData, iHeight, iWidth);
#elif defined(HAVE_NEON_AARCH64)
    mirror_YUV420_arm_neon_aarch64(pInData, pOutData, iHeight, iWidth);
#endif
    
    
    
    //c++: IPhone6s --> 2017-11-04 17:20:28.030662+0600 MediaEngine[518:125677] mirrorYUVI420 TimeElapsed = 0, frames = 1000, totalDiff = 771
    //arm64: Iphone6s--> 2017-11-04 17:22:32.765089+0600 MediaEngine[522:126777] mirrorYUVI420 TimeElapsed = 1, frames = 1000, totalDiff = 130
    
    //c++: Ipod --> 2017-11-04 17:31:54.738 MediaEngine[240:30295] mirrorYUVI420 TimeElapsed = 1, frames = 1000, totalDiff = 1421
    //arm32: Ipod --> 2017-11-04 17:30:13.779 MediaEngine[234:29610] mirrorYUVI420 TimeElapsed = 0, frames = 1000, totalDiff = 1127
}

void NeonAssemblyWrapper::DownScaleOneFourthAssembly(unsigned char *pInData, int iHeight, int iWidth, unsigned char *pOutData)
{
#if defined(HAVE_NEON)
    down_scale_one_fourth_arm_neon(pInData, iHeight, iWidth, pOutData);
#elif defined(HAVE_NEON_AARCH64)
    down_scale_one_fourth_arm_neon_aarch64(pInData, iHeight, iWidth, pOutData);
#endif
    
    
    //arm32: Ipod5G 2017-11-14 13:25:27.451 MediaEngine[256:28313] DownScaleOneFourth TimeElapsed = 4, frames = 1010, totalDiff = 4289
    //c++: Ipod5G 2017-11-14 13:28:33.492 MediaEngine[262:29125] DownScaleOneFourth TimeElapsed = 12, frames = 1010, totalDiff = 12282
    
    //arm64: Iphone6S 2017-11-13 17:14:00.506603+0600 MediaEngine[966:252105] DownScaleOneFourth TimeElapsed = 1, frames = 1065, totalDiff = 782
    //c++: Iphone6S2 017-11-13 17:15:51.738798+0600 MediaEngine[969:253245] DownScaleOneFourth TimeElapsed = 3, frames = 1048, totalDiff = 4324
}



