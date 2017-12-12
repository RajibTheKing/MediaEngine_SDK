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


//void mirrorYUVI420(unsigned char *pInData, unsigned char *pOutData, int iHeight, int iWidth)
NEON_ARM_AACH64_FUNC_BEGIN mirror_YUV420_arm_neon_aarch64
//r0 = pInData, r1 = pOutData, r2 = iHeight, r3 = iWidth
sub sp, sp, #16
stp x19, x30, [sp]
bl ReverseArrayForMirror //Calling for Y Values

lsr         x2,     x2,     #1
lsr         x3,     x3,     #1

bl ReverseArrayForMirror //Calling for U Values

bl ReverseArrayForMirror //Calling for V Values
ldp x19, x30, [sp]
add sp, sp, #16
NEON_ARM_AACH64_FUNC_END


//It's a branch to reverse row by row array from a frame
ReverseArrayForMirror:
pInData     .req    x0
pOutData    .req    x1
iHeight     .req    x2
iWidth      .req    x3
iH          .req    x4
iOverlap    .req    x5
iTemp       .req    x6
iLen        .req    x7

mov         iH,         0
ReverseForMirror_HLoop: //Starting Height Loop
mov         iLen,       iWidth
add         pOutData,   pOutData,   iLen
add         pOutData,   pOutData,   #8
ands        iOverlap,   iLen,       #7
beq         ReverseForMirror_WLoop //if(iLen % 8 == 0) goto ReverseForMirror_WLoop


sub         pOutData,   pOutData,   #16
LD1     {V0.8B},       [pInData], #8
rev64   V1.8B,         V0.8B
ST1     {V1.8B},       [pOutData], #8
subs        iLen,       iLen,       iOverlap
beq ReverseForMirror_completed //if(iLen == 0) go to end

mov         iTemp,      #8
sub         iTemp,      iTemp,      iOverlap
sub         pInData,    pInData,    iTemp
add         pOutData,   pOutData,   iTemp

ReverseForMirror_WLoop: //Starting Width Loop
LD1     {V0.8B},       [pInData], #8
rev64   V1.8B,         V0.8B
sub         pOutData,   pOutData, #16
ST1     {V1.8B},       [pOutData], #8
subs        iLen,    iLen, #8
bne ReverseForMirror_WLoop

ReverseForMirror_completed:

sub         pOutData,   pOutData,   #8
add         pOutData,   pOutData,   iWidth
add         iH,         iH,         #1
cmp         iH,         iHeight

bne ReverseForMirror_HLoop //Height Loop

ret //return



#endif


