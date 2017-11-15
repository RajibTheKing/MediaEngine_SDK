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
    void convert_nv12_to_i420_arm_neon(unsigned char*  src, unsigned char*  dest, int iHeight, int iWidth);
    void convert_i420_to_nv12_arm_neon(unsigned char*  src, unsigned char*  dest, int iHeight, int iWidth);
    void crop_yuv420_arm_neon(unsigned char*  src, unsigned char*  dst, unsigned int*  param);
    
    void mirror_YUV420_arm_neon(unsigned char *pInData, unsigned char *pOutData, int iHeight, int iWidth);
    void down_scale_one_fourth_arm_neon(unsigned char *pInData, int iHeight, int iWidth, unsigned char *pOutData);
    
}
#elif defined(HAVE_NEON_AARCH64)
extern "C"
{
    void convert_nv12_to_i420_arm_neon_aarch64(unsigned char*  src, unsigned char*  dest, int iHeight, int iWidth);
    void convert_i420_to_nv12_arm_neon_aarch64(unsigned char*  src, unsigned char*  dest, int iHeight, int iWidth);
    void crop_yuv420_arm_neon_aarch64(unsigned char*  src, unsigned char*  dst, unsigned int*  param);
    
    void mirror_YUV420_arm_neon_aarch64(unsigned char *pInData, unsigned char *pOutData, int iHeight, int iWidth);
    void down_scale_one_fourth_arm_neon_aarch64(unsigned char *pInData, int iHeight, int iWidth, unsigned char *pOutData);
    
}
#endif


class NeonAssemblyWrapper
{
public:
    NeonAssemblyWrapper();
    ~NeonAssemblyWrapper();
    
    void convert_nv12_to_i420_assembly(unsigned char*  src, int iHeight, int iWidth);
    void convert_i420_to_nv12_assembly(unsigned char*  src, int iHeight, int iWidth);
    void Crop_yuv420_assembly(unsigned char* src, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* dst, int &outHeight, int &outWidth);
    
    
    void Mirror_YUV420_Assembly(unsigned char *pInData, unsigned char *pOutData, int iHeight, int iWidth);
    void DownScaleOneFourthAssembly(unsigned char *pInData, int iHeight, int iWidth, unsigned char *pOutData);
    
    
    unsigned int* param;
    unsigned char m_pTempArray[MAX_VIDEO_FRAME_INPUT_HEIGHT * MAX_VIDEO_FRAME_INPUT_WIDTH * 3];
    
};

#endif /* NeonAssemblyWrapper_hpp */
