//
//  TestNeonAssembly.hpp
//  TestCamera 
//
//  Created by Rajib Chandra Das on 7/26/17.
//
//

#ifndef TestNeonAssembly_hpp
#define TestNeonAssembly_hpp

#include <stdio.h>
//#include "Neon_Assembly.s"

extern "C"
{
    void convert_asm_neon( unsigned char*  __restrict dest, unsigned char* __restrict src, int n);
    void add_asm_neon(unsigned char*  __restrict dest, unsigned char*  __restrict src, int n);
    void copy_asm_neon(unsigned char* __restrict src, unsigned char* __restrict dest, int iLen);
    void convert_nv12_to_i420_asm_neon(unsigned char* __restrict src, unsigned char* __restrict dest, int iHeight, int iWidth);
    void learn_asm_neon();
    void crop_yuv420_arm_neon(unsigned char* __restrict src, unsigned char* __restrict dst, unsigned int* __restrict param);
    void CalculateSumOfLast64_ARM_NEON(unsigned int* __restrict pData, unsigned int* __restrict ans);
    
}


class TestNeonAssembly
{
public:
    TestNeonAssembly();
    ~TestNeonAssembly();
    
    void reference_convert (unsigned char* __restrict dest, unsigned char* __restrict src, int n);
    void neon_intrinsic_convert (unsigned char* __restrict dest, unsigned char* __restrict src, int n);
    void neon_assembly_convert (unsigned char*  __restrict dest, unsigned char* __restrict src, int n);
    void neon_assembly_Inc(unsigned char*  __restrict dest, unsigned char*  __restrict src, int n);
    void Copy_Assembly_Inc(unsigned char* __restrict src, unsigned char* __restrict dest, int iLen);
    void convert_nv12_to_i420_assembly(unsigned char* __restrict src, unsigned char* __restrict dest, int iHeight, int iWidth);
    void learn();
    void Crop_yuv420_assembly(unsigned char* src, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* dst, int &outHeight, int &outWidth);
    void CalculateSumOfLast64_assembly(unsigned int * pData, unsigned int *ans);
    
    unsigned int* __restrict param;
};

#endif /* TestNeonAssembly_hpp */
