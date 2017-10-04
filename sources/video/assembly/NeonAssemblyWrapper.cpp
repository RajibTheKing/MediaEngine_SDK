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


void NeonAssemblyWrapper::convert_nv12_to_i420_assembly(unsigned char* __restrict src, int iHeight, int iWidth)
{
#if defined(HAVE_NEON)
    convert_nv12_to_i420_arm_neon(src, m_pTempArray, iHeight, iWidth);
    memcpy(src, m_pTempArray, iHeight * iWidth * 3 / 2);
#elif defined(HAVE_NEON_AARCH64)
    convert_nv12_to_i420_arm_neon_aarch64(src, m_pTempArray, iHeight, iWidth);
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

