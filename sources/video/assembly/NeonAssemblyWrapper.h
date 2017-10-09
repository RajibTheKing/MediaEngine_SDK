//
//  NeonAssemblyWrapper.hpp
//  TestCamera 
//
//  Created by Rajib Chandra Das on 25_September_2017
//
//

#ifndef NeonAssemblyWrapper_hpp
#define NeonAssemblyWrapper_hpp

#include <stdio.h>
#include "Size.h"

#if defined(HAVE_NEON)
extern "C"
{
    void convert_nv12_to_i420_arm_neon(unsigned char* __restrict src, unsigned char* __restrict dest, int iHeight, int iWidth);
    void crop_yuv420_arm_neon(unsigned char* __restrict src, unsigned char* __restrict dst, unsigned int* __restrict param);
}
#elif defined(HAVE_NEON_AARCH64)
extern "C"
{
    void convert_nv12_to_i420_arm_neon_aarch64(unsigned char* __restrict src, unsigned char* __restrict dest, int iHeight, int iWidth);
    void crop_yuv420_arm_neon_aarch64(unsigned char* __restrict src, unsigned char* __restrict dst, unsigned int* __restrict param);
}
#endif


class NeonAssemblyWrapper
{
public:
    NeonAssemblyWrapper();
    ~NeonAssemblyWrapper();
    
    void convert_nv12_to_i420_assembly(unsigned char* __restrict src, int iHeight, int iWidth);
    void Crop_yuv420_assembly(unsigned char* src, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* dst, int &outHeight, int &outWidth);
    unsigned int* __restrict param;
    unsigned char __restrict m_pTempArray[MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3];
    
};

#endif /* NeonAssemblyWrapper_hpp */
