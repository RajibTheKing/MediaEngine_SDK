//
//  Neon_Assembly.s
//  TestCamera 
//
//  Created by Rajib Chandra Das on 7/26/17.
//
//

#ifdef HAVE_NEON

#ifdef ASSEMBLY_IOS
.macro NEON_ARM_FUNC_BEGIN
.syntax unified
.text
.extern printf
.align 2
.arm
.globl _$0
_$0:
.endm
#endif


#ifdef ASSEMBLY_ANDROID
.macro NEON_ARM_FUNC_BEGIN func_name
.syntax unified
.text
.extern printf
.align 2
.arm
.global \func_name
\func_name:
.endm
#endif

.macro NEON_ARM_FUNC_END
mov pc, lr
.endm

NEON_ARM_FUNC_BEGIN down_scale_one_fourth_arm_neon
push {r4-r12, lr}
pInData     .req    r0
iHeight     .req    r1
iWidth      .req    r2
pOutData    .req    r3
ptr1        .req    r4
ptr2        .req    r5
ptr3        .req    r6
ptr4        .req    r7
dx          .req    r8
dy          .req    r9
temp        .req    r10
jumpDiff    .req    r11
temp2       .req    r12

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
        vld1.u8     {q1},       [ptr1]!
        vld1.u8     {q2},       [ptr2]!
        vld1.u8     {q3},       [ptr3]!
        vld1.u8     {q4},       [ptr4]!

        vpaddl.u8 q1, q1
        vpaddl.u16 q1, q1

        vpaddl.u8 q2, q2
        vpaddl.u16 q2, q2

        vpaddl.u8 q3, q3
        vpaddl.u16 q3, q3

        vpaddl.u8 q4, q4
        vpaddl.u16 q4, q4


        vmov.u32 q0, q1
        vadd.u32 q0, q2
        vadd.u32 q0, q3
        vadd.u32 q0, q4

        vshr.u32 q0, q0, #4

        vmov.u32 temp, d0[0]
        strb temp, [pOutData], #1
        vmov.u32 temp, d0[1]
        strb temp, [pOutData], #1
        vmov.u32 temp, d1[0]
        strb temp, [pOutData], #1
        vmov.u32 temp, d1[1]
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
        vld2.u8     {d1, d2},       [ptr1]!
        vld2.u8     {d3, d4},       [ptr2]!
        vld2.u8     {d5, d6},       [ptr3]!
        vld2.u8     {d7, d8},       [ptr4]!

        vpaddl.u8 d1, d1
        vpaddl.u16 d1, d1

        vpaddl.u8 d2, d2
        vpaddl.u16 d2, d2

        vpaddl.u8 d3, d3
        vpaddl.u16 d3, d3

        vpaddl.u8 d4, d4
        vpaddl.u16 d4, d4

        vpaddl.u8 d5, d5
        vpaddl.u16 d5, d5

        vpaddl.u8 d6, d6
        vpaddl.u16 d6, d6

        vpaddl.u8 d7, d7
        vpaddl.u16 d7, d7

        vpaddl.u8 d8, d8
        vpaddl.u16 d8, d8

        vmov.u32 d0, d1
        vadd.u32 d0, d3
        vadd.u32 d0, d5
        vadd.u32 d0, d7
        vshr.u32 d0, d0, #4

        vmov.u32 d9, d2
        vadd.u32 d9, d4
        vadd.u32 d9, d6
        vadd.u32 d9, d8
        vshr.u32 d9, d9, #4

        vmov.u32 temp, d0[0]
        strb temp, [pOutData], #1
        vmov.u32 temp, d9[0]
        strb temp, [pOutData], #1

        vmov.u32 temp, d0[1]
        strb temp, [pOutData], #1
        vmov.u32 temp, d9[1]
        strb temp, [pOutData], #1

        add dx, dx, #16
        cmp dx, iWidth
        bne loopSecondUV

add dy, dy, #4
add pInData, pIndata, jumpDiff
cmp dy, iHeight
bne loopMainUV



pop {r4-r12, pc}
NEON_ARM_FUNC_END

#endif


