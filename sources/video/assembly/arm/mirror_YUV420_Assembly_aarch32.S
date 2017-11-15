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


//void mirrorYUVI420(unsigned char *pInData, unsigned char *pOutData, int iHeight, int iWidth)
NEON_ARM_FUNC_BEGIN mirror_YUV420_arm_neon
push {r4-r8, lr}
//r0 = pInData, r1 = pOutData, r2 = iHeight, r3 = iWidth

bl ReverseArrayForMirror //Calling for Y Values

lsr         r2,     r2,     #1
lsr         r3,     r3,     #1

bl ReverseArrayForMirror //Calling for U Values

bl ReverseArrayForMirror //Calling for V Values

pop {r4-r8, pc}
NEON_ARM_FUNC_END


//It's a branch to reverse row by row array from a frame
ReverseArrayForMirror:
pInData     .req    r0
pOutData    .req    r1
iHeight     .req    r2
iWidth      .req    r3
iH          .req    r4
iOverlap    .req    r5
iTemp       .req    r6
iLen        .req    r7

mov         iH,         0
ReverseForMirror_HLoop: //Starting Height Loop
    mov         iLen,       iWidth
    add         pOutData,   pOutData,   iLen
    add         pOutData,   pOutData,   #8
    ands        iOverlap,   iLen,       #7
    beq         ReverseForMirror_WLoop //if(iLen % 8 == 0) goto ReverseForMirror_WLoop

    sub         pOutData,   pOutData,   #16
    vld1.u8     {d0},       [pInData]!
    vrev64.u8   d1,         d0
    vst1.u8     {d1},       [pOutData]!
    subs        iLen,       iLen,       iOverlap
    beq ReverseForMirror_completed //if(iLen == 0) go to end

    mov         iTemp,      #8
    sub         iTemp,      iTemp,      iOverlap
    sub         pInData,    pInData,    iTemp
    add         pOutData,   pOutData,   iTemp

    ReverseForMirror_WLoop: //Starting Width Loop
        vld1.u8     {d0},       [pInData]!
        vrev64.u8   d1,         d0
        sub         pOutData,   pOutData, #16
        vst1.u8     {d1},       [pOutData]!
        subs        iLen,        #8
        bne ReverseForMirror_WLoop

    ReverseForMirror_completed:

    sub         pOutData,   pOutData,   #8
    add         pOutData,   pOutData,   iWidth
    add         iH,         iH,         #1
    cmp         iH,         iHeight

    bne ReverseForMirror_HLoop //Height Loop
BX     lr //return



#endif


