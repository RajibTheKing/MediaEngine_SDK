//
//  Neon_Assembly.s
//  TestCamera 
//
//  Created by Rajib Chandra Das on 7/26/17.
//
//

#if defined(HAVE_NEON_AARCH64)

#ifdef ASSEMBLY_IOS
.macro NEON_ARM_AACH64_FUNC_BEGIN
.text
.extern printf
.align 2
.globl _$0
_$0:
.endm
#endif


#ifdef ASSEMBLY_ANDROID
.macro NEON_ARM_AACH64_FUNC_BEGIN func_name
.text
.extern printf
.align 2
.global \func_name
\func_name:
.endm
#endif

.macro NEON_ARM_AACH64_FUNC_END
ret
.endm

// DownScaleYUVNV12_YUVNV21_OneFourth(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData)
NEON_ARM_AACH64_FUNC_BEGIN down_scale_one_fourth_arm_neon_aarch64
pInData     .req    x0
iHeight     .req    x1
iWidth      .req    x2
pOutData    .req    x3
ptr1        .req    x4
ptr2        .req    x5
ptr3        .req    x6
ptr4        .req    x7
dx          .req    x8
dy          .req    x9
temp        .req    w10
jumpDiff    .req    x11
temp2       .req    x12

/*
total       .req    x13
idx         .req    x14
mov idx , #0
mul total, iHeight, iWidth

loop:
ldrb temp, [pInData], #1
strb temp, [pOutData], #1
add idx, idx, #1
cmp idx, total
bne loop
*/


mov temp2, #4
mul jumpDiff, iWidth, temp2
mov dy, #0

loopMain:
    mov ptr1, pInData
    add ptr2, ptr1, iWidth
    add ptr3, ptr2, iWidth
    add ptr4, ptr3, iWidth
    mov dx, #0

    loopSecond:
            LD1     {V1.16B},       [ptr1], #16
            LD1     {V2.16B},       [ptr2], #16
            LD1     {V3.16B},       [ptr3], #16
            LD1     {V4.16B},       [ptr4], #16

            uaddlp  V1.8H, V1.16B
            uaddlp  V1.4S, V1.8H

            uaddlp  V2.8H, V2.16B
            uaddlp  V2.4S, V2.8H

            uaddlp  V3.8H, V3.16B
            uaddlp  V3.4S, V3.8H

            uaddlp  V4.8H, V4.16B
            uaddlp  V4.4S, V4.8H


            MOV V0.4S, V1.4S
            ADD V0.4S, V0.4S, V2.4S
            ADD V0.4S, V0.4S, V3.4S
            ADD V0.4S, V0.4S, V4.4S

            ushr V0.4S, V0.4S, #4
            mov temp, V0.S[0]
            strb temp, [pOutData], #1
            mov temp, V0.S[1]
            strb temp, [pOutData], #1
            mov temp, V0.S[2]
            strb temp, [pOutData], #1
            mov temp, V0.S[3]
            strb temp, [pOutData], #1
            add dx, dx, #16
            cmp dx, iWidth
            bne loopSecond

    add dy, dy, #4
    add pInData, pIndata, jumpDiff
    cmp dy, iHeight
bne loopMain

lsr iHeight, iHeight, #1
mov dy, #0
loopMainUV:
    mov ptr1, pInData
    add ptr2, ptr1, iWidth
    add ptr3, ptr2, iWidth
    add ptr4, ptr3, iWidth
    mov dx, #0

        loopSecondUV:
        LD2     {V1.8B, V2.8B},       [ptr1], #16
        LD2     {V3.8B, V4.8B},       [ptr2], #16
        LD2     {V5.8B, V6.8B},       [ptr3], #16
        LD2     {V7.8B, V8.8B},       [ptr4], #16

        uaddlp  V1.8H, V1.16B
        uaddlp  V1.4S, V1.8H

        uaddlp  V2.8H, V2.16B
        uaddlp  V2.4S, V2.8H

        uaddlp  V3.8H, V3.16B
        uaddlp  V3.4S, V3.8H

        uaddlp  V4.8H, V4.16B
        uaddlp  V4.4S, V4.8H

        uaddlp  V5.8H, V5.16B
        uaddlp  V5.4S, V5.8H

        uaddlp  V6.8H, V6.16B
        uaddlp  V6.4S, V6.8H

        uaddlp  V7.8H, V7.16B
        uaddlp  V7.4S, V7.8H

        uaddlp  V8.8H, V8.16B
        uaddlp  V8.4S, V8.8H

        MOV V0.4S, V1.4S
        ADD V0.4S, V0.4S, V3.4S
        ADD V0.4S, V0.4S, V5.4S
        ADD V0.4S, V0.4S, V7.4S
        ushr V0.4S, V0.4S, #4

        MOV V9.4S, V2.4S
        ADD V9.4S, V9.4S, V4.4S
        ADD V9.4S, V9.4S, V6.4S
        ADD V9.4S, V9.4S, V8.4S
        ushr V9.4S, V9.4S, #4


        mov temp, V0.S[0]
        strb temp, [pOutData], #1
        mov temp, V9.S[0]
        strb temp, [pOutData], #1

        mov temp, V0.S[1]
        strb temp, [pOutData], #1
        mov temp, V9.S[1]
        strb temp, [pOutData], #1

        add dx, dx, #16
        cmp dx, iWidth
        bne loopSecondUV

    add dy, dy, #4
    add pInData, pIndata, jumpDiff
    cmp dy, iHeight
bne loopMainUV



NEON_ARM_AACH64_FUNC_END


#endif


